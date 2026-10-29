#!/usr/bin/env python3
"""Carry the C type rename (tools/c_rename.py) into non-C canonical sources.

Only distinctive names are replaced: module-prefixed CamelCase such as `PFPmu`
or `TcCollection`, never single capitalized words such as `Context` or
`Model`, which also occur as English prose. Replacement is confined to places
that hold C text:

- Python: string literals only (C anchors, templates and expected snippets);
  Python identifiers and comments are left alone.
- Markdown: inline code spans and fenced code blocks.
- JSON: string values.

Historical records (reviews/, research/ evidence JSON, references/, earlier
editions) and generated outputs are not edited here; generated outputs are
rebuilt by their own generators.

    python3 tools/rename_text.py --map MAP.json [--check]
"""
from __future__ import annotations

import argparse
import io
import json
from pathlib import Path
import re
import subprocess
import tokenize as pytokenize

ROOT = Path(__file__).resolve().parents[1]

CANONICAL_MARKDOWN = ("curriculum/projects/", "curriculum/modules/", "curriculum/guides/",
                      "curriculum/v5/exercises/", "curriculum/v5/routes/",
                      "curriculum/v5/expansion/legacy_assessments/", "labs/", "assessor/")
CANONICAL_JSON = ("curriculum/v5/expansion/legacy_assessments/", "curriculum/v5/routes/contextual-v1.source.json",
                  "fixtures/", "labs/", "assessor/")
PYTHON = ("tools/", "labs/", "assessor/")  # research/ recipes are historical inputs: never edited
GENERATED = {"curriculum/v5/exercises.json", "curriculum/v5/routes/contextual-v1.json",
             "curriculum/v5/routes/coverage.json", "curriculum/v5/routes/criterion_resolutions.json",
             "curriculum/v5/routes/migration.json"}
SKIP_PYTHON = {"tools/c_rename.py", "tools/rename_text.py", "tools/source_equivalence.py"}


def distinctive(name):
    return re.fullmatch(r"[A-Z][a-z0-9]+", name) is None and re.search(r"[A-Z]", name) is not None


def build_pattern(mapping):
    names = sorted((n for n in mapping if distinctive(n) and len(n) >= 3), key=len, reverse=True)
    return re.compile(r"(?<![A-Za-z0-9_])(" + "|".join(map(re.escape, names)) + r")(?![A-Za-z0-9_])")


def replace(text, pattern, mapping):
    return pattern.sub(lambda m: mapping[m.group(1)], text)


def python_strings(source, pattern, mapping):
    edits = []
    try:
        for token in pytokenize.generate_tokens(io.StringIO(source).readline):
            if token.type == pytokenize.STRING:
                new = replace(token.string, pattern, mapping)
                if new != token.string:
                    edits.append((token.start, token.end, new))
    except (pytokenize.TokenError, IndentationError, SyntaxError):
        return source
    if not edits:
        return source
    lines = source.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    for (sl, sc), (el, ec), new in reversed(edits):
        start, end = offsets[sl - 1] + sc, offsets[el - 1] + ec
        source = source[:start] + new + source[end:]
    return source


C_FENCES = {"c", "h", "c17", "c23", "c11"}


def markdown_code(text, pattern, mapping):
    """Rename inside C code fences and inline code only. Other fences (notably
    the ```json metadata header of canonical briefs, whose source locators
    must match the inspected-source registry) are left untouched."""
    out, fence, is_c = [], None, False
    for line in text.splitlines(keepends=True):
        stripped = line.lstrip()
        if stripped.startswith(("```", "~~~")):
            marker = stripped[:3]
            if fence is None:
                fence, is_c = marker, stripped[3:].strip().lower() in C_FENCES
            elif marker == fence:
                fence, is_c = None, False
            out.append(line)
            continue
        if fence:
            out.append(replace(line, pattern, mapping) if is_c else line)
        else:
            out.append(re.sub(r"`[^`\n]+`", lambda m: replace(m.group(0), pattern, mapping), line))
    return "".join(out)


def json_strings(text, pattern, mapping):
    # Edit string values in place so key order, spacing and line endings stay.
    return re.sub(r'"(?:[^"\\]|\\.)*"', lambda m: replace(m.group(0), pattern, mapping), text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", required=True, help="JSON {old name: replacement text}")
    parser.add_argument("--check", action="store_true", help="report files that would change")
    args = parser.parse_args()
    mapping = json.loads(Path(args.map).read_text())
    pattern = build_pattern(mapping)
    files = subprocess.run(["git", "ls-files"], cwd=ROOT, capture_output=True, text=True,
                           check=True).stdout.split()
    changed = []
    for name in files:
        if name in GENERATED or name.startswith(("reviews/", "build/")):
            continue
        if name.endswith(".py") and name.startswith(PYTHON) and name not in SKIP_PYTHON:
            transform = python_strings
        elif name.endswith(".md") and name.startswith(CANONICAL_MARKDOWN):
            transform = markdown_code
        elif name.endswith(".json") and name.startswith(CANONICAL_JSON):
            transform = json_strings
        else:
            continue
        path = ROOT / name
        raw = path.read_bytes()
        text = raw.decode("utf-8", "surrogateescape")
        new = transform(text, pattern, mapping)
        if new != text:
            changed.append(name)
            if not args.check:
                path.write_bytes(new.encode("utf-8", "surrogateescape"))
    print(json.dumps({"changed": len(changed), "files": changed[:40]}, indent=0))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
