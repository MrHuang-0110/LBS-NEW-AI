# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for an STM32H723 (Cortex-M7 @ 480 MHz) educational robotics controller. Bare C on top of STM32 HAL + FreeRTOS, with a PikaScript (Python-on-MCU) runtime so end-user programs written in Python are downloaded to the device and executed at runtime. Also integrates ST X-CUBE-AI (neural network inference), STM32 MotionFX sensor fusion, FATFS on external QSPI flash, and USB CDC.

There is **no CLI build, no test suite, no linter**. All building is done in Keil µVision (MDK-ARM). `compile_flags.txt` is for clangd/IDE intellisense only — it does not drive a build.

## Build & flash

1. Open `MDK-ARM/STM32H723.uvprojx` in Keil µVision (MDK-ARM). Build target `STM32H723`.
2. Linker output is `MDK-ARM/STM32H723/pikaNewAi.axf`; scatter file is `MDK-ARM/STM32H723/pikaNewAi.sct`.
3. The project's **After-Build user commands** run automatically:
   - `fromelf.exe --bin -o "$L@L.bin" "#L"` → produces `pikaNewAi.bin`
   - `postbuild.bat` → copies that `.bin` into the separate download project at `E:\LBS-Project\NEW-AI-PROJECT\piKaNewAI-bin-downLoad\DownloadFWLIB\LB_FWLIB\app` (hardcoded path — edit `postbuild.bat` if your checkout lives elsewhere).
4. `keilkilll.bat` cleans Keil intermediate artifacts (`.o`, `.d`, `.crf`, `.axf`, `.map`, …). It deliberately does **not** delete `*.opt` (JLink settings).

App code runs from flash offset `0x08020800` — `main()` sets `SCB->VTOR = 0x08020800`, so a bootloader occupies the first 0x20800 bytes. Keep this offset in mind when touching the scatter file or vector table.

Compile defines: `STM32H723xx`, `USE_HAL_DRIVER`, `PIKA_CONFIG_ENABLE`.

## Boot & runtime architecture

Single entry chain; understanding this is prerequisite to touching anything:

1. `main()` ([Core/Src/main.c](Core/Src/main.c)) — enables I/D-cache, HAL init, clocks (480 MHz), `freeRtosHeapMemInit()` (defines the FreeRTOS heap across two regions: 64 KB DTCM + 256 KB AXI, see `ucHeapAXI`/`ucHeapDCM` in [matchineState.c](Drivers/DataFile/machine/matchineState.c)), then `NEWAI_CreatePowerOnStartTask()`.
2. `NEWAI_CreatePowerOnStartTask()` ([matchineState.c](Drivers/DataFile/machine/matchineState.c)) — creates the `MatChineStateTask`, the global event group `xEventGroup`, the shared mutexes (`xUsbMutex`, `xRunPythonMutex`, `xRefreshMutex`, `xFsMutex`), calls `MultiUart_Init()`, then `vTaskStartScheduler()`.
3. `MatChineStateTask` — the central state machine. It spawns `USB_Download_Task` and `EnteryTask`, runs `bspInit()` (all peripheral bring-up), sets up the LED matrix UI, then creates a set of **periodic FreeRTOS software timers** (key scan 10 ms, port scan 50 ms, monitor 30 ms, battery 100 ms, bluetooth 200 ms, IWDG feed 50 ms, motion/mem 20 ms, remote timeout 2000 ms) and finally `MX_USB_DEVICE_Init()`. Its main loop waits on `xEventGroupWaitBits` and dispatches events.
4. `EnteryTask` — waits on `EVENT_RUN_PYTHON`, takes `xRunPythonMutex`, calls `runPython()`. `EVENT_RUN_PYTHON` is set from `ui_entery()` when the user picks a UI slot `< 20` (or `REMOTRE_LOGO`). Calling it again while running triggers `pks_vm_exit()` to stop the active script.

### Event system

All events are bits on a single `EventGroupHandle_t xEventGroup` (defined in [matchineState.h](Drivers/DataFile/machine/matchineState.h)):

