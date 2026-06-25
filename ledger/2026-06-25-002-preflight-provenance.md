# 2026-06-25 002 Preflight And Provenance

## Goal

Make the scaffold easier for another engineer or LLM to pick up safely before
ESPHome is available locally.

## Hypothesis

A dependency-free preflight script and explicit provenance policy will reduce
handoff risk and make future upstream work more credible.

## Exact Config And Code State

Added:

- `scripts/preflight.py`
- `docs/provenance.md`

Updated:

- `README.md`
- `docs/testing.md`
- `ledger/2026-06-25-001-baseline.md`

No driver command sequence changes were made in this entry.

## Build Result

Dependency-free preflight passed:

```bash
python scripts/preflight.py
```

Output:

```text
PASS: repository preflight checks passed
```

ESPHome validation is still pending because `esphome` is not installed in this
shell.

## Flash Method

Not flashed.

## Observed Logs

No device logs collected.

## Physical Screen Result

Not tested.

## Photo Placeholder

No new photos.

## Conclusion

The repository now has a lightweight self-check and a documented policy for
AI-assisted, evidence-led development.

## Next Step

Install or access ESPHome `2026.6.2`, run `esphome config` on the minimal
example, then compare the SSD1681 init sequence against the Good Display
`GDEY0154D67` sample code before flashing hardware.

