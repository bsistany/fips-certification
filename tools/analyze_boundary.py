#!/usr/bin/env python3
"""
analyze_boundary.py — Cryptographic Module Boundary Analyzer

Runs nm on all compiled object files in src/ and classifies every symbol
to produce a formal boundary definition report.

Usage:
    python3 tools/analyze_boundary.py

Must be run from the repo root after `make compile`.

Output sections:
    1. Public API          — T symbols: exported functions (module interfaces)
    2. Internal Symbols    — t/s symbols: private functions and data
    3. Internal Deps       — U symbols resolved within our own object files
    4. External Deps       — U symbols resolved outside (operational environment)

Reference: FIPS 140-3 Section 7.6 (Interfaces)
"""

import subprocess
import sys
import os
from collections import defaultdict

SRC_DIR   = "src"
OBJ_GLOB  = "*.o"
SEPARATOR = "-" * 60


def run_nm(obj_file):
    """Run nm on a single object file, return list of (address, type, name)."""
    try:
        result = subprocess.run(
            ["nm", obj_file],
            capture_output=True, text=True, check=True
        )
    except subprocess.CalledProcessError as e:
        print(f"ERROR: nm failed on {obj_file}: {e}", file=sys.stderr)
        return []

    symbols = []
    for line in result.stdout.splitlines():
        parts = line.strip().split()
        if len(parts) == 3:
            addr, sym_type, name = parts
        elif len(parts) == 2:
            sym_type, name = parts
            addr = None
        else:
            continue
        # Strip leading underscore (macOS prefixes all C symbols with _)
        name = name.lstrip("_")
        symbols.append((addr, sym_type, name, obj_file))
    return symbols


def collect_all_symbols(src_dir):
    """Collect symbols from all .o files in src_dir."""
    obj_files = [
        os.path.join(src_dir, f)
        for f in sorted(os.listdir(src_dir))
        if f.endswith(".o")
    ]
    if not obj_files:
        print(f"ERROR: No .o files found in {src_dir}/. Run 'make compile' first.",
              file=sys.stderr)
        sys.exit(1)

    all_symbols = []
    for obj in obj_files:
        all_symbols.extend(run_nm(obj))
    return all_symbols


def analyze(symbols):
    """Classify symbols into boundary categories."""
    # All names defined within our module (T or t or s or d or b)
    defined_internally = {
        name for (_, sym_type, name, _) in symbols
        if sym_type in ("T", "t", "s", "d", "b", "S", "D", "B")
    }

    public_api      = []  # T — exported, visible outside boundary
    internal_impl   = []  # t/s — private, inside boundary
    internal_deps   = []  # U resolved within our own .o files
    external_deps   = []  # U resolved outside (operational environment)
    seen_undefined  = set()

    for addr, sym_type, name, obj_file in symbols:
        module = os.path.basename(obj_file)

        if sym_type == "T":
            public_api.append((name, module))

        elif sym_type in ("t", "s", "d", "b", "S", "D", "B"):
            internal_impl.append((name, sym_type, module))

        elif sym_type == "U":
            if name in seen_undefined:
                continue
            seen_undefined.add(name)
            if name in defined_internally:
                internal_deps.append(name)
            else:
                external_deps.append(name)

    return public_api, internal_impl, internal_deps, external_deps


def print_report(public_api, internal_impl, internal_deps, external_deps):
    print()
    print("=" * 60)
    print("  FIPS-CRYPTO MODULE BOUNDARY ANALYSIS")
    print("=" * 60)

    # ----------------------------------------------------------------
    print(f"\n{SEPARATOR}")
    print("1. PUBLIC API  (inside boundary — exposed to callers)")
    print(f"   These are the module's formal interfaces per FIPS 140-3 §7.6")
    print(SEPARATOR)
    by_module = defaultdict(list)
    for name, module in sorted(public_api):
        by_module[module].append(name)
    for module in sorted(by_module):
        print(f"\n  [{module}]")
        for name in sorted(by_module[module]):
            print(f"    {name}")

    # ----------------------------------------------------------------
    print(f"\n{SEPARATOR}")
    print("2. INTERNAL SYMBOLS  (inside boundary — not exposed)")
    print(f"   Private functions and static data")
    print(SEPARATOR)
    by_module = defaultdict(list)
    for name, sym_type, module in sorted(internal_impl):
        by_module[module].append((sym_type, name))
    for module in sorted(by_module):
        print(f"\n  [{module}]")
        for sym_type, name in sorted(by_module[module]):
            kind = "fn " if sym_type in ("t",) else "data"
            print(f"    {kind}  {name}")

    # ----------------------------------------------------------------
    print(f"\n{SEPARATOR}")
    print("3. INTERNAL DEPENDENCIES  (inside boundary)")
    print(f"   Calls between our own modules")
    print(SEPARATOR)
    for name in sorted(internal_deps):
        print(f"    {name}")

    # ----------------------------------------------------------------
    print(f"\n{SEPARATOR}")
    print("4. EXTERNAL DEPENDENCIES  (outside boundary — operational environment)")
    print(f"   These must be documented as environmental assumptions")
    print(SEPARATOR)
    for name in sorted(external_deps):
        print(f"    {name}")

    # ----------------------------------------------------------------
    print(f"\n{SEPARATOR}")
    print("SUMMARY")
    print(SEPARATOR)
    print(f"  Public API functions   : {len(public_api)}")
    print(f"  Internal symbols       : {len(internal_impl)}")
    print(f"  Internal dependencies  : {len(internal_deps)}")
    print(f"  External dependencies  : {len(external_deps)}")
    print()


def main():
    if not os.path.isdir(SRC_DIR):
        print(f"ERROR: Must be run from repo root (no '{SRC_DIR}/' found).",
              file=sys.stderr)
        sys.exit(1)

    symbols = collect_all_symbols(SRC_DIR)
    public_api, internal_impl, internal_deps, external_deps = analyze(symbols)
    print_report(public_api, internal_impl, internal_deps, external_deps)


if __name__ == "__main__":
    main()