| Event | Bit | Triggered by | Handled in |
| --- | --- | --- | --- |
| `EVENT_REFRESH_MATRIX` | 0 | Timer/key events | `MatChineStateTask` main loop |
| `EVENT_SEND_MONITOR` | 1 | 30 ms timer | `newAiMonitor()` sends JSON sensor data over USB |
| `EVENT_FIND_PORT_DEV` | 2 | 50 ms timer | `FindProtDev()` — scans ports for attached devices |
| `EVENT_USB_FRAM_BYTE` | 3 | USB CDC data received | `USB_Download_Task` |
| `EVENT_KEY_ENTERY` | 4 | Key press detected | `ui_entery()` — UI navigation |
| `EVENT_BLUE_FRAM_BYTE` | 6 | Bluetooth data received | `USB_Download_Task` |
| `EVENT_RUN_PYTHON` | 7 | UI entry selection | `EnteryTask` → `runPython()` |
| `EVENT_MEM_REFRESH` | 9 | 20 ms timer | IMU sensor fusion update |
| `EVENT_SENORD_REFRESH` | 10 | — | Sensor data refresh |
| `EVENT_BLUE_STATE_REFRESH` | 11 | — | Bluetooth state update |
| `EVENT_BAT_REFRESH` | 12 | 100 ms timer | Battery level display refresh |
| `EVENT_CAMER_MODE` | 5 | — | Camera mode switching |
| `EVENT_PLAYER_KEY_VIOC` | 16 | — | Player/multimedia key handler |

Use `SET_EVENT_GROUP()` / `SET_EVENT_GROUP_ISR()` to signal; never invent a second event group.

### Concurrency model

- `usbDownloadActive` is a global gate: when a USB/Bluetooth file download is in progress, the state-machine timers skip sensor/UI refresh, key scanning, and port scanning. Respect this when adding timer callbacks.
- FreeRTOS heap is the only general allocator inside tasks (`pvPortMalloc`/`vPortFree`). Older `mymalloc(SRAMIN, …)` calls are commented out in favor of `pvPortMalloc` — follow that pattern.
- Shared mutexes: `xUsbMutex` (USB/Bluetooth download), `xRunPythonMutex` (Python execution), `xRefreshMutex` (matrix display), `xFsMutex` (FATFS file system).

## Multi-UART device framework

Sensors and actuators are addressed through a multi-UART abstraction in [matchineState.c](Drivers/DataFile/machine/matchineState.c) and [portAgree/](Drivers/DataFile/portAgree):

- `MultiUart_Init()` (in [uart.c](Drivers/BSP/uart/)) — initializes all UART peripherals. Each UART gets a device context (`UartDeviceContext_t`) with its own `devControlQueue`, `devTxQueue`, `uartMutex`, and `txCompleteSem`.
- `vDevControlTask` / `vDevUartSendTask` — one instance per UART, draining `devControlQueue` (incoming sensor data) / `devTxQueue` (outgoing commands).
- `vDevControlTask` dispatches by `SensorBase->type` to `refsh_motor`, `refsh_gray`, `refsh_color`, `refsh_touch`, `refsh_ultrasion`, `refsh_camer`, `refsh_gray_v2`, `refsh_nfc`, `refsh_ir`, `refsh_elect_sensor`.

### Device identification protocol

`FindProtDev()` (triggered by `EVENT_FIND_PORT_DEV` on a 50 ms timer) scans 8 ports. Each port has a `__PORT` struct with a `SensorBase*` pointer. The protocol:

1. Sends a link query frame to the device (`DEV_PORT_LINKE` command).
2. Device responds with a string — if it contains `"Play Aplication"`, the device is identified and `identify_and_bind()` creates the appropriate typed device struct (e.g., `DEV_MOTOR`, `DEV_GRAY`).
3. Device IDs (as defined in the module headers): `DEV_ID_BIG_MOTOR` (0xA1), `DEV_ID_COLOR` (0xA2), `DEV_ID_ULTRASION` (0xA3), `DEV_ID_TOUCH` (0xA4), `DEV_ID_SMALL_Motor` (0xA6), `DEV_ID_CAMER` (0xA7), `DEV_ID_GRAY` (0xA9), `DEV_ID_GRAY_V2` (0xB0), `DEV_ID_NFC` (0xB2). `DEV_ID_IR` (0xB3) is host-internal only: the IR_REMOTE device reports the wire ObjectID `0xA3` on its handshake, so `port_linke()` maps it to `DEV_ID_IR` and the ADC-detected ultrasonic keeps `0xA3`. `DEV_ID_ELECT_SENSOR` (0xE0) uses the handshake ObjectID directly — no internal ID mapping.

