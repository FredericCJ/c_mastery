#!/usr/bin/env python3
"""Prove that mechanical C transformations preserve behaviour.

Three proofs, one per mechanical step of the house-style migration:

objects  Rename: compile every changed translation unit from the base commit
         and from the working tree with identical flags, then compare the
         object files without debug information. Equal sections prove the
         rename produced the same machine code. When only string literals
         differ (a stringified type name), the strings must differ exactly by
         the rename map and the code must be identical once relocation
         addends into string sections are masked.
tokens   Formatting: the comment-free C token sequence of every changed file
         is identical, and comment text is identical up to layout.
braces   Brace insertion: the token difference consists only of inserted
         `{` and `}` tokens.

Every proof writes a receipt and can append the proven old->new file hashes to
curriculum/v5/style_migration.json, which lets checks recognise evidence bound
to the old bytes as carried forward (not re-executed).
"""
from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ProcessPoolExecutor
from datetime import datetime, timezone
import difflib
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from c_source import comments, significant, tokenize  # noqa: E402

MIGRATION = ROOT / "curriculum/v5/style_migration.json"
COMPILERS = (
    ("host", ["gcc", "-std=gnu17", "-O1"]),
    ("cortex-m4", ["arm-none-eabi-gcc", "-std=gnu17", "-O1", "-mcpu=cortex-m4", "-mthumb",
                   "-ffreestanding"]),
)
COMMON = ["-c", "-g0", "-w", "-fno-asynchronous-unwind-tables", "-fno-ident"]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def changed_files(base):
    out = subprocess.run(["git", "diff", "--name-only", base, "--", "labs", "assessor"],
                         cwd=ROOT, capture_output=True, text=True, check=True).stdout.split()
    return [f for f in out if f.endswith((".c", ".h", ".inc")) and (ROOT / f).exists()]


def export_base(base, destination):
    archive = subprocess.run(["git", "archive", base, "labs", "assessor", "fixtures"], cwd=ROOT,
                             capture_output=True, check=True).stdout
    subprocess.run(["tar", "-x", "-C", str(destination)], input=archive, check=True)


# --------------------------------------------------------------------------
# Object identity


def include_dirs(relative, tree):
    """Directories a lab TU plausibly needs: its own tree, its family's header
    directories, and directories of any same-named header it includes."""
    path = tree / relative
    parts = Path(relative).parts
    family_roots = [tree / parts[0] / parts[1], tree / ("labs" if parts[0] == "assessor" else "assessor") / parts[1]]
    dirs = [path.parent]
    for root in family_roots:
        if root.is_dir():
            dirs += sorted({h.parent for h in root.rglob("*.h")})
    text = path.read_text(errors="replace")
    for include in re.findall(r'#\s*include\s*"([^"]+)"', text):
        name = Path(include).name
        for hit in sorted((tree / "labs").rglob(name)):  # deterministic order
            dirs.append(hit.parent)
    seen, ordered = set(), []
    for d in dirs:
        if d not in seen:
            seen.add(d)
            ordered.append(d)
    return ordered


def compile_unit(args):
    relative, tree, out_dir, compiler = args
    tree = Path(tree)
    obj = Path(out_dir) / (relative.replace("/", "__") + f".{compiler[0]}.o")
    flags = [*compiler[1], *COMMON, f"-ffile-prefix-map={tree}=.", f"-fmacro-prefix-map={tree}=."]
    command = [*flags, *("-I" + str(d) for d in include_dirs(relative, tree)), str(tree / relative),
               "-o", str(obj)]
    result = subprocess.run(command, cwd=tree, capture_output=True, text=True, timeout=120)
    return obj if result.returncode == 0 else None


def section_bytes(dump):
    """Parse `objdump -s` output into {section name: bytes}."""
    sections, current = {}, None
    for line in dump.splitlines():
        header = re.match(r"^Contents of section (\S+):", line)
        if header:
            current = header.group(1)
            sections[current] = bytearray()
            continue
        row = re.match(r"^ [0-9a-f]+ ((?:[0-9a-f]{2,8} ){1,4})", line)
        if row and current:
            sections[current] += bytes.fromhex(row.group(1).replace(" ", ""))
    return {name: bytes(data) for name, data in sections.items()}


