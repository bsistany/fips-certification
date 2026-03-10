#!/usr/bin/env python3
"""
add_headers.py — Add copyright/SPDX header to C source files.

Targets:
  src/*.c
  include/*.h
  acvp/src/*.c
  tests/*.c

Usage:
  python3 tools/add_headers.py            # apply changes
  python3 tools/add_headers.py --dry-run  # preview only, no files written

Behaviour:
  - Files with no existing copyright comment: header prepended at line 1
  - Files with an existing copyright comment at the top: script asks before
    replacing
  - All other files: header prepended silently
"""

import argparse
import glob
import os
import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Header template
# ---------------------------------------------------------------------------

HEADER = """\
/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */
"""

# Regex that matches a C block comment starting at the very beginning of the
# file (possibly preceded only by whitespace/newlines) that contains the word
# "Copyright" or "SPDX".
_EXISTING_HEADER_RE = re.compile(
    r"^\s*/\*.*?(Copyright|SPDX).*?\*/",
    re.DOTALL | re.IGNORECASE,
)

# ---------------------------------------------------------------------------
# File targeting
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parents[1]

PATTERNS = [
    "src/*.c",
    "include/*.h",
    "acvp/src/*.c",
    "tests/*.c",
]


def collect_files() -> list[Path]:
    files = []
    for pattern in PATTERNS:
        matches = sorted(REPO_ROOT.glob(pattern))
        files.extend(matches)
    return files


# ---------------------------------------------------------------------------
# Header detection and manipulation
# ---------------------------------------------------------------------------

def has_existing_header(content: str) -> bool:
    """Return True if the file already starts with a copyright/SPDX block."""
    return bool(_EXISTING_HEADER_RE.match(content))


def strip_existing_header(content: str) -> str:
    """Remove an existing copyright/SPDX block comment from the top of content."""
    match = _EXISTING_HEADER_RE.match(content)
    if not match:
        return content
    # Strip the matched block and any immediately following blank lines
    remainder = content[match.end():]
    return remainder.lstrip("\n")


def prepend_header(content: str) -> str:
    """Prepend HEADER to content, separated by a blank line."""
    return HEADER + "\n" + content


# ---------------------------------------------------------------------------
# Interactive prompt
# ---------------------------------------------------------------------------

def ask_replace(path: Path) -> bool:
    """Ask the user whether to replace an existing header. Returns True to replace."""
    while True:
        answer = input(f"  '{path.relative_to(REPO_ROOT)}' already has a copyright header. Replace? [y/n]: ")
        answer = answer.strip().lower()
        if answer in ("y", "yes"):
            return True
        if answer in ("n", "no"):
            return False
        print("  Please enter y or n.")


# ---------------------------------------------------------------------------
# Per-file processing
# ---------------------------------------------------------------------------

def process_file(path: Path, dry_run: bool) -> str:
    """
    Process a single file. Returns a status string for the summary.

    Statuses: "added", "replaced", "skipped", "unchanged"
    """
    content = path.read_text(encoding="utf-8")

    if has_existing_header(content):
        if dry_run:
            print(f"  [DRY RUN] '{path.relative_to(REPO_ROOT)}' — has existing header (would ask to replace)")
            return "would-ask"

        replace = ask_replace(path)
        if not replace:
            print(f"  Skipped  '{path.relative_to(REPO_ROOT)}'")
            return "skipped"

        new_content = prepend_header(strip_existing_header(content))
        path.write_text(new_content, encoding="utf-8")
        print(f"  Replaced '{path.relative_to(REPO_ROOT)}'")
        return "replaced"

    else:
        new_content = prepend_header(content)
        if dry_run:
            print(f"  [DRY RUN] '{path.relative_to(REPO_ROOT)}' — would add header")
            return "would-add"

        path.write_text(new_content, encoding="utf-8")
        print(f"  Added    '{path.relative_to(REPO_ROOT)}'")
        return "added"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Add copyright/SPDX headers to C source files."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview changes without writing any files.",
    )
    args = parser.parse_args()

    dry_run = args.dry_run

    if dry_run:
        print("=== DRY RUN — no files will be modified ===\n")
    else:
        print("=== Adding copyright headers ===\n")

    files = collect_files()

    if not files:
        print("No files found. Check that you are running from the repo root.")
        sys.exit(1)

    counts = {
        "added": 0,
        "replaced": 0,
        "skipped": 0,
        "would-add": 0,
        "would-ask": 0,
    }

    for path in files:
        status = process_file(path, dry_run)
        counts[status] = counts.get(status, 0) + 1

    print()
    if dry_run:
        print(f"Would add header:          {counts['would-add']}")
        print(f"Would ask before replacing:{counts['would-ask']}")
        print(f"\nRe-run without --dry-run to apply changes.")
    else:
        print(f"Headers added:    {counts['added']}")
        print(f"Headers replaced: {counts['replaced']}")
        print(f"Skipped:          {counts['skipped']}")


if __name__ == "__main__":
    main()