### Wire protocol

The `_AGREEMENT` frame format (in [portagree.h](Drivers/DataFile/portAgree/portagree.h)):

```
Head(0x5A) | sID | oID | length(2B) | index | data[256] | crc | tard(0xA5)
```

A `FrameParser` state machine (`STATE_IDLE` → `STATE_HEADER` → `STATE_SRC_ID` → `STATE_DEST_ID` → `STATE_LENGTH` → `STATE_TYPE` → `STATE_DATA` → `STATE_CHECKSUM` → `STATE_FOOTER`) handles byte-by-byte deserialization. `handle_complete_frame()` routes completed frames to the appropriate handler.

## Device modules

### Motor (`_motor.pyi` / `pikascript-lib/motor/_motor.c` / [Drivers/DataFile/motor/](Drivers/DataFile/motor/))

- Supports big motors (0xA1) and small motors (0xA6).
- `DEV_MOTOR` struct contains `_USER_MOTOR` (stop mode, power), `MOTOR_RUN_INFO` (speed, angle, position, encoders), and `MOTOR_CONTROL` (PID position control).
- Motor control modes: `MOTOR_IDLE`, `MOTOR_STOP_SILD`, `MOTOR_STOP_BREAK`, `MOTOR_STOP`, `MOTOR_SPEED`, `MOTOR_SPEED_POS`, `MOTOR_PWM`.
- `_USER_DOUBLE_MOTOR` pairs two motors for differential drive with yaw PID and line-following PID (`pid_find_line`).
- Python API includes `run()`, `stop()`, `stop_module()`, `run_for_degrees()`, `run_power()`, `set_duty()`, `pair()`, `mov()*` (differential drive), `pid_*` (PID tuning), `absolute_position()`, `relative_angle()`.

### Gray (grayscale/line sensor, `_gray.pyi` / `pikascript-lib/gray/_gray.c`)

- `DEV_ID_GRAY` (0xA2). `DEV_GRAY` struct with 8 sensor values, 8 states, thresholds, and calibration.
- Python API: `read()`, `isLine()`, `isCross()`, `grayValue()`, `one_calibrate()`, `two_calibrate()`.

### GrayV2 (enhanced line sensor, `_grayv2.pyi` / `pikascript-lib/grayv2/_grayv2.c`)

- `DEV_ID_GRAY_V2` (0xB0). 7-channel sensor with way-type detection (`T_WAY`, `LEFT_WAY`, `RIGHT_WAY`, `MIDDLE_WAY`, etc.).
- `grayv2_find_line()` — integrated line-following controller using motor pair + PID.
- Python API: `getWay()`, `getL()`, `getState()`, `getThreshold()`, `setThreshold()`, `pid()`, `init()`, `calibrate()`, `findLine()`.

### Touch (`_touch.pyi` / `pikascript-lib/touch/_touch.c`)

- `DEV_ID_TOUCH` (0xA3). Returns touch sensor state.
- Python API: `isDown()`, `htouch()`.

### Ultrasion (ultrasonic sensor, `_ultrasion.pyi` / `pikascript-lib/ultrasion/_ultrasion.c`)

- `DEV_ID_ULTRASION` (0xA4). Distance measurement.
- Python API: `get()`, `getCM()`, `getMM()`.

### Color (`_color.pyi` / `pikascript-lib/color/_color.c`)

- `DEV_ID_COLOR` (0xA5). Color sensor with calibration.
- Python API: `get()`, `getRGB()`, `getHue()`, `getLight()`, `isColor()`, `one_calibrate()`, `two_calibrate()`.

### Camer (AI camera, `_camer.pyi` / `pikascript-lib/camer/_camer.c`)

