# Testing Plan

## Static Validation

Run from the repository root:

```bash
python scripts/preflight.py
.venv/bin/esphome config examples/minimal-coreink.yaml
.venv/bin/esphome compile examples/minimal-coreink.yaml
```

Expected baseline version:

```text
ESPHome 2026.6.2
```

Current static result:

- `python scripts/preflight.py`: passed.
- `.venv/bin/esphome version`: `2026.6.2`.
- `.venv/bin/esphome config examples/minimal-coreink.yaml`: passed.
- `.venv/bin/esphome compile examples/minimal-coreink.yaml`: passed.
- First compile warnings: GPIO9 flash-interface warning, GPIO15 strapping-pin
  warning, GPIO12 strapping-pin warning. These are expected for CoreInk wiring
  but must be kept in mind during hardware testing.

## Hardware Validation

Record each run in `ledger/` with the exact YAML, firmware build result, flash
method, logs, and physical screen result.

Required smoke tests:

- Full-screen white.
- Full-screen black.
- Border and text.
- Checkerboard.
- Manual refresh button.
- Three-page Home Assistant display.
- Ten automatic refreshes at 60 second intervals.
- OTA update followed by reboot and power-latch recovery.
- Watchdog stability during setup and display updates.

## Pass Criteria For First Milestone

- The panel updates reliably on the confirmed CoreInk unit.
- `Color::WHITE` renders physically white.
- `Color::BLACK` renders physically black.
- No `reset_pin` is needed.
- No setup watchdog reboot.
- No display update timeout during normal 60 second refresh.

### Status (2026-06-25)

All visual criteria met. Confirmed on commit `b9f3ccd` (Y-only orientation fix).
Remaining: Wi-Fi/OTA smoke tests and ten-refresh stability run.

## Known Risks

- The full-refresh command sequence has been aligned with the official Good
  Display `GDEY0154D67` ESP32 sample, but it has not yet been flashed to the
  CoreInk hardware.
- The vendor examples deep-sleep after updates. This component does not do that
  by default because the known CoreInk wiring has no reset pin.
- `cs_pin` is still explicit in examples until the SPI schema/default behaviour
  is confirmed under ESPHome `2026.6.2`.
