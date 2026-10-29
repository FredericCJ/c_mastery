#!/usr/bin/env python3
"""PostToolUse check: report house-style layout violations in edited C files.

Reads the Claude Code hook payload from stdin. It never rewrites a file: on a
violation it prints the pinned clang-format diagnostics to stderr and exits 2,
which shows them to the agent as feedback. Files outside the style manifest's
scope (vendored shims, specimens with layout exemptions) are skipped.
"""
import fnmatch
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
CLANG_FORMAT = ROOT / "build/author-tools/python/bin/clang-format"
MANIFEST = ROOT / "tools/c_style_manifest.json"


def in_scope(relative):
    if not MANIFEST.is_file():
        return relative.startswith(("labs/", "assessor/"))
    manifest = json.loads(MANIFEST.read_text())
    included = any(fnmatch.fnmatch(relative, pattern) for pattern in manifest.get("include", []))
    excluded = any(fnmatch.fnmatch(relative, entry["pattern"]) for entry in manifest.get("exclude", []))
    return included and not excluded


def main():
    try:
        payload = json.load(sys.stdin)
    except ValueError:
        return 0
    path = payload.get("tool_input", {}).get("file_path", "")
    if not path.endswith((".c", ".h")) or not (ROOT / ".clang-format").is_file():
        return 0
    try:
        relative = Path(path).resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return 0
    if not in_scope(relative) or not CLANG_FORMAT.is_file():
        return 0
    result = subprocess.run([str(CLANG_FORMAT), "--dry-run", "--Werror", "--style=file", path],
                            cwd=ROOT, capture_output=True, text=True, timeout=60)
    if result.returncode == 0:
        return 0
    lines = (result.stderr or result.stdout).splitlines()
    print(f"House C style (clang-format 21.1.8) violations in {relative}:", file=sys.stderr)
    print("\n".join(lines[:40]), file=sys.stderr)
    print("Fix the layout (or run the pinned clang-format on this file) before continuing.",
          file=sys.stderr)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