- `DEV_ID_CAMER` (0xA7). Vision AI camera with multiple recognition modes.
- Modes: `CAMER_MENU_TYPE`, `CAMER_MODE_TYPE`, `CAMER_FACE_TYPE`, `CAMER_LABE_TYPE`, `CAMER_OBJECT_TYPE`, `CAMER_COLOR_TYPE`, `CAMER_WAY_TYPE`, `CAMER_GESTURE_TYPE`, `CAMER_BODY_TYPE`, `CAMER_OBJECT_BODY_TYPE`, `CAMER_PHOTO_TYPE`.
- Python API: `isDetect()`, `get_id()`, `get_x()`, `get_y()`, `get_w()`, `get_h()`, `mode()`, `photo()`, `get_photo()`.

### IR remote (`_ir.pyi` / `pikascript-lib/ir/_ir.c`)

- Wire ObjectID `0xA3` (same value as `DEV_ID_ULTRASION`), host-internal `DEV_ID_IR` (0xB3). Handshake
  reply is `"Play Aplication"` (sic, misspelled on purpose in the device firmware).
- Uplink `0xED` every 10 ms: packed `{state, bat}` - **no version field** (unlike motor/color/gray).
  `state` is the last commanded colour (0=off 1=red 2=green 3=blue), not a receiver acknowledgement;
  `bat` is the receiver battery 0..100, `0xFF` = unknown.
- Downlink `0xD1` + 1 byte state. Python API: `set_rgb(port, state)`.

### Elect sensor (`_elect_sensor.pyi` / `pikascript-lib/elect_sensor/_elect_sensor.c`)

- `DEV_ID_ELECT_SENSOR` (0xE0) — electromagnetic engagement sensor. The handshake ObjectID is used
directly as the host device ID; there is no internal ID mapping.
- Uplink `0xED`, payload = **exactly 1 byte** command state: `0` = released, `1` = engaged. A frame
  whose length is not 1, or whose state byte is outside 0/1, is ignored and the last valid state is
  kept.
- Downlink `0xD1` with an **empty** payload = engage, `0xD2` with an **empty** payload = release.
  The device requires exactly 7 bytes (`len == 0`) delivered as **one contiguous burst** — it
  delimits frames with the USART IDLE interrupt and its `0xD1`/`0xD2` handlers additionally require
  `rx_data->len >= 7`, so a command split across writes is ignored.
- **Delivery model (do not make this fire-and-forget).** The device sends no ACK and the host's TX
  queue is depth 1 with a 10 ms timeout, so a single command frame can be dropped silently. The
  device also forces both outputs off on *every* handshake, so a reconnect/reset/re-handshake drops
  an engaged coil. The vendor protocol therefore requires the host to (a) re-send while `0xED` does
  not echo the requested state and (b) restore the requested state after a re-handshake; the command
  is idempotent and 10 Hz is explicitly allowed. `set_state()` sends immediately, then
  `elect_sensor_poll()` — called from the existing 50 ms port scan, no new timer — re-sends while the
  echo differs (worst case ~100 ms to recover).
- A full unplug/replug frees the port and `create_elect_sensor()` resets the requested state to 0, so
  a physical reconnect never re-energises the coil on its own; only an explicit `set_state(port, 1)`
  does. A re-handshake *without* losing the port does restore the state the user program asked for.
- The echoed `state` is a command echo, **not** physical coil/contact feedback.
- Python API: `set_state(port, state)`. Out-of-range ports/states and ports without a bound 0xE0
  device are silently ignored. Monitor JSON: `elect_sensor.state` (device echo),
  `elect_sensor.SoftwareVersion`.

### NFC (`_nfc.pyi` / `pikascript-lib/nfc/_nfc.c`)

- `DEV_ID_NFC` (0xA8). NFC tag reader/writer.
- Python API: `read()`, `write()`.

## Sensor firmware update (IAP)

Device firmware is updated from the host over the device's own port by `sensord_updata()` in
[download.c](Drivers/DataFile/download/download.c). It is triggered by the `0x32` command and only
runs while `getRunState()` is false (no user program active). The sequence per port is:

| Step | Frame | Notes |
| --- | --- | --- |
| 1 | `0xEE` + `"1"` soft reset | device reboots into its bootloader, which then handshakes with `"Not Aplication"` (host sees `_LinkeObjDev == 0xEF`) |
| 2 | `0x08` + `"0"` | keepalive/ack gate before flashing |
| 3 | `0xAA` | firmware payload, 128 bytes per frame |
| 4 | `0xBB` | version string, written after the last chunk |
| 5 | `0xFE` | jump back to the application |

