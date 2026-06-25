# 2026-06-25 003 ESPHome Install And Compile

## Goal

Install ESPHome locally using the official Linux manual install route and run
static validation against the minimal CoreInk D67 example.

## Hypothesis

If the external component is structurally compatible with ESPHome `2026.6.2`,
`esphome config` and `esphome compile` should complete before any hardware
testing.

## Exact Config And Code State

Validation target:

```bash
examples/minimal-coreink.yaml
```

Local validation-only secrets were created outside Git tracking:

```text
secrets.yaml
examples/secrets.yaml
```

Local ESPHome install:

```bash
python3 -m venv .venv
.venv/bin/pip install esphome
```

The command installed ESPHome `2026.6.2`, matching the current ESPHome docs page
used for the install flow.

## Build Result

Passed:

```bash
python scripts/preflight.py
.venv/bin/esphome version
.venv/bin/esphome config examples/minimal-coreink.yaml
.venv/bin/esphome compile examples/minimal-coreink.yaml
```

Observed ESPHome version:

```text
Version: 2026.6.2
```

Compile completed successfully:

```text
SUCCESS Took 410.63 seconds
INFO Successfully compiled program.
```

Firmware artifacts were produced under:

```text
examples/.esphome/build/m5stack-core-ink-d67-test/.pioenvs/m5stack-core-ink-d67-test/
```

Notable artifacts:

- `firmware.bin`
- `firmware.factory.bin`
- `firmware.ota.bin`
- `firmware.elf`

Warnings observed during config/compile:

- GPIO9 may already be used by flash interface in QUAD IO flash mode.
- GPIO15 is a strapping pin.
- GPIO12 is a strapping pin.
- PlatformIO warned about reading `/usr/bin/python3.14` as a symlink, but the
  build continued and succeeded.

## Flash Method

Not flashed.

## Observed Logs

No device logs collected. Only local build logs were observed.

## Physical Screen Result

Not tested.

## Photo Placeholder

No photos collected.

## Conclusion

The component validates and compiles with ESPHome `2026.6.2`. The next blocker
is no longer basic ESPHome compatibility; it is hardware behaviour and whether
the SSD1681 init/update sequence actually drives the physical panel correctly.

## Next Step

Before flashing, compare the driver command sequence against the Good Display
`GDEY0154D67` ESP32/Arduino sample code and SSD1681 datasheet. Then flash the
minimal test firmware with USB power available and record the physical result.