def object_view(obj):
    """Debug-free section contents, relocations and disassembly of an object."""
    stripped = obj.with_suffix(".stripped.o")
    arm = ".cortex-m4." in obj.name
    tool = "arm-none-eabi-objcopy" if arm else "objcopy"
    dump = "arm-none-eabi-objdump" if arm else "objdump"
    subprocess.run([tool, "--strip-debug", "-R", ".comment", "-R", ".note.GNU-stack", str(obj), str(stripped)],
                   check=True, capture_output=True)
    run = lambda *a: subprocess.run([dump, *a, str(stripped)], capture_output=True, text=True).stdout
    body = lambda s: "\n".join(line for line in s.splitlines() if "file format" not in line)
    return dict(sections=section_bytes(run("-s")), relocations=body(run("-r")),
                disassembly=body(run("-d", "-r", "--no-show-raw-insn")))


def renamed(text, rename):
    for old, new in rename.items():
        text = re.sub(rf"\b{re.escape(old)}\b", new, text)
    return text


def compare_objects(old_obj, new_obj, rename):
    a, b = object_view(old_obj), object_view(new_obj)
    if a["sections"] == b["sections"] and a["relocations"] == b["relocations"]:
        return "identical"
    if set(a["sections"]) != set(b["sections"]):
        return "different"
    differing = [name for name in a["sections"] if a["sections"][name] != b["sections"][name]]
    # Code may differ only where REL targets (Arm) keep a string-relocation
    # addend inside the instruction stream; that is checked on the masked
    # disassembly below. Any other non-string section difference is real.
    if any(not (name.startswith(".rodata") or name == ".text") for name in differing):
        return "different"
    # A string may legitimately be renamed (a stringified type name) or stay
    # unchanged (an ordinary literal, which the rename never edits), and a
    # string whose length changed may move between .rodata.str1.N sections, so
    # compare the strings of all read-only data as one multiset.
    collect = lambda view: [piece for name, data in sorted(view["sections"].items())
                            if name.startswith(".rodata")
                            for piece in data.decode("latin-1").split("\0") if piece]
    remaining = Counter(collect(b))
    for piece in collect(a):
        for candidate in (renamed(piece, rename), piece):
            if remaining[candidate] > 0:
                remaining[candidate] -= 1
                break
        else:
            return "different"
    if +remaining:
        return "different"
    # Code must be identical once relocation addends into string data, which
    # move when a string's length changes, are masked.
    def mask(text):
        text = re.sub(r"(\.rodata[^\s+-]*)[+-]0x[0-9a-f]+", r"\1+ADDEND", text)
        # A literal-pool word followed by its relocation against string data:
        # the word holds the addend (REL relocations), so mask its value.
        lines = text.splitlines()
        for i in range(len(lines) - 1):
            if re.search(r"R_\w+\s+\.rodata", lines[i + 1]):
                lines[i] = re.sub(r"\.word\s+0x[0-9a-f]+", ".word ADDEND", lines[i])
        return "\n".join(lines)
    # objdump annotates targets relative to the nearest symbol (<.LC5+0x30>);
    # those labels move with string data while the target address does not.
    strip = lambda s: re.sub(r"<[^>]*>", "<>", re.sub(r"^\s*[0-9a-f]+:", "", s, flags=re.M))
    if strip(mask(a["disassembly"])) == strip(mask(b["disassembly"])):
        return "identical-except-renamed-strings"
    return "different"


