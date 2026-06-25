# 2026-06-25 006 - First USB Flash

## Goal

Flash the vendor-aligned `m5stack_coreink_d67` minimal firmware to the physical
M5Stack CoreInk over USB from the Bazzite laptop and capture the first boot
logs.

## Hypothesis

If the Good Display-aligned full-refresh sequence is suitable for the confirmed
`0154BN-D67-D2` panel, the firmware should boot without watchdog resets or BUSY
timeouts and should draw the minimal white/black/text screen.

## Exact Config And Code State

Repository state:

- Commit before flashing: `16cc6df Align D67 refresh sequence with vendor sample`
- Config flashed: `examples/minimal-coreink.yaml`
- ESPHome version: `2026.6.2`
- Display platform: `m5stack_coreink_d67`
- Display update interval: `60s`

USB/host state:

- Host OS: Bazzite laptop
- USB serial adapter observed by `lsusb`:
  `1a86:55d4 QinHeng Electronics USB Single Serial`
- Stable serial path:
  `/dev/serial/by-id/usb-1a86_USB_Single_Serial_5A6C011853-if00`
- Kernel device:
  `/dev/ttyACM0`
- Device permissions were temporarily opened with `chmod a+rw /dev/ttyACM0`
  because the immutable host exposes `dialout` from `/usr/lib/group` and the
  user was not a member of a local `/etc/group` entry.

Important config caveat:

- The local ignored `secrets.yaml` still used validation-only Wi-Fi credentials.
  This was acceptable for USB flash and serial boot logs but means OTA/API could
  not come online in this run.

## Build Result

`esphome run` rebuilt successfully before upload:

- Build path:
  `examples/.esphome/build/m5stack-core-ink-d67-test`
- Firmware binary:
  `firmware.bin`
- Compile result: success

## Flash Method

Command:

```bash
.venv/bin/esphome run examples/minimal-coreink.yaml --device /dev/serial/by-id/usb-1a86_USB_Single_Serial_5A6C011853-if00
```

Upload result:

- Connected chip: `ESP32-PICO-D4`, revision `v1.1`
- MAC: `00:4B:12:A0:FA:C0`
- Flash size auto-detected: `4MB`
- Main firmware write verified.
- Bootloader write verified.
- Partition table write verified.
- OTA data write verified.
- Hard reset via RTS succeeded.
- ESPHome reported `Successfully uploaded program`.

## Observed Logs

Boot/setup:

- Logger initialized.
- Safe mode unsuccessful boot attempts: `0`.
- `setup()` started and finished successfully.
- SPI device mode `0`, data rate `2000kHz`.
- Display setup took `70ms`.
- Display dimensions reported as `200px x 200px`.
- Pins reported:
  - DC: `GPIO15`
  - BUSY: `GPIO4`
  - power hold: `GPIO12`
- First display operation warning:
  `display took a long time for an operation (1843 ms), max is 50 ms`.
  This is expected for e-paper refresh timing and was not accompanied by a BUSY
  timeout.
- Safe mode marked boot successful after 60 seconds and reset the boot loop
  counter.

Wi-Fi/API:

- Wi-Fi scan started but could not find/connect to `validation-only`.
- API remained in warning state waiting for a client.
- OTA advertised `m5stack-core-ink-d67-test.local:3232`, but this was not
  reachable because Wi-Fi credentials were dummy values.

Errors not observed:

- No display BUSY timeout.
- No watchdog reboot.
- No safe-mode boot loop.

## Physical Screen Result

Pending user observation.

Expected test image:

- Physical white background.
- Black border rectangle around the 200x200 screen.
- Black text: `CoreInk D67` and `WHITE bg`.
- Black filled rectangle with white text: `BLACK`.

## Photo Placeholder

- `photos/2026-06-25-006-first-usb-flash/`

## Conclusion

The first USB flash succeeded and the firmware booted cleanly. Serial logs show
the custom display component initialized and refreshed without a BUSY timeout or
watchdog reset. The hardware screen result still needs to be recorded before
declaring the driver visually successful.

## Next Step

Ask for the physical screen result. If the screen image is correct, replace the
validation-only Wi-Fi secrets with real local credentials, rerun USB flashing
once, then test OTA and repeated 60-second refreshes. If the screen is blank,
inverted, shifted, or otherwise wrong, record the observed pattern and adjust
the command/addressing path accordingly.
