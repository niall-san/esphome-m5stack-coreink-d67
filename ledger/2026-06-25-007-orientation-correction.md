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

Not captured for this flash.

## Physical Screen Result

### Second flash (commit bb7532e — 180° transform, both axes)

- Image was right-side up but text was horizontally mirrored.
- Root cause: "upside down and mirrored" from first flash was actually Y-only.
  Upside-down text reads right-to-left so it appeared mirrored, but X was
  always correct. The 180° fix introduced a spurious X flip.

### Third flash (commit b9f3ccd — Y-only flip)

- Rendered perfectly.
- Physical white background.
- Black border rectangle around the 200x200 screen.
- Text upright and readable from normal CoreInk orientation.
- Black filled rectangle with upright white `BLACK` text.

## Conclusion

The physical D67 panel only requires Y inversion (`pos = x + (HEIGHT-1-y)*WIDTH`).
X is correct in the native data entry mode (X-increment). The gate driver scans
bottom-to-top in the CoreInk mounted orientation, which is the sole transform
needed.

## Next Step

Wi-Fi/OTA smoke tests: replace validation-only secrets with real credentials,
reflash over USB, then exercise OTA update and ten automatic 60-second refreshes.
