# M5Stack CoreInk D67 ESPHome Component

Local-first ESPHome external component for the M5Stack CoreInk panel variant
physically marked `0154BN-D67-D2`.

The panel behaves like a Good Display `GDEY0154D67` / `SSD1681` family
1.54 inch, 200x200, black-and-white SPI e-paper display rather than the older
CoreInk `GDEW0154M09` panel documented by ESPHome.

## Status

Experimental. This repository is intentionally local-first until the component
has been validated on the kitchen CoreInk hardware.

Known baseline:

- ESPHome version used during discovery: `2026.6.2`
- Current workaround: `waveshare_epaper` with `model: 1.54inv2`
- Broken documented model: `1.54in-m5coreink-m09`
- Confirmed panel marking: `0154BN-D67-D2`

## Repository Layout

- `components/m5stack_coreink_d67/`: ESPHome external component.
- `examples/`: minimal and Home Assistant-oriented example YAML.
- `docs/`: hardware, references, and testing notes.
- `ledger/`: chronological experiment log detailed enough for handoff.
- `scripts/preflight.py`: dependency-free repository sanity checks.

## Minimal Usage

When this repository is next to or copied into an ESPHome config directory:

```yaml
external_components:
  - source: ./m5stack-coreink-d67-esphome/components
    components: [m5stack_coreink_d67]

spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23

display:
  - platform: m5stack_coreink_d67
    id: coreink_display
    cs_pin: GPIO9
    dc_pin: GPIO15
    busy_pin: GPIO4
    power_hold_pin: GPIO12
    update_interval: 60s
    lambda: |-
      it.fill(Color::WHITE);
      it.print(10, 10, id(helvetica_20), Color::BLACK, "CoreInk D67");
```

The component is intended to expose normal display color semantics:
`Color::WHITE` should render physically white and `Color::BLACK` should render
physically black.

## Validation

ESPHome `2026.6.2` was installed into a local `.venv` following the official
Linux manual install flow. The minimal example validates and compiles.

Run the local preflight before ESPHome validation:

```bash
python scripts/preflight.py
```

Then, once ESPHome is available:

```bash
.venv/bin/esphome config examples/minimal-coreink.yaml
.venv/bin/esphome compile examples/minimal-coreink.yaml
```

The first compile produced `firmware.bin`, `firmware.factory.bin`, and
`firmware.ota.bin` under `examples/.esphome/build/...`; these build artifacts
are intentionally ignored.

## Provenance

This repository is AI-assisted and should remain evidence-led. See
`docs/provenance.md` for the policy around vendor references, hardware proof,
and eventual upstream readiness.