Per-device file mapping, resolved by `_read_sensord_bin()`, `_read_sensord_versionfile()` and
`getdevSoftware()` in [devfile.c](Middle/File_IO/devfile.c):

| `DEV_ID_*` | Firmware image | Version file |
| --- | --- | --- |
| `DEV_ID_BIG_MOTOR` (0xA1) | `1:app/motor.bin` | `1:version/BigMotorVersion.txt` |
| `DEV_ID_COLOR` (0xA2) | `1:app/color.bin` | `1:version/ColorVersion.txt` |
| `DEV_ID_SMALL_Motor` (0xA6) | `1:app/small_motor.bin` | `1:version/MiddleMotorVersion.txt` |
| `DEV_ID_GRAY` (0xA9) | `1:app/gray.bin` | `1:version/gray.txt` |
| `DEV_ID_GRAY_V2` (0xB0) | `1:app/grayv2.bin` | `1:version/grayv2.txt` |
| `DEV_ID_ELECT_SENSOR` (0xE0) | `1:app/elect_sensor.bin` | `1:version/elect_sensor.txt` |

`_DEV_CFG` in [devfile.h](Middle/File_IO/devfile.h) caches the versions read at boot by
`fatfsInit()`; `verElectSensor` was **appended** after the existing fields so the on-flash
`1:system.cfg` layout stays compatible. A missing image, a missing/zero version, or any bootloader
ACK timeout takes the existing error path (`showError()` + abort, no success prompt) — success is
only shown after the `0xBB` version write is acknowledged.

## PikaScript (Python) toolchain — read before editing `python/`

User-facing Python API is defined by `*.pyi` stubs in `python/` (e.g. `_motor.pyi`, `_gray.pyi`, `newai.pyi`). The C side is **auto-generated** — do not hand-edit:

- `python/pikascript-api/*.h` and `pikaScript.c` — generated bindings included by the firmware.
- `python/pikascript-api/*.py.o`, `pikaModules.py.a` — pre-compiled bytecode modules.
- `python/pikascript-lib/<module>/_<module>.c` — the **hand-written** native C implementations behind each Python module (e.g. `pikascript-lib/motor/_motor.c` implements what `_motor.pyi` declares). This is where you change native behavior.

Tooling in `python/`:

- `rust-msc-latest-win10.exe` — the rust-based Pika compiler that actually generates the bindings. **After editing any `.pyi` or `.py` you must re-run this** (`python/rust-msc-latest-win10.exe` with `python/` as the working directory) to regenerate `pikascript-api/` bindings and bytecode, then rebuild the firmware. There is no prebuild hook in the Keil project (`<BeforeMake>` is empty), so regeneration is manual. It also rewrites the gitignored-but-build-required `pikascript-api/main.py.o`, `pikaModules.py.a` and `__asset_pikaModules_py_a.c` (the latter is compiled into the firmware, so a stale copy silently ships stale bytecode).
- `pikaPackage.exe` — the upstream package manager. **Do not use it here**: it re-resolves and downloads `requestment.txt` dependencies instead of regenerating bindings against the vendored `pikascript-core`, which can churn the core version. Use `rust-msc-latest-win10.exe`.
- `requestment.txt` — PikaScript package versions (pikascript-core==v1.13.4, PikaStdLib==v1.13.4, _thread==v0.0.7, time==v0.2.2, math==v0.1.1, random==v0.1.4).
- `main.py` — the default Python entry source that imports all modules.

At runtime, user programs are **not** compiled on-device. `runPython()` in [Drivers/DataFile/entery/entery.c](Drivers/DataFile/entery/entery.c) loads a pre-compiled `.o` bytecode file from external flash (FATFS path `1:app/<uiListNum>.o`) and executes it via `pikaVM_runByteCodeInconstant`. Files are downloaded to flash over USB/Bluetooth by `USB_Download_Task`.

### PikaScript platform glue

`pika_config.c` ([Core/Src/pika_config.c](Core/Src/pika_config.c)) provides the platform adaptation layer — `pika_platform_malloc`/`free`/`realloc` (wrapping `pvPortMalloc`/`vPortFree`), `pika_platform_sleep_ms` (using `vTaskDelay` with GIL release), `pika_platform_fopen`/`fread`/`fwrite`/`fclose`/`fseek`/`ftell` (wrapping FATFS `f_open`/`f_read`/etc.), and `newai_Task_platformGetTick`.