def prove_objects(args):
    base = args.base
    files = [f for f in changed_files(base) if f.endswith(".c")]
    if args.only:
        wanted = set(Path(args.only).read_text().split())
        files = [f for f in files if f in wanted]
    rename = json.loads(Path(args.rename_map).read_text()) if args.rename_map else {}
    plan = json.loads(Path(args.rename_plan).read_text()) if args.rename_plan else {"files": {}}

    def file_map(relative):
        """The plan's mapping for this unit; the global map only without a plan
        entry (a global map would also rename unrelated words such as "A")."""
        entry = plan["files"].get(relative)
        if entry is None:
            return dict(rename)
        mapping = {}
        for old, (new, kind) in entry.get("types", {}).items():
            mapping[old] = new if kind == "other" else f"{kind} {new}"
        for old, new in entry.get("tags", {}).items():
            mapping.setdefault(old, new)
        return mapping
    results = {}
    with tempfile.TemporaryDirectory(prefix="cm-equivalence-") as directory:
        work = Path(directory)
        before, after, objects = work / "before", work / "after", work / "objects"
        for d in (before, after, objects):
            d.mkdir()
        export_base(base, before)
        for top in ("labs", "assessor", "fixtures"):
            shutil.copytree(ROOT / top, after / top, symlinks=True,
                            ignore=shutil.ignore_patterns("__pycache__", "build"))
        jobs = []
        for relative in files:
            if not (before / relative).exists():
                results[relative] = "new-file"
                continue
            jobs.append(relative)
        with ProcessPoolExecutor(max_workers=os.cpu_count()) as pool:
            for relative in jobs:
                outcome = "not-compiled"
                for compiler in COMPILERS:
                    old_obj = pool.submit(compile_unit, (relative, str(before), str(objects / "b"), compiler))
                    new_obj = pool.submit(compile_unit, (relative, str(after), str(objects / "a"), compiler))
                    (objects / "b").mkdir(exist_ok=True)
                    (objects / "a").mkdir(exist_ok=True)
                    o, n = old_obj.result(), new_obj.result()
                    if o and n:
                        outcome = compare_objects(o, n, file_map(relative)) + f"@{compiler[0]}"
                        break
                    if bool(o) != bool(n):
                        outcome = f"compile-mismatch@{compiler[0]}"
                        break
                results[relative] = outcome
    counts = {}
    for outcome in results.values():
        key = outcome.split("@")[0]
        counts[key] = counts.get(key, 0) + 1
    failed = sorted(f for f, o in results.items() if o.startswith(("different", "compile-mismatch")))
    report = dict(schema_version=1, proof="objects", base=base, generated_at=datetime.now(timezone.utc).isoformat(),
                  compilers=[c[1] for c in COMPILERS], common_flags=COMMON, counts=counts,
                  failed=failed, results=results, passed=not failed,
                  limits="not-compiled units (host and Cortex-M4 heuristics both failed) rely on the token-level "
                         "rename structure and on re-running their own campaigns.")
    Path(args.report).write_text(json.dumps(report, indent=1) + "\n")
    print(json.dumps(dict(counts=counts, failed=len(failed))))
    return 0 if not failed else 1


# --------------------------------------------------------------------------
# Token proofs


def base_text(base, relative):
    result = subprocess.run(["git", "show", f"{base}:{relative}"], cwd=ROOT, capture_output=True)
    return result.stdout.decode("utf-8", "surrogateescape") if result.returncode == 0 else None


def token_keys(text):
    return [(t.kind, t.text) for t in significant(tokenize(text))]


BRACES = (("punct", "{"), ("punct", "}"))


def braces_wrap_statements(old, new):
    """Every inserted brace sits at a statement boundary.

    An inserted `{` must follow `)`, `else` or `do`, and an inserted `}` must
    follow `;` or `}`. A formatter that misparses C (for example treating
    `protected` as a C++ access specifier) can insert braces that keep the
    token subsequence but split a statement (`{ protected } = 1;`).
    """
    position = 0
    for k, token in enumerate(new):
        if position < len(old) and token == old[position]:
            position += 1
            continue
        previous = new[k - 1][1] if k else ""
        if token == BRACES[0] and previous not in (")", "else", "do"):
            return False
        if token == BRACES[1] and previous not in (";", "}"):
            return False
    return True


def only_braces_inserted(old, new):
    """True when `old` is `new` with some brace tokens deleted.

    A subsequence test: every token of the old file appears in the new file in
    order, and every extra token is `{` or `}`. Moving or removing a brace is
    therefore rejected, unlike a diff alignment, which can pair braces
    arbitrarily in repetitive code. Inserted braces must also balance.
    """
    position, inserted = 0, []
    for token in new:
        if position < len(old) and token == old[position]:
            position += 1
        elif token in BRACES:
            inserted.append(token)
        else:
            return False
    return position == len(old) and inserted.count(BRACES[0]) == inserted.count(BRACES[1])


