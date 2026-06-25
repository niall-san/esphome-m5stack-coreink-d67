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
Partial refresh added in `7ddb3f1`; base-RAM sync fixed in `567343f`.

Remaining hardware smoke tests:
- Wi-Fi with real credentials → OTA update → reboot and power-latch recovery.
- Ten automatic refreshes at 60 second intervals (full and partial paths).
- Three-page Home Assistant dashboard (dial/button navigation, sensor data).

## Known Risks

- `cs_pin` is still explicit in examples until the SPI schema/default behaviour
  is confirmed under ESPHome `2026.6.2`.
- Partial refresh accumulates minor ghosting (inherent to waveform LUT). Clears
  on full refresh every `full_update_every` cycles. Set to 10 in both examples.
- Partial refresh LUT voltage bytes were sourced from the 2.9in V2R2 ESPHome
  driver (same SSD1681 controller family). Not validated against the GDEY0154D67
  datasheet voltage spec; monitor for display degradation over time.
