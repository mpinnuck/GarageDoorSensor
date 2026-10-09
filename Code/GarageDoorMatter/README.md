# GarageDoorMatter

Matter (Wi-Fi) contact sensor firmware for the garage door sensor.
Seeed XIAO ESP32C3, door reed on D1 (GPIO3), reed closed = door shut.

## Build (VS Code + PlatformIO)
For a single VS Code window with both the repository and firmware available,
open `GarageDoorSensor.code-workspace` from the repository root. The firmware
folder is included as a separate workspace folder so PlatformIO can find its
`platformio.ini`.

You can also open this folder directly when working only on the firmware.
Uses the pioarduino platform (Arduino ESP32 core 3.x, which has the Matter library).

Run PlatformIO CLI commands from this folder. Erase only on the first flash, or
to force a re-pair, because it wipes the Apple Home pairing:

```sh
pio run -t erase     # first flash only (or to force a re-pair)
pio run -t upload
pio device monitor
```

If the board is not detected, hold BOOT while plugging in USB.

## Pairing
Serial monitor prints a manual pairing code and a QR code link.
- Apple Home: Add Accessory, scan the QR code or enter the code.
- Google Home: in Apple Home, accessory settings > Turn On Pairing Mode,
  then add the new code in Google Home > Add device > Matter.

Hold BOOT for 5 s to decommission (factory reset Matter).
