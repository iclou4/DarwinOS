#!/usr/bin/env python3

"""
A simple utility that verifies the syntax for Python scripts.
The checks it does are:
  * Check for 'tab' characters in .py files
  * Compile errors in Python sources

Usage:
  python syntax_checker.py <python_source_file> [<python_source_file> ..]
"""

import py_compile
import sys
import os
import re

tabs_search_rex = re.compile(r"^\s*\t+", re.MULTILINE | re.DOTALL)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: Unknown arguments", file=sys.stderr)
        print(__doc__)
        sys.exit(1)

    for fname in sys.argv[1:]:
        if not os.path.exists(fname):
            print(f"Error: Cannot recognize {fname} as a file", file=sys.stderr)
            sys.exit(1)

        if not fname.endswith(".py"):
            print(f"Note: {fname} is not a valid Python file. Skipping.")
            continue

        with open(fname, "r", encoding="utf-8") as fh:
            lines = fh.readlines()

        tab_check_status = True
        for lineno, line in enumerate(lines, start=1):
            if tabs_search_rex.findall(line):
                print(f"Error: Found a TAB character at {fname}:{lineno}", file=sys.stderr)
                tab_check_status = False

        if not tab_check_status:
            print("Error: Syntax check failed. Please fix the errors and try again.", file=sys.stderr)
            sys.exit(1)

        # Now check for compile-time syntax errors
        try:
            py_compile.compile(fname, doraise=True)
        except py_compile.PyCompileError as exc:
            print(str(exc), file=sys.stderr)
            print("Error: Compilation failed. Please fix the errors and try again.", file=sys.stderr)
            sys.exit(1)

        print(f"Success: Checked {fname}. No syntax errors found.")

    sys.exit(0)

