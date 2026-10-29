#!/usr/bin/env python3
"""Check the house C style (curriculum/guides/c-style.md) over its manifest scope.

Layout (Tier L) is checked with the pinned clang-format in dry-run mode, so no
file is modified. Naming rules that need no compilation are checked on the
token stream:

- no typedef of a struct, union or enum (use the tag: `struct pf_pmu`);
- typedef names are lowercase, and function-pointer typedefs end in `_fn`;
- a `clang-format off` region carries a `CM-STYLE:` justification;
- every `CM-SPECIMEN:` file is declared in the manifest and vice versa.

A family is enforced once its manifest state is `formatted` or later; pending
families are reported but do not fail the check. Use --all to enforce every
family regardless of state.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from datetime import datetime, timezone
import fnmatch
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from c_source import significant, tokenize  # noqa: E402

MANIFEST = ROOT / "tools/c_style_manifest.json"
ENFORCED_STATES = {"formatted", "renamed", "rewritten", "reviewed"}
DIAGNOSTIC = re.compile(r"^(?P<path>[^:]+):\d+:\d+: (?:error|warning): code should be clang-formatted")


def resolve_clang_format(manifest):
    """Author tools first, then the interpreter's environment (the publishing
    venv or CI, which install the same pin from requirements-lock.txt)."""
    for candidate in (ROOT / manifest["clang_format"]["binary"],
                      Path(sys.executable).parent / "clang-format"):
        if candidate.is_file():
            return candidate
    found = shutil.which("clang-format")
    if not found:
        raise SystemExit("pinned clang-format not found; pip install -r requirements-lock.txt")
    return Path(found)


def family_of(relative):
    parts = relative.split("/")
    return parts[1] if len(parts) > 2 else parts[0]


def in_scope(manifest):
    listed = subprocess.run(["git", "ls-files", "*.c", "*.h"], cwd=ROOT, capture_output=True,
                            text=True, check=True).stdout.split()
    files = []
    for name in listed:
        if not any(fnmatch.fnmatch(name, p) for p in manifest["include"]):
            continue
        if any(fnmatch.fnmatch(name, e["pattern"]) for e in manifest["exclude"]):
            continue
        files.append(name)
    return files


def layout_failures(binary, style, files):
    failing = set()
    for start in range(0, len(files), 150):
        batch = files[start:start + 150]
        result = subprocess.run([str(binary), "--dry-run", "--Werror", f"--style=file:{style}", *batch],
                                cwd=ROOT, capture_output=True, text=True)
        for line in result.stderr.splitlines():
            match = DIAGNOSTIC.match(line)
            if match:
                failing.add(Path(match["path"]).as_posix())
        if result.returncode not in (0, 1):
            raise RuntimeError(f"clang-format failed: {result.stderr[-500:]}")
    return failing


def typedef_findings(text):
    """Yield (rule, detail) for typedef declarations that break the naming rules."""
    tokens = significant(tokenize(text))
    for i, token in enumerate(tokens):
        if token.kind != "ident" or token.text != "typedef":
            continue
        depth, j = 0, i + 1
        while j < len(tokens):
            t = tokens[j].text
            if t == "{":
                depth += 1
            elif t == "}":
                depth -= 1
            elif t == ";" and depth == 0:
                break
            j += 1
        declaration = tokens[i + 1:j]
        if not declaration:
            continue
        head = declaration[0].text
        names = [t.text for k, t in enumerate(declaration) if t.kind == "ident" and
                 (k + 1 == len(declaration) or declaration[k + 1].text in (",", ")", "["))]
        if head in ("struct", "union", "enum") and any(t.text == "{" for t in declaration):
            yield "aggregate-typedef", " ".join(t.text for t in declaration[:2])
            continue
        if head in ("struct", "union", "enum") and len(declaration) >= 3:
            yield "aggregate-typedef", " ".join(t.text for t in declaration[:3])
            continue
        is_function_pointer = any(declaration[k].text == "(" and declaration[k + 1].text == "*"
                                  for k in range(len(declaration) - 1))
        for name in names:
            if name != name.lower():
                yield "typedef-case", name
            elif is_function_pointer and not name.endswith("_fn"):
                yield "function-pointer-suffix", name


def format_off_findings(text):
    for match in re.finditer(r"clang-format off", text):
        line_start = text.rfind("\n", 0, match.start()) + 1
        line_end = text.find("\n", match.end())
        if "CM-STYLE:" not in text[line_start:line_end if line_end >= 0 else len(text)]:
            yield "unjustified-format-off", text[line_start:line_end].strip()[:80]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, help="write a JSON receipt to this new path")
    parser.add_argument("--all", action="store_true", help="enforce every family regardless of manifest state")
    parser.add_argument("--family", action="append", default=[], help="limit to these families")
    args = parser.parse_args()
    if args.report and args.report.exists():
        parser.error("report exists; use a fresh path")
    manifest = json.loads(MANIFEST.read_text())
    binary = resolve_clang_format(manifest)
    version = subprocess.run([str(binary), "--version"], capture_output=True, text=True).stdout.strip()
    if manifest["clang_format"]["version"] not in version:
        parser.error(f"pinned clang-format {manifest['clang_format']['version']} required, found {version!r}")
    files = in_scope(manifest)
    if args.family:
        files = [f for f in files if family_of(f) in args.family]
    kernel = {entry["path"] for entry in manifest["kernel_style"]}
    house_files = [f for f in files if f not in kernel]
    kernel_files = [f for f in files if f in kernel]
    failing = layout_failures(binary, ROOT / manifest["clang_format"]["house_style"], house_files)
    kernel_style = ROOT / manifest["clang_format"]["kernel_style"]
    if kernel_files and kernel_style.is_file():
        failing |= layout_failures(binary, kernel_style, kernel_files)
    exempt = [e["pattern"] for e in manifest["naming_exemptions"]]
    lessons = set(manifest.get("typedef_lessons", []))
    specimens = {entry["path"] for entry in manifest.get("specimens", [])}
    findings = defaultdict(list)
    tagged = set()
    for name in files:
        text = (ROOT / name).read_text(errors="replace")
        if "CM-SPECIMEN:" in text:
            tagged.add(name)
        for rule, detail in format_off_findings(text):
            findings[name].append({"rule": rule, "detail": detail})
        if name in kernel or name in lessons or any(fnmatch.fnmatch(name, p) for p in exempt):
            continue
        for rule, detail in typedef_findings(text):
            findings[name].append({"rule": rule, "detail": detail})
    for name in sorted(tagged ^ (specimens & set(files))):
        findings[name].append({"rule": "specimen-manifest-mismatch",
                               "detail": "CM-SPECIMEN tag and manifest entry must agree"})
    states = manifest.get("families", {})
    families = defaultdict(lambda: {"files": 0, "layout_failures": 0, "naming_findings": Counter()})
    for name in files:
        row = families[family_of(name)]
        row["files"] += 1
        row["layout_failures"] += name in failing
        for finding in findings.get(name, []):
            row["naming_findings"][finding["rule"]] += 1
    enforced_failures = []
    for name in files:
        family = family_of(name)
        enforced = args.all or states.get(family, {}).get("state") in ENFORCED_STATES
        if enforced and (name in failing or findings.get(name)):
            enforced_failures.append(name)
    summary = {
        "schema_version": 1,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "clang_format": version,
        "manifest_sha256": hashlib.sha256(MANIFEST.read_bytes()).hexdigest(),
        "config_sha256": hashlib.sha256((ROOT / ".clang-format").read_bytes()).hexdigest(),
        "files": len(files),
        "layout_failures": len(failing),
        "naming_findings": sum(len(v) for v in findings.values()),
        "enforced_failures": sorted(enforced_failures),
        "families": {k: {"files": v["files"], "layout_failures": v["layout_failures"],
                         "naming_findings": dict(v["naming_findings"]),
                         "state": states.get(k, {}).get("state", "pending")}
                     for k, v in sorted(families.items())},
        "passed": not enforced_failures,
        "scope": "House style layout and token-level naming rules only; not a semantic, safety or compliance verdict.",
    }
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({k: summary[k] for k in ("files", "layout_failures", "naming_findings", "passed")}
                     | {"enforced_failures": len(enforced_failures)}))
    return 0 if summary["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
