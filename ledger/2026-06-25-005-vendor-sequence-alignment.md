# 2026-06-25 005 - Vendor Sequence Alignment

## Goal

Compare the local `m5stack_coreink_d67` driver against official Good Display
`GDEY0154D67` reference material before flashing the CoreInk hardware.

## Hypothesis

The initial driver compiled, but its SSD1681 setup still contained guessed
details. Aligning the full-refresh RAM addressing and update sequence with the
official Good Display ESP32 sample should reduce first-flash risk.

## Exact Config And Code State

Repository before this entry:

- Commit: `805b9b4 Record initial repository commit in ledger`
- ESPHome version: `2026.6.2`
- Example config: `examples/minimal-coreink.yaml`
- Display platform: `m5stack_coreink_d67`
- Pins:
  - CLK: `GPIO18`
  - MOSI: `GPIO23`
  - CS: `GPIO9`
  - DC: `GPIO15`
  - BUSY: `GPIO4`, non-inverted
  - power hold: `GPIO12`
  - reset: omitted

Vendor material inspected:

- Good Display page: https://www.good-display.com/companyfile/1128.html
- Page title: `GDEY0154D67 ESP32 Sample Code`
- Page file entry: `A32-GDEY0154D67`
- Page issue time: `2025-02-18 15:21:02`
- Download endpoint:
  `/comp/xcompanyFile/downloadNew.do?appId=24&fid=1777&id=1128`
- CDN target observed:
  `https://v4.cecdn.yun300.cn/100001_1909185148/A32-GDEY0154D67.rar`
- Extracted local folder:
  `/tmp/coreink-d67-vendor/esp32/A32-GDEY0154D67/A32-GDEY0154D67-FP4G-20250212/`

Vendor full-refresh findings:

- BUSY is active high and idle low.
- Full hardware reset uses low for at least 10 ms, then high for at least
  10 ms.
- SW reset command is `0x12`, followed by BUSY wait.
- Driver output control is `0x01`, data `199`, `0`, `0`.
- Full-refresh data entry mode is `0x11`, data `0x01`.
- RAM X range is `0` to `24`.
- RAM Y range is `199` down to `0`.
- Border waveform is `0x3C`, data `0x05`.
- Built-in temperature sensor command is `0x18`, data `0x80`.
- RAM X counter starts at `0`.
- RAM Y counter starts at `199`.
- Full B/W RAM write command is `0x24`.
- Full update command is `0x22`, data `0xF7`, then command `0x20`.
- `0xff` is physical white; `0x00` is physical black.
- Vendor examples deep-sleep with `0x10`, data `0x01`.

Driver changes made:

- Changed data entry mode from `0x03` to vendor-aligned `0x01`.
- Changed RAM Y range from `0..199` to `199..0`.
- Changed RAM Y counter from `0` to `199`.
- Removed the extra `0x21` display update control write because it was not in
  the inspected vendor full-refresh path.
- Documented that automatic deep sleep remains disabled because our known
  CoreInk wiring has no reset pin and SSD1681-family deep sleep normally needs
  hardware reset to wake.

## Build Result

Static validation after the vendor-alignment edit:

- `python scripts/preflight.py`: passed.
- `.venv/bin/esphome config examples/minimal-coreink.yaml`: passed.
- `.venv/bin/esphome compile examples/minimal-coreink.yaml`: passed with
  ESPHome `2026.6.2`.

Compile notes:

- Initial sandboxed compile failed because PlatformIO attempted to write
  `/home/niall/.platformio/platforms.lock`, which is outside the workspace
  sandbox. The same command succeeded when rerun with filesystem access.
- Expected CoreInk wiring warnings remain:
  - GPIO9 flash-interface warning.
  - GPIO15 strapping-pin warning.
  - GPIO12 strapping-pin warning.
- Firmware artifacts were regenerated under
  `examples/.esphome/build/m5stack-core-ink-d67-test/`.

## Flash Method

Not flashed.

## Observed Logs

No hardware logs. Vendor archive extraction and source inspection only.

## Physical Screen Result

Not tested on the physical screen.

## Photo Placeholder

Photo placeholder:

- `photos/2026-06-25-005-vendor-sequence-alignment/`

## Conclusion

The local driver now follows the official Good Display full-refresh sequence
more closely than the original conservative SSD1681 guess. This should be the
first version considered for hardware flashing, subject to static validation.

## Next Step

Commit the vendor-alignment change, then flash the minimal firmware to the
CoreInk and record the first physical display result.
