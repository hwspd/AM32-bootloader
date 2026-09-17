# GD32F350 bootloader

This port targets the 64 KiB flash, 8 KiB SRAM GD32F350x8 used by the AM32
`REF_F350` target. The one-wire bootloader input is PB4.

## Build

```sh
make AM32_F350_BOOTLOADER_PB4
make AM32_F350_BL_UPDATER_PB4
```

The bootloader output is linked at `0x08000000`. The updater is linked as an
application at `0x08001000` and contains the bootloader image it will install.

## Memory map

| Region | Address range |
|---|---|
| Bootloader | `0x08000000-0x08000FFF` |
| Application | `0x08001000-0x0800F7DF` |
| Firmware name | `0x0800F7E0-0x0800F7FF` |
| EEPROM | `0x0800F800-0x0800FFFF` |

The F350 FMC uses 1 KiB erase pages. `openocd.cfg` fixes the bank at 64 KiB
and uses a 4 KiB RAM work area so OpenOCD reports 64 sectors of 1 KiB.

## Hardware validation

The PB4 build was validated on a GD32F350 ESC with a CMSIS-DAP SWD probe and
a Betaflight four-way bridge on 2026-09-17. The following operations passed:

- SWD programming and readback verification.
- Bootloader-to-application jump at `0x08001000`.
- Four-way device initialization with signature `0x3506` and pin code `0x14`.
- Protocol-v3 devinfo, firmware-name, and EEPROM reads.
- Application page erase, four 256-byte writes, and readback verification.
- EEPROM disable/restore writes and readback verification.
- Bootloader updater transfer, execution, and exact bootloader rewrite.
- Recovery into the standard `REF_F350` Preview 3 application.

The updater intentionally resets continuously after installing a matching
bootloader because it remains in the application region. Flash a normal AM32
application after running the updater.

The device SVD is based on `GigaDevice.GD32F3x0_DFP.3.0.2.pack`, with CPU
metadata corrected to match the GD32F350 device header.
