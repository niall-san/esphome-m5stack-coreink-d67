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

The official `GDEY0154D67 ESP32 Sample Code` page exposes archive
`A32-GDEY0154D67.rar`, issued by the page as file id `1777`. That archive was
downloaded from the Good Display CDN and inspected locally as reference
material only. The full-refresh path in this component is a clean
implementation of the observed command order and address semantics, not copied
vendor source.

Vendor sample code and datasheets should be used as references for command
meaning, timing, and initialization order. Do not copy non-trivial vendor code
into this repository unless its license permits it and the copied portion is
clearly attributed.

## Vendor Full-Refresh Findings

The Good Display ESP32 sample for `GDEY0154D67` confirms:

- BUSY is active high and idle low.
- The B/W frame buffer is written with command `0x24`.
- `0xff` means physical white and `0x00` means physical black.
- Full refresh uses `0x22` data `0xF7`, then command `0x20`.
- Full-refresh data entry mode is `0x01`: X increments while Y decrements.
- RAM X range is `0` to `24`, representing `200 / 8 - 1`.
- RAM Y range starts at `199` and ends at `0`.
- RAM counters start at X `0`, Y `199`.
- Border waveform is `0x3C` data `0x05`.
- Built-in temperature sensor command is `0x18` data `0x80`.

The sample deep-sleeps the panel after updates with command `0x10` data `0x01`.
This component deliberately does not do that by default because the known
CoreInk wiring has no reset pin, and waking SSD1681-family panels from deep
sleep typically requires hardware reset. This should be revisited only after
hardware testing proves a safe wake path for the CoreInk board.

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