## Monitor protocol

`newAiMonitor()` (in [portagree.c](Drivers/DataFile/portAgree/portagree.c)) sends a JSON-formatted monitor string over USB CDC at 30 ms intervals (triggered by `EVENT_SEND_MONITOR`). The JSON contains all sensor readings, port states, battery level, and motor data. The monitor is suppressed during USB/Bluetooth idle time or when `is_monitor` is false.

## IMU / MotionFX

[Drivers/DataFile/mem/](Drivers/DataFile/mem/) contains LSM6DS3TR-C (accelerometer + gyroscope) and LIS2MDL (magnetometer) drivers with ST MotionFX sensor fusion:

- `lsm6ds3tr_c_motion_fx_determin()` — called on `EVENT_MEM_REFRESH` (20 ms timer). Reads IMU data and runs MotionFX algorithm to compute pitch, roll, yaw.
- `DEV_MEM` struct holds raw and processed data: `angular_rate_mdps`, `acceleration_mg`, `magnetic_mG`, `pitch`/`roll`/`yaw`, `dt`, and `MFX_output_t`.
- `get_yaw()`, `get_continuous_yaw()`, `resetyaw()`, `get_raw_grayz()` — query functions.
- `stm32_motionfx_library` precompiled library at `STM32_MotionFX_Library/`.

## PID control

`Middle/PID_CONTROL/pid_control.h` provides:

- `PIController` — PI controller with `Kp`, `Ki`, `integral`, `out_limit`, `integral_limit`.
- `PositionController` — PID position controller with `Kp`, `Ki`, `Kd`, `target_pos`.
- Functions: `PI_Init`, `PI_Compute`, `PI_Reset`, `Pos_Init`, `Pos_Compute`, `Pos_SetTarget`.

Used by motor control (`MOTOR_CONTROL` uses `PositionController` + `PIController` for V/50 speed), line following (`grayv2_find_line` + `pid_find_line`), and yaw control (`yaw_pid`).

## Filter system

[Drivers/DataFile/filter/](Drivers/DataFile/filter/) provides:

- `AdaptiveFilter` — adaptive filter with configurable min/max alpha and speed threshold.
- `DualMotorSyncController` — dual-motor synchronization controller with speed difference tracking, filtering, and sync adjustment calculation.

## USB CDC / Bluetooth download

[Drivers/DataFile/download/](Drivers/DataFile/download/) handles file download over USB (`USB_PORT 0x09`) and Bluetooth (`BLUE_PORT 0x0A`):

- `USB_Download_Task` — waits on `EVENT_USB_FRAM_BYTE` / `EVENT_BLUE_FRAM_BYTE`, calls `downloadFile()`.
- `downloadFile()` — state machine: `STATE_CREATE_FILE` → `STATE_RECEIVE_DATA` → `STATE_RECEIVE_END` → `STATE_SAVE_FILE`. Saves to FATFS on external QSPI flash.
- Ring buffers (`getUSB_RingBuffer_Handle`, `getBLUE_RingBuffer_Handle`) buffer incoming data.
- `USB_IdleTimeoutCallback` / `BLUE_IdleTimeoutCallback` — 3 ms one-shot timers that fire after data stops arriving, triggering `process_received_data()`.

## LED matrix display

[Drivers/User/matrix/](Drivers/User/matrix/) drives a 9×7 LED matrix:

- `_API_MATRIX_CFG` — holds lamp array (9 bytes), XY pixel array (9×7), color, brightness, roll delay.
- `DRIVER_MATRIX` — lower-level driver struct.
- Functions: `_show_write_led`, `_show_roll_ui`, `_show_roll_pika`, `_ui_row_refresh`, `_clear_matrix`, `set_pixe`, `matrix_set_xy_color`.
- UI constants: `UI_DEFAULT_COLOR` (0x080FF00), `UI_DEFAULT_BRIGTNESS` (12).
- `refresmatrtixlamp()` — updates the lamp display for a given port state.
- Python binding: `_matrix.pyi` → `pikascript-lib/matrix/_matrix.c`.

