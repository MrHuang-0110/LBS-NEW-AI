# AGENTS.md

Firmware for an STM32H723 (Cortex-M7 @ 480 MHz) educational robotics controller: bare C on STM32 HAL + FreeRTOS, with PikaScript (Python-on-MCU) runtime, ST X-CUBE-AI inference, MotionFX sensor fusion, FATFS on external QSPI flash, and USB CDC.

## Read this first

**`CLAUDE.md` is the authoritative deep-dive** — boot/event/concurrency model, wire protocol, device modules, PikaScript toolchain, and directory map. Read it before touching anything. This file only covers what you need up front.

## Build

- **No CLI build, no test suite, no linter.** Build with Keil µVision: open `MDK-ARM/STM32H723.uvprojx`, build target `STM32H723`. `compile_flags.txt` is only for clangd/IDE intellisense.
- After-build runs `fromelf` (→ `.bin`) and `postbuild.bat`, which copies the `.bin` to a **hardcoded external path** `E:\LBS-Project\NEW-AI-PROJECT\...` — edit `postbuild.bat` if the checkout lives elsewhere.
- `keilkilll.bat` cleans Keil intermediates (keeps `*.opt` JLink settings).

## PikaScript toolchain (the #1 gotcha)

User-facing Python API is `*.pyi` stubs in `python/`. After editing **any** `.pyi`/`.py` you must re-run `python/pikaPackage.exe` to regenerate `python/pikascript-api/` bindings and bytecode, then rebuild in Keil. There is **no prebuild hook** — regeneration is manual. Native C behavior lives in `python/pikascript-lib/<mod>/_<mod>.c` (hand-written), which calls into `Drivers/DataFile/<device>/`. User programs run as pre-compiled `.o` bytecode loaded from external flash (`1:app/<uiListNum>.o`) — never compiled on-device.

## Critical constraints

- **Flash offset `0x08020800`** — bootloader occupies the first 0x20800 bytes; `main()` sets `SCB->VTOR` accordingly. Respect it in scatter file / vector table changes.
- **Memory:** FreeRTOS heap spans `ucHeapDCM` (64 KB DTCM) + `ucHeapAXI` (256 KB AXI). `pvPortMalloc`/`vPortFree` only — never stdlib `malloc`/`free`. Note: SRAM2 overflows easily (see camer fix commit) — move large structs to AXI RAM if the linker complains.
- **Encoding:** C files contain GBK-encoded Chinese comments. Preserve GBK when editing — do not convert to UTF-8.
- **Events:** single event group `xEventGroup` (bits listed in CLAUDE.md). Use `SET_EVENT_GROUP()` / `SET_EVENT_GROUP_ISR()`; never add a second event group. New periodic work = new FreeRTOS software timer in `MatChineStateTask` + new `EVENT_*` bit, never work inside the timer callback.
- **Device IDs:** sensors `0xA1`–`0xB0`; USB port `0x09`, Bluetooth `0x0A`. New device type needs a `DEV_ID_*`, registration in `identify_and_bind()` + `vDevControlTask`, and a Python module binding.
- **Monitor JSON** goes out over USB CDC every 30 ms (`EVENT_SEND_MONITOR` → `newAiMonitor()` in `portagree.c`); suppressed during downloads (`usbDownloadActive` gate) — respect that gate.

## Docs

- `docs/superpowers/` holds design specs and plans for in-flight work (e.g. camera monitor JSON, dynamic label mode). **Read the relevant spec before touching `portagree.c` / `camer`.** The old root-level `通讯协议.txt` was deleted — the camera protocol spec now lives in `docs/superpowers/specs/`.

## Git state

- Default branch: `main`; feature work happens on `main-work` and is merged back into `main`.
- Recent work (committed): camera module (`camer`/`portagree`) — frame parsing, dynamic-N monitor JSON, `cam_count` API, name-frame (0x0E) support with per-slot name/learned fields in monitor JSON, multi-frame-per-DMA-batch parsing; plus a `time.sleep()` fix routing it through `pika_platform_sleep_ms` (vTaskDelay + GIL release + VM exit check) so the middle key can stop a running script promptly — see `python/pikascript-lib/time/_time.c`.
