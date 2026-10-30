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

Two guards, added after the first run (f6f8d8c07) rewrote prose and expected
program output:

- COMMON_WORDS: a C type that is spelled like an ordinary word or acronym
  (`CPU`) is never replaced outside C files; in prose it means the word.
- Printed output: C string literals are not renamed, so C programs still print
  the old names. A name is left alone when it, with up to eight neighbouring
  characters, occurs in a C string literal under labs/ or assessor/ (markers
  such as `OAA_BASE actual-OwAllocator`), or when a string consists of the
  name alone and the C sources print it as a word (ledger keys such as
  `"HttpRequest"`, provider labels such as `PdTransfer`).

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


def frozen_route_files():
    """Paths a later route pins byte-for-byte (predecessor.frozen_sha256)."""
    frozen = set()
    for source in sorted((ROOT / "curriculum/v5/routes").glob("*.source.json")):
        predecessor = json.loads(source.read_text()).get("predecessor") or {}
        frozen.update(predecessor.get("frozen_sha256", {}))
    return frozen


def distinctive(name):
    return re.fullmatch(r"[A-Z][a-z0-9]+", name) is None and re.search(r"[A-Z]", name) is not None


COMMON_WORDS = {"CPU"}


def build_pattern(mapping):
    names = sorted((n for n in mapping if distinctive(n) and len(n) >= 3 and n not in COMMON_WORDS),
                   key=len, reverse=True)
    return re.compile(r"(?<![A-Za-z0-9_])(" + "|".join(map(re.escape, names)) + r")(?![A-Za-z0-9_])")


_PRINTED = None


def printed_literals():
    """C string literals under labs/ and assessor/, joined for substring search."""
    global _PRINTED
    if _PRINTED is None:
        files = subprocess.run(["git", "ls-files", "labs", "assessor"], cwd=ROOT, capture_output=True,
                               text=True, check=True).stdout.split()
        literals = []
        for name in files:
            if name.endswith((".c", ".h", ".inc")):
                text = (ROOT / name).read_text(encoding="utf-8", errors="replace")
                literals += re.findall(r'"((?:[^"\\\n]|\\.)*)"', text)
        _PRINTED = "\0".join(literals)
    return _PRINTED


def quotes_output(text, match):
    """True when the name at `match` is program output: with up to eight
    neighbouring characters on each side (stopping at quotes) it occurs in a C
    string literal, or it is the whole string and C prints it as a word."""
    name = match.group(1)
    before = re.split(r"[\"'\n]", text[max(0, match.start() - 8):match.start()])[-1]
    after = re.split(r"[\"'\n]", text[match.end():match.end() + 8])[0]
    if before or after:
        return before + name + after in printed_literals()
    bare = re.fullmatch(r"[rRbBuU]{0,2}(['\"])" + re.escape(name) + r"\1", text) is not None
    # Printed labels join words with underscores (`actual_PdTransfer`), so only
    # letters and digits delimit the name here.
    return bare and re.search(r"(?<![A-Za-z0-9])" + re.escape(name) + r"(?![A-Za-z0-9])",
                              printed_literals()) is not None


def replace(text, pattern, mapping):
    return pattern.sub(lambda m: m.group(1) if quotes_output(text, m) else mapping[m.group(1)], text)


class Patterns:
    """The compiled name pattern and its replacement map."""

    def __init__(self, mapping):
        self.mapping = mapping
        self.pattern = build_pattern(mapping)

    def rename(self, text):
        return replace(text, self.pattern, self.mapping)


def python_strings(source, patterns):
    edits = []
    try:
        for token in pytokenize.generate_tokens(io.StringIO(source).readline):
            if token.type == pytokenize.STRING:
                new = patterns.rename(token.string)
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


def markdown_code(text, patterns):
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
            out.append(patterns.rename(line) if is_c else line)
        else:
            out.append(re.sub(r"`[^`\n]+`", lambda m: patterns.rename(m.group(0)), line))
    return "".join(out)


def json_strings(text, patterns):
    # Edit string values in place so key order, spacing and line endings stay.
    return re.sub(r'"(?:[^"\\]|\\.)*"', lambda m: patterns.rename(m.group(0)), text)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--map", required=True, help="JSON {old name: replacement text}")
    parser.add_argument("--check", action="store_true", help="report files that would change")
    args = parser.parse_args()
    mapping = json.loads(Path(args.map).read_text())
    patterns = Patterns(mapping)
    files = subprocess.run(["git", "ls-files"], cwd=ROOT, capture_output=True, text=True,
                           check=True).stdout.split()
    changed = []
    frozen = frozen_route_files()
    for name in files:
        if name in GENERATED or name in frozen or name.startswith(("reviews/", "build/")):
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
        new = transform(text, patterns)
        if new != text:
            changed.append(name)
            if not args.check:
                path.write_bytes(new.encode("utf-8", "surrogateescape"))
    print(json.dumps({"changed": len(changed), "files": changed[:40]}, indent=0))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