## LED matrix UI / rawMatrix

[Drivers/DataFile/rawMatrix/](Drivers/DataFile/rawMatrix/) manages the UI screen system:

- UI screens: `REMOTRE_LOGO` (54), `BLUE_LOGO` (53), `BAT_LOGO` (55).
- `redrawUIInit()` — initializes UI from file, shows startup logo.
- `redrawMatrixUI()` — redraws the current UI screen.
- `sendUIlistNumber()` — sends the current UI list number over USB.
- `get_ui_num()`, `get_ui_file()`, `getCurrentUiList()`, `getDefaultKey()` — UI navigation queries.

## Key scanning

[Drivers/User/key/](Drivers/User/key/) handles key input scanned at 10 ms (`KeyScanTimerCallback`). `updata_key_value()` reads key state, `ui_entery()` (in [entery/](Drivers/DataFile/entery/)) dispatches key events to the UI system.

## Bluetooth

[Drivers/DataFile/blue/](Drivers/DataFile/blue/) manages the Bluetooth module:

- `BLUE_CONFIG` — state (`blueState`, `atState`), AT command data, MAC address, monitor buffer.
- `blue_init()` — initializes BT module.
- `blue_onAndoff()` — power control.
- `is_valid_at_command()`, `is_get_at_data()` — AT command parsing.
- Blue LED indicator on GPIOD pin 6, BT state on GPIOD pin 10.
- `BLUE_printf()` — sends monitor data over Bluetooth when connected.

## Remote control

[Drivers/DataFile/remote/](Drivers/DataFile/remote/) handles remote control input:

- `getremotevalue()`, `refreshRemoteValue()`, `clearRemoteValue()`.
- `set_remote_linke()` / `close_remote_linke()` / `get_remote_linke()` — remote connection state.
- `REMOTE_TimeoutCallback` — 2 second timer that clears remote values on timeout.

## Battery monitoring

In `matchineState.c` (`BatSendTimerCallback` at 100 ms):

- Reads ADC channels for battery voltage and voice coil voltage.
- `ADC_TO_VOLTAGE = 3.3f / 65535.0f`, `VOLTAGE_SCALE = 151.0f / 51.0f` (≈2.96 divider).
- Battery range: 7.0V min, 8.2V max (1.2V range). 4-level indicator.
- `getBatLevel()` → `_show_write_led()` updates the battery icon on the matrix.

## QSPI flash / FATFS

[Drivers/User/w25qxx/](Drivers/User/w25qxx/) drives the external W25Qxx QSPI flash. The `devfile.c` ([Middle/File_IO/](Middle/File_IO/)) layer provides FATFS integration:

- `getFatfsHandle()` — returns the `_IO_FILE` handle for FATFS operations.
- File paths: `1:app/<filename>` for user programs, `1:` for the flash root.
- Used by the download system to save user programs, by the UI system to load screen layouts, and by pika_config for `pika_platform_fopen`/`fread`/`fwrite`.

## AI / X-CUBE-AI

`Middle/AI/` contains the ST X-CUBE-AI runtime headers and `Lib/NetworkRuntime1020_CM7_Keil.lib` (precompiled). `X-CUBE-AI/App/` has the generated neural network model (`network.c/h`, `network_data*.*`) — regenerated by CubeMX/X-CUBE-AI, not hand-edited. The AI inference is used by the color AI module (`colorAi_init()` is called but commented out in the current state machine).

## Utility libraries

- [Drivers/DataFile/cjson/](Drivers/DataFile/cjson/) — JSON parsing (used in monitor).
- [Drivers/DataFile/list/](Drivers/DataFile/list/) — `DoublyLinkedList` (doubly linked list with `create_node`, `insert_head`, `delete_at`, `find`, `destroy_list`).
- [Drivers/DataFile/strlist/](Drivers/DataFile/strlist/) — string list utilities.
- [Drivers/DataFile/ringbufer/](Drivers/DataFile/ringbufer/) — ring buffer for USB/Bluetooth data.
- [Drivers/DataFile/valueType/](Drivers/DataFile/valueType/) — data type definitions (`DATA_TYPE_INT8`, `DATA_TYPE_FLOAT`, `DATA_TYPE_PID_PARAMS`, etc.).
- [Drivers/DataFile/message/](Drivers/DataFile/message/) — message passing utilities.
- [Drivers/DataFile/devPrograment/](Drivers/DataFile/devPrograment/) — device programming/timer-command infrastructure (`create_timer_with_cmd`, `TimerCommandPair`).
- [Middle/CTL_CMD/](Middle/CTL_CMD/) — command control system.
- [Middle/MALLOC/](Middle/MALLOC/) — legacy malloc implementation (mostly replaced by `pvPortMalloc`).
- [Middle/SYSTEM/](Middle/SYSTEM/) — `sys.c`/`sys.h` with basic system utilities.