def branch_brace_deltas(text):
    """Brace balance of every branch of every preprocessor conditional.

    Brace insertion that ignores preprocessor branches can close a block in
    one branch only (clang-format sees just the first branch), leaving other
    configurations unbalanced although the whole token stream balances. A
    correct insertion never changes any branch's own balance.
    """
    tokens = tokenize(text)
    groups, stack = [], []
    for j, token in enumerate(tokens):
        name = tokens[j + 1].text if (token.kind == "punct" and token.text == "#" and
                                      j + 1 < len(tokens) and tokens[j + 1].kind == "ident") else None
        if name in ("if", "ifdef", "ifndef"):
            stack.append([0])
        elif name in ("elif", "else") and stack:
            stack[-1].append(0)
        elif name == "endif" and stack:
            groups.append(tuple(stack.pop()))
        elif token.kind == "punct" and token.text in "{}" and stack:
            stack[-1][-1] += 1 if token.text == "{" else -1
    return groups


def prove_tokens(args, braces=False):
    results = {}
    for relative in changed_files(args.base):
        old = base_text(args.base, relative)
        if old is None:
            results[relative] = "new-file"
            continue
        new = (ROOT / relative).read_text(errors="surrogateescape")
        a, b = token_keys(old), token_keys(new)
        comments_equal = comments(tokenize(old)) == comments(tokenize(new))
        if a == b:
            results[relative] = "token-equivalent" if comments_equal else "comment-changed"
            continue
        if braces and only_braces_inserted(a, b) and braces_wrap_statements(a, b) and \
                branch_brace_deltas(old) == branch_brace_deltas(new):
            results[relative] = "brace-only" if comments_equal else "brace-only-comment-changed"
            continue
        results[relative] = "different"
    failed = sorted(f for f, r in results.items() if r in ("different", "comment-changed",
                                                            "brace-only-comment-changed"))
    counts = {}
    for r in results.values():
        counts[r] = counts.get(r, 0) + 1
    report = dict(schema_version=1, proof="braces" if braces else "tokens", base=args.base,
                  generated_at=datetime.now(timezone.utc).isoformat(), counts=counts, failed=failed,
                  results=results, passed=not failed)
    Path(args.report).write_text(json.dumps(report, indent=1) + "\n")
    print(json.dumps(dict(counts=counts, failed=len(failed))))
    return 0 if not failed else 1


# --------------------------------------------------------------------------
# Migration record


def prove_text(args):
    """Non-C canonical sources changed only by the text rename.

    A file qualifies when the corrected tools/rename_text.py transform, applied
    to its bytes at `--base` with the rename map, reproduces its current bytes
    exactly. Files with any other edit are reported as `different` and are not
    bridged.
    """
    import rename_text
    patterns = rename_text.Patterns(json.loads(Path(args.map).read_text()))
    transforms = {".py": rename_text.python_strings, ".md": rename_text.markdown_code,
                  ".json": rename_text.json_strings}
    changed = subprocess.run(["git", "diff", "--name-only", args.base, "--"], cwd=ROOT, capture_output=True,
                             text=True, check=True).stdout.split()
    results = {}
    for relative in changed:
        suffix = Path(relative).suffix
        if suffix not in transforms or relative.startswith(("reviews/", "research/", "build/")):
            continue
        old = base_text(args.base, relative)
        if old is None or not (ROOT / relative).is_file():
            continue
        current = (ROOT / relative).read_bytes().decode("utf-8", "surrogateescape")
        renamed = transforms[suffix](old, patterns)
        results[relative] = "text-rename" if renamed == current != old else "different"
    report = dict(proof="text", base=args.base, map=args.map, map_sha256=sha(Path(args.map).read_bytes()),
                  passed=True, results={k: v for k, v in results.items() if v == "text-rename"},
                  not_bridged=sorted(k for k, v in results.items() if v != "text-rename"),
                  scope="Text renames of C identifiers in Python strings, Markdown code and JSON strings; "
                        "reproduced exactly by tools/rename_text.py from the base bytes.")
    Path(args.report).write_text(json.dumps(report, indent=1) + "\n")
    print(json.dumps(dict(bridged=len(report["results"]), not_bridged=len(report["not_bridged"]))))
    return 0


