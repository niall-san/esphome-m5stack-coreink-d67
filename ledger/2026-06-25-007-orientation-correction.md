# 2026-06-25 007 - Orientation Correction

## Goal

Record the first physical screen result and correct the logical orientation so
ESPHome drawing lambdas render normally on the CoreInk-mounted D67 panel.

## Hypothesis

The first USB flash rendered clearly, which confirms the display bus, color
polarity, BUSY handling, and full-refresh sequence are broadly correct. The
reported upside-down and mirrored image can be corrected by rotating logical
pixel writes 180 degrees before they enter the SSD1681 RAM buffer.

## Exact Config And Code State

Previous flashed commit:

- `ed0a957 Record first CoreInk USB flash`

Observed physical result from first flash:

- Image rendered clearly.
- Image was upside down and mirrored.

Code change:

- `draw_absolute_pixel_internal()` now maps logical ESPHome coordinates to RAM
  coordinates with:
  - `ram_x = WIDTH - 1 - x`
  - `ram_y = HEIGHT - 1 - y`
- The vendor-aligned SSD1681 command sequence is unchanged.
- Color semantics remain unchanged:
  - `Color::WHITE` writes RAM bit `1`.
  - `Color::BLACK` writes RAM bit `0`.

## Build Result

- `python scripts/preflight.py`: passed.
- `.venv/bin/esphome config examples/minimal-coreink.yaml`: passed.
- `.venv/bin/esphome compile examples/minimal-coreink.yaml`: passed.
  - ESPHome 2026.6.2
  - RAM: 14.0% (45948 / 327680 bytes)
  - Flash: 40.2% (738323 / 1835008 bytes)
  - Build time: 18.66 s

## Flash Method

Not flashed yet.

Planned command:

```bash
.venv/bin/esphome run examples/minimal-coreink.yaml --device /dev/serial/by-id/usb-1a86_USB_Single_Serial_5A6C011853-if00
```

## Observed Logs

No logs for the corrected build yet.

## Physical Screen Result

Pending second flash.

Expected corrected result:

- Physical white background.
- Black border rectangle around the 200x200 screen.
- Text upright and readable from normal CoreInk orientation.
- Black filled rectangle with upright white `BLACK` text.

## Photo Placeholder

- `photos/2026-06-25-007-orientation-correction/`

## Conclusion

The first hardware flash was electrically successful but orientation was wrong.
A 180-degree logical coordinate transform is the smallest correction because it
does not alter the now-proven full-refresh command path.

## Next Step

Compile and flash the correction over USB, then record the second physical
screen result.
