#!/usr/bin/env python3
"""Dependency-free repository preflight checks.

This script intentionally does not validate ESPHome semantics. Its job is to
catch broken scaffold state before running `esphome config` or flashing
hardware.
"""

from __future__ import annotations

import py_compile
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "README.md",
    "components/m5stack_coreink_d67/__init__.py",
    "components/m5stack_coreink_d67/display.py",
    "components/m5stack_coreink_d67/m5stack_coreink_d67.cpp",
    "components/m5stack_coreink_d67/m5stack_coreink_d67.h",
    "docs/hardware.md",
    "docs/references.md",
    "docs/testing.md",
    "examples/minimal-coreink.yaml",
    "examples/home-assistant-dashboard.yaml",
    "ledger/2026-06-25-001-baseline.md",
]

LEDGER_REQUIRED_HEADINGS = [
    "## Goal",
    "## Hypothesis",
    "## Exact Config And Code State",
    "## Build Result",
    "## Flash Method",
    "## Observed Logs",
    "## Physical Screen Result",
    "## Photo Placeholder",
    "## Conclusion",
    "## Next Step",
]


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    sys.exit(1)


def check_required_files() -> None:
    missing = [path for path in REQUIRED_FILES if not (ROOT / path).is_file()]
    if missing:
        fail("missing required files: " + ", ".join(missing))


def check_python_syntax() -> None:
    for path in [
        ROOT / "components/m5stack_coreink_d67/__init__.py",
        ROOT / "components/m5stack_coreink_d67/display.py",
    ]:
        py_compile.compile(str(path), doraise=True)


def check_component_strings() -> None:
    display_py = (ROOT / "components/m5stack_coreink_d67/display.py").read_text()
    component_cpp = (ROOT / "components/m5stack_coreink_d67/m5stack_coreink_d67.cpp").read_text()
    readme = (ROOT / "README.md").read_text()

    for text, label in [
        (display_py, "display.py"),
        (component_cpp, "m5stack_coreink_d67.cpp"),
        (readme, "README.md"),
    ]:
        if "m5stack_coreink_d67" not in text:
            fail(f"{label} does not mention m5stack_coreink_d67")

    for required in ["GPIO12", "GPIO15", "GPIO4", "GPIO9", "0154BN-D67-D2", "GDEY0154D67", "SSD1681"]:
        if required not in readme and required not in component_cpp and required not in display_py:
            fail(f"missing expected identifier: {required}")


def check_examples() -> None:
    for path in [
        ROOT / "examples/minimal-coreink.yaml",
        ROOT / "examples/home-assistant-dashboard.yaml",
    ]:
        text = path.read_text()
        for required in [
            "external_components:",
            "platform: m5stack_coreink_d67",
            "power_hold_pin: GPIO12",
            "busy_pin: GPIO4",
            "dc_pin: GPIO15",
            "cs_pin: GPIO9",
        ]:
            if required not in text:
                fail(f"{path.name} missing `{required}`")


def check_ledger_shape() -> None:
    ledger_files = sorted((ROOT / "ledger").glob("*.md"))
    if not ledger_files:
        fail("no ledger entries found")

    name_pattern = re.compile(r"^\d{4}-\d{2}-\d{2}-\d{3}-[a-z0-9-]+\.md$")
    for path in ledger_files:
        if not name_pattern.match(path.name):
            fail(f"ledger filename does not match convention: {path.name}")
        text = path.read_text()
        missing = [heading for heading in LEDGER_REQUIRED_HEADINGS if heading not in text]
        if missing:
            fail(f"{path.name} missing headings: {', '.join(missing)}")


def main() -> int:
    check_required_files()
    check_python_syntax()
    check_component_strings()
    check_examples()
    check_ledger_shape()
    print("PASS: repository preflight checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