## Directory map

- `Core/` — STM32 HAL boilerplate, `main.c`, `pika_config.*` (PikaScript platform glue).
- `Drivers/BSP/` — low-level peripheral BSP (adc, dac, gpio, iic, iwdg, ospi, spi, tim, uart with `MultiUart_Init`).
- `Drivers/User/` — higher-level drivers: `w25qxx` (QSPI flash), `matrix` (LED matrix), `key`, `music`, `os`.
- `Drivers/DataFile/` — the application framework. Subdirs: `machine` (state machine + tasks + heap), `entery` (Python runner + UI entry), `portAgree` (port/wire protocol), `download` (file ingest), `blue` (Bluetooth), `motor`/`gray`/`touch`/`ultrasion`/`color`/`camer`/`grayv2`/`nfc` (device modules), `mem` (IMU/MotionFX), `remote` (remote control), `rawMatrix` (UI manager), `filter` (adaptive filter + motor sync), `message`, `cjson`, `ringbufer`, `dataStruct`, `valueType`, `list`, `strlist`, `devPrograment`.
- `Middle/` — `SYSTEM`, `DELAY`, `MALLOC`, `File_IO` (FATFS wrapper), `AI` (X-CUBE-AI), `PID_CONTROL`, `CTL_CMD`, `DRIVER_CONTROL/motor_control`.
- `X-CUBE-AI/App` — ST-generated neural network. Regenerated by CubeMX, not by hand.
- `STM32_MotionFX_Library` — precompiled ST motion library.
- `USB/` — STM32 USB CDC device stack. `python/`, `FreeRTOS/`, `FATFS/` are upstream vendored — avoid editing in place where possible.
- `MDK-ARM/` — Keil project, scatter file, startup `startup_stm32h723xx.s`.

## Working in this repo

- When changing native behavior exposed to Python: edit `python/pikascript-lib/<mod>/_<mod>.c` (and/or the `Drivers/DataFile/<device>` C it calls), re-run `python/rust-msc-latest-win10.exe` from the `python/` directory, then rebuild in Keil. Editing a `.pyi` without regenerating leaves the C bindings stale.
- When adding a new periodic action: create a FreeRTOS software timer in `MatChineStateTask` (mirroring the existing `xTimerCreate` + `xTimerStart` pattern) and signal work to the main loop via a new `EVENT_*` bit rather than doing the work in the timer callback.
- Source files use GBK-encoded Chinese comments in places. Preserve encoding when editing existing C files to avoid mojibake in Keil.
- **Device ID range**: sensors occupy `0xA1`–`0xB3` (`0xB0` grayv2, `0xB2` NFC, `0xB3` IR remote) plus `0xE0` for the electromagnetic sensor; `0x09` is the USB port, `0x0A` the Bluetooth port. `0xEF` is the reserved "device present but unidentified" sentinel, not a real device ID. When adding a new device type, assign a new `DEV_ID_*`, add it to `identify_and_bind()`, `vDevControlTask`, and create a Python module binding.
- **Memory**: The FreeRTOS heap spans two regions — `ucHeapDCM` (64 KB DTCM, accessed via `__attribute__((section(".DTCM_Data")))`) and `ucHeapAXI` (256 KB AXI SRAM). `pvPortMalloc` auto-selects the region. Never use `malloc`/`free` from the standard library.
- **PikaScript GIL**: `pika_platform_sleep_ms` releases the GIL (`pika_GIL_EXIT()`) during `vTaskDelay` so other Python threads can run. Blocking operations in native C bindings should follow the same pattern.
