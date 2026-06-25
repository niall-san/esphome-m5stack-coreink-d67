# Provenance And PR Notes

This repository is AI-assisted, but the target for any future upstream work is
evidence-led and reviewable.

## Hardware Facts

Confirmed by physical inspection:

- M5Stack CoreInk unit contains a panel marked `0154BN-D67-D2`.
- The documented ESPHome CoreInk M09 model does not update this panel.
- The ESPHome `waveshare_epaper` `1.54inv2` workaround does update the panel
  with inverted logical colors.

## Public Reference Material

Public docs indicate the closest known panel family is Good Display
`GDEY0154D67`, a 1.54 inch, `200x200`, black-and-white, SPI e-paper display
using an `SSD1681` controller.

Vendor sample code and datasheets should be used as references for command
meaning, timing, and initialization order. Do not copy non-trivial vendor code
into this repository unless its license permits it and the copied portion is
clearly attributed.

## Implementation Policy

- Keep each hardware experiment in `ledger/`.
- Treat untested command sequences as hypotheses.
- Prefer small commits with one behavioural change each.
- Record build logs and physical observations before changing the driver again.
- Keep examples minimal enough that maintainers can reproduce the display issue
  without Home Assistant entities.

## Upstream Readiness Checklist

- Hardware photos added to `docs/hardware.md` or linked from the ledger.
- Minimal ESPHome config validates and compiles.
- The component has passed at least ten 60 second refresh cycles.
- OTA recovery behaviour is documented.
- The init sequence has been cross-checked against the `SSD1681` datasheet and
  Good Display `GDEY0154D67` sample code.
- Differences from ESPHome `1.54in-m5coreink-m09` and `1.54inv2` are documented.
- Public license decision has been made before publishing the repo.

