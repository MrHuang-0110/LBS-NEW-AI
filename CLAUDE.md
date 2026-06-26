# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for an STM32H723 (Cortex-M7 @ 480 MHz) educational robotics controller. Bare C on top of STM32 HAL + FreeRTOS, with a PikaScript (Python-on-MCU) runtime so end-user programs written in Python are downloaded to the device and executed at runtime. Also integrates ST X-CUBE-AI (neural network inference), STM32 MotionFX, FATFS on external QSPI flash, and USB CDC.

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

## PikaScript (Python) toolchain — read before editing `python/`

User-facing Python API is defined by `*.pyi` stubs in `python/` (e.g. `_motor.pyi`, `_gray.pyi`, `newai.pyi`). The C side is **auto-generated** — do not hand-edit:

- `python/pikascript-api/*.h` and `pikaScript.c` — generated bindings included by the firmware.
- `python/pikascript-api/*.py.o`, `pikaModules.py.a` — pre-compiled bytecode modules.
- `python/pikascript-lib/<module>/_<module>.c` — the **hand-written** native C implementations behind each Python module (e.g. `pikascript-lib/motor/_motor.c` implements what `_motor.pyi` declares). This is where you change native behavior.

Tooling in `python/`:
- `pikaPackage.exe` — the PikaScript compiler/bundler. **After editing any `.pyi` or `.py` you must re-run this** to regenerate `pikascript-api/` bindings and bytecode, then rebuild the firmware. There is no prebuild hook in the Keil project (`<BeforeMake>` is empty), so regeneration is manual.
- `rust-msc-latest-win10.exe` — underlying rust-based Pika compiler.
- `requestment.txt` — PikaScript package versions (pikascript-core==v1.13.4, …).
- `main.py` — the default Python entry source.

At runtime, user programs are **not** compiled on-device. `runPython()` in [Drivers/DataFile/entery/entery.c](Drivers/DataFile/entery/entery.c) loads a pre-compiled `.o` bytecode file from external flash (FATFS path `1:app/<uiListNum>.o`) and executes it via `pikaVM_runByteCodeInconstant`. Files are downloaded to flash over USB/Bluetooth by `USB_Download_Task`.

## Boot & runtime architecture

Single entry chain; understanding this is prerequisite to touching anything:

1. `main()` ([Core/Src/main.c](Core/Src/main.c)) — enables I/D-cache, HAL init, clocks (480 MHz), `freeRtosHeapMemInit()` (defines the FreeRTOS heap across two regions: 64 KB DTCM + 256 KB AXI, see `ucHeapAXI`/`ucHeapDCM` in [matchineState.c](Drivers/DataFile/machine/matchineState.c)), then `NEWAI_CreatePowerOnStartTask()`.
2. `NEWAI_CreatePowerOnStartTask()` ([matchineState.c](Drivers/DataFile/machine/matchineState.c)) — creates the `MatChineStateTask`, the global event group `xEventGroup`, the shared mutexes (`xUsbMutex`, `xRunPythonMutex`, `xRefreshMutex`, `xFsMutex`), calls `MultiUart_Init()`, then `vTaskStartScheduler()`.
3. `MatChineStateTask` — the central state machine. It spawns `USB_Download_Task` and `EnteryTask`, runs `bspInit()` (all peripheral bring-up), sets up the LED matrix UI, then creates a set of **periodic FreeRTOS software timers** (key scan 10 ms, port scan 50 ms, monitor 30 ms, battery 100 ms, bluetooth 200 ms, IWDG feed 50 ms, motion/mem 20 ms) and finally `MX_USB_DEVICE_Init()`. Its main loop blocks on `xEventGroupWaitBits` and dispatches `EVENT_*` bits to refresh the matrix UI, send monitor data, scan ports, run UI entry, etc.
4. `EnteryTask` — waits on `EVENT_RUN_PYTHON`, takes `xRunPythonMutex`, calls `runPython()`. `EVENT_RUN_PYTHON` is set from `ui_entery()` when the user picks a UI slot `< 20` (or `REMOTRE_LOGO`). Calling it again while running triggers `pks_vm_exit()` to stop the active script.