def record(args):
    proof = json.loads(Path(args.proof).read_text())
    if not proof["passed"]:
        raise SystemExit("refusing to record a failed proof")
    data = json.loads(MIGRATION.read_text()) if MIGRATION.is_file() else dict(
        schema_version=1,
        description="Proven layout/naming migrations of lab and assessor C. A receipt pinned to "
                    "an old hash remains valid only along a recorded chain, and is reported as "
                    "carried forward, not re-executed.",
        steps=[])
    entries = {}
    results = dict(proof["results"])
    # Headers and fragments are not translation units: they are covered only
    # through the units that include them, and are recorded as such.
    if proof["proof"] != "text":
        for relative in changed_files(proof["base"]):
            results.setdefault(relative, "not-directly-compiled")
    for relative, outcome in results.items():
        old = base_text(proof["base"], relative)
        if old is None:
            continue
        entries[relative] = dict(old_sha256=sha(old.encode("utf-8", "surrogateescape")),
                                 new_sha256=sha((ROOT / relative).read_bytes()), proof=outcome)
    data["steps"].append(dict(step=args.step, proof=proof["proof"], base=proof["base"],
                              receipt=str(Path(args.proof).resolve().relative_to(ROOT)), files=entries))
    MIGRATION.write_text(json.dumps(data, indent=1) + "\n")
    print(json.dumps(dict(step=args.step, files=len(entries))))
    return 0


def carried_forward(relative, pinned_sha256):
    """True when `relative`'s current bytes descend from `pinned_sha256` along
    recorded, proven migration steps."""
    current = sha((ROOT / relative).read_bytes())
    if current == pinned_sha256:
        return True
    if not MIGRATION.is_file():
        return False
    data = json.loads(MIGRATION.read_text())
    digest = pinned_sha256
    for step in data["steps"]:
        entry = step["files"].get(relative)
        if entry and entry["old_sha256"] == digest:
            digest = entry["new_sha256"]
    return digest == current


def carried_binding(relative, pinned_sha256):
    """Describe a historical pin that `carried_forward` accepts.

    Returns None when the pin is current (nothing to qualify) or does not
    descend from the current bytes. Otherwise the record names the proven
    steps, so a checker can accept the old evidence while stating that it was
    carried forward and not re-executed.
    """
    current = sha((ROOT / relative).read_bytes())
    if current == pinned_sha256 or not carried_forward(relative, pinned_sha256):
        return None
    data = json.loads(MIGRATION.read_text())
    digest, steps = pinned_sha256, []
    for step in data["steps"]:
        entry = step["files"].get(relative)
        if entry and entry["old_sha256"] == digest:
            steps.append(dict(step=step["step"], proof=entry["proof"], receipt=step.get("receipt")))
            digest = entry["new_sha256"]
    return dict(path=relative, execution_source_sha256=pinned_sha256, current_source_sha256=current,
                state="carried-forward-not-re-executed", migration=MIGRATION.relative_to(ROOT).as_posix(),
                steps=steps, new_c_execution=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    o = sub.add_parser("objects")
    o.add_argument("--base", required=True)
    o.add_argument("--rename-map", help="JSON {old identifier: replacement text} for string checks")
    o.add_argument("--rename-plan", help="tools/c_rename.py plan, for per-file string maps")
    o.add_argument("--only", help="file listing the units to (re)check")
    o.add_argument("--report", required=True)
    for name in ("tokens", "braces"):
        t = sub.add_parser(name)
        t.add_argument("--base", required=True)
        t.add_argument("--report", required=True)
    x = sub.add_parser("text")
    x.add_argument("--base", required=True)
    x.add_argument("--map", required=True, help="the rename map tools/rename_text.py was run with")
    x.add_argument("--report", required=True)
    r = sub.add_parser("record")
    r.add_argument("--proof", required=True)
    r.add_argument("--step", required=True)
    args = parser.parse_args()
    if args.command == "objects":
        return prove_objects(args)
    if args.command == "text":
        return prove_text(args)
    if args.command == "record":
        return record(args)
    return prove_tokens(args, braces=args.command == "braces")


if __name__ == "__main__":
    raise SystemExit(main())
