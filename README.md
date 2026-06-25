# m5stack-coreink-d67-esphome

ESPHome external component for M5Stack CoreInk units containing the **D67 panel variant** (physically marked `0154BN-D67-D2`).

## Background

Some M5Stack CoreInk units shipped with a Good Display `GDEY0154D67` / `SSD1681` panel rather than the `GDEW0154M09` panel that ESPHome's built-in `1.54in-m5coreink-m09` model targets. The built-in model does not update the D67 panel. This component implements the correct full-refresh and partial-refresh sequences for it.

## Does this apply to your CoreInk?

Remove the back cover and look at the panel markings. This component is for units marked **`0154BN-D67-D2`**. The built-in ESPHome model targets units marked `GDEW0154M09`.

If you have the D67 variant you may have already noticed:
- `model: 1.54in-m5coreink-m09` does nothing.
- `model: 1.54inv2` updates the panel but `Color::WHITE` renders black and vice versa.

## Installation

Add to your ESPHome config:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/niall-san/m5stack-coreink-d67-esphome
    components: [m5stack_coreink_d67]
```

Or clone locally and reference the path:

```yaml
external_components:
  - source: /path/to/m5stack-coreink-d67-esphome/components
    components: [m5stack_coreink_d67]
```

## Minimal configuration

```yaml
spi:
  clk_pin: GPIO18
  mosi_pin: GPIO23

display:
  - platform: m5stack_coreink_d67
    id: my_display
    cs_pin: GPIO9
    update_interval: 60s
    lambda: |-
      it.fill(Color::WHITE);
      it.print(10, 10, id(my_font), Color::BLACK, "Hello CoreInk");
```

All display drawing works with normal ESPHome color semantics: `Color::WHITE` is physically white, `Color::BLACK` is physically black.

## Configuration options

| Option | Default | Description |
|---|---|---|
| `cs_pin` | — | SPI chip select (required) |
| `dc_pin` | `GPIO15` | Data/command select |
| `busy_pin` | `GPIO4` | Panel busy signal (active high) |
| `power_hold_pin` | `GPIO12` | CoreInk power latch — held high during setup |
| `reset_pin` | — | Optional hardware reset |
| `reset_duration` | `10ms` | Reset pulse width |
| `full_update_every` | `30` | Full refresh every N updates; all others use fast partial refresh |
| `update_interval` | — | Standard ESPHome polling interval |
| `lambda` | — | Standard ESPHome display lambda |

## Partial refresh

By default, the first update after boot is a full refresh (~2 seconds, with the characteristic e-ink flicker). Subsequent updates use a fast partial waveform (~300 ms, no full flicker) until `full_update_every` cycles have elapsed, at which point a full refresh clears accumulated ghosting.

```yaml
display:
  - platform: m5stack_coreink_d67
    update_interval: 60s
    full_update_every: 10   # full refresh every 10 minutes
```

Set `full_update_every: 1` to always use full refresh.

## Hardware pins

| Function | GPIO | Notes |
|---|---:|---|
| SPI CLK | GPIO18 | |
| SPI MOSI | GPIO23 | |
| Display CS | GPIO9 | |
| Display DC | GPIO15 | Component default |
| Display BUSY | GPIO4 | Active high; component default |
| Power hold | GPIO12 | Must be held high; component default |
| Dial up | GPIO37 | Input only — wire to `binary_sensor` for page navigation |
| Dial down | GPIO39 | Input only |
| User button | GPIO5 | Input only |

Do not use `GPIO0` as a reset pin — it caused unreliable behaviour during testing.

## Power hold

The CoreInk's power latch (`GPIO12`) must be held high or the board loses power. This component asserts it during `setup()` before anything else. The pin is configurable via `power_hold_pin` in case a future board revision changes it.

## Validation

```bash
python scripts/preflight.py
.venv/bin/esphome config examples/minimal-coreink.yaml
.venv/bin/esphome compile examples/minimal-coreink.yaml
```

Tested against ESPHome `2026.6.2`.

## Examples

- [`examples/minimal-coreink.yaml`](examples/minimal-coreink.yaml) — bare minimum for flashing and verifying the display.
- [`examples/home-assistant-dashboard.yaml`](examples/home-assistant-dashboard.yaml) — three-page Home Assistant display with dial navigation.

## Limitations

- Partial refresh uses a waveform LUT from the SSD1681 controller family (cross-checked with Good Display samples). Minor ghosting accumulates between full refreshes — this is inherent to partial e-ink updates and is cleared on each full refresh cycle.
- Deep sleep is not implemented. Entering deep sleep normally requires a hardware reset to wake; the CoreInk's reset pin behaves unreliably in this role, so the panel stays awake between updates. Power consumption is higher than deep-sleep designs as a result.
- The component targets the `0154BN-D67-D2` panel specifically. It may work on other `SSD1681`-based CoreInk panels but has only been validated on this marking.

## Upstream

This component is a candidate for upstreaming to ESPHome. See [`docs/provenance.md`](docs/provenance.md) for the checklist of what is needed before opening a PR.

## License

MIT — see [`LICENSE`](LICENSE).