### Concurrency model
- One global `EventGroupHandle_t xEventGroup` carries all `EVENT_*` flags (defined in [matchineState.h](Drivers/DataFile/machine/matchineState.h)). Use `SET_EVENT_GROUP()` / `SET_EVENT_GROUP_ISR()` to signal; never invent a second event group.
- `usbDownloadActive` is a global gate: when a USB/Bluetooth file download is in progress, the state-machine timers skip sensor/UI refresh and port scanning. Respect this when adding timer callbacks.
- FreeRTOS heap is the only general allocator inside tasks (`pvPortMalloc`/`vPortFree`). Older `mymalloc(SRAMIN, …)` calls are commented out in favor of `pvPortMalloc` — follow that pattern.

## Device / port abstraction

Sensors (motor, gray, touch, ultrasion, color, camer, grayv2, nfc) are addressed through a multi-UART device framework in [matchineState.c](Drivers/DataFile/machine/matchineState.c):

- `vDevControlTask` / `vDevUartSendTask` run per UART device, draining `devControlQueue` / `devTxQueue`.
- Each device is a `SensorBase*` with a `type` field (`DEV_ID_BIG_MOTOR`, `DEV_ID_GRAY`, `DEV_ID_TOUCH`, …) dispatched in `vDevControlTask` to `refsh_motor` / `refsh_gray` / …
- The wire protocol lives in [Drivers/DataFile/portAgree](Drivers/DataFile/portAgree) (`_AGREEMENT` frame: Head / sID / oID / length / index / data[256] / crc / tard). `FindProtDev()` (triggered by `EVENT_FIND_PORT_DEV`) scans ports to detect attached devices.
- Python modules (`_motor`, `_gray`, …) are thin bindings over these C device modules — the real logic is in `Drivers/DataFile/<device>/` and `python/pikascript-lib/<module>/_<module>.c`.

## Directory map (non-obvious parts)

- `Core/` — STM32 HAL boilerplate, `main.c`, `pika_config.*` (PikaScript platform glue).
- `Drivers/BSP/` — low-level peripheral BSP (adc, dac, gpio, iic, iwdg, ospi, spi, tim, uart).
- `Drivers/User/` — higher-level drivers: `w25qxx` (external QSPI flash holding FATFS), `matrix` (LED matrix display), `key`, `music`, `os`.
- `Drivers/DataFile/` — the application framework. Key subdirs: `machine` (state machine + tasks + heap), `entery` (Python runner + UI entry), `portAgree` (port protocol), `download` (file ingest to flash), `blue` (Bluetooth), `motor`/`gray`/`touch`/`ultrasion`/`color`/`camer`/`grayv2`/`nfc` (device modules), `mem` (motion/IMU mem), `remote`, `message`, `filter`, `cjson`, `ringbufer`, `dataStruct`, `valueType`, `rawMatrix`, `list`, `strlist`.
- `Middle/` — `SYSTEM`, `DELAY`, `MALLOC`, `File_IO`, `AI` (X-CUBE-AI runtime headers + `Lib/NetworkRuntime1020_CM7_Keil.lib`), `PID_CONTROL`, `CTL_CMD`, `DRIVER_CONTROL/motor_control`.
- `X-CUBE-AI/App` — ST-generated neural network (`network.c/h`, `network_data*.*`). Regenerated by CubeMX/X-CUBE-AI, not by hand.
- `STM32_MotionFX_Library` — precompiled ST library; consumed by `lsm6ds3tr_c_motion_fx_determin()` (called on `EVENT_MEM_REFRESH`).
- `USB/` — STM32 USB CDC device stack. `python/` and `FreeRTOS/`, `FATFS/` are upstream vendored — avoid editing in place where possible.
- `MDK-ARM/` — Keil project, scatter file, startup `startup_stm32h723xx.s`.

## Working in this repo

- When changing native behavior exposed to Python: edit `python/pikascript-lib/<mod>/_<mod>.c` (and/or the `Drivers/DataFile/<device>` C it calls), re-run `python/pikaPackage.exe`, then rebuild in Keil. Editing a `.pyi` without re-running pikaPackage leaves the C bindings stale.
- When adding a new periodic action: create a FreeRTOS software timer in `MatChineStateTask` (mirroring the existing `xTimerCreate` + `xTimerStart` pattern) and signal work to the main loop via a new `EVENT_*` bit rather than doing the work in the timer callback.
- Source files use GBK-encoded Chinese comments in places. Preserve encoding when editing existing C files to avoid mojibake in Keil.
