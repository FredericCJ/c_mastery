#!/usr/bin/env python3
"""Rename C types to the house naming rules (curriculum/guides/c-style.md).

Structures, unions and enumerations become typedef-free lowercase module-
prefixed tags (`typedef struct {...} PFPmu;` becomes `struct pf_pmu {...};` and
every use of `PFPmu` becomes `struct pf_pmu`). Remaining typedefs (function
pointers, scalars, arrays, vector types) keep their typedef under a lowercase
prefixed name; function-pointer typedefs end in `_fn`.

The transformation works on the C token stream and edits only the characters
of the affected tokens, so line numbers do not change. That makes the object
code of every translation unit comparable before and after the rename (see
tools/source_equivalence.py).

    python3 tools/c_rename.py plan --output PLAN.json        # analyse only
    python3 tools/c_rename.py apply --plan PLAN.json [--family F ...]

`plan` refuses to produce a mapping that is ambiguous or that would capture an
existing identifier; such names need an entry in tools/c_rename_overrides.json.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import fnmatch
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from c_source import significant, tokenize  # noqa: E402

MANIFEST = ROOT / "tools/c_style_manifest.json"
OVERRIDES = ROOT / "tools/c_rename_overrides.json"
AGGREGATES = ("struct", "union", "enum")
TYPE_KEYWORDS = {
    "void", "char", "short", "int", "long", "float", "double", "signed", "unsigned",
    "_Bool", "bool", "_Complex", "const", "volatile", "restrict", "static", "extern",
    "register", "inline", "_Atomic", "_Alignas", "_Thread_local", "auto",
}
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)


def snake(name: str) -> str:
    """CamelCase/ACRONYMCase to snake_case: PFPmuOS -> pf_pmu_os."""
    s = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", name)
    s = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", s)
    return re.sub(r"_+", "_", s).lower().strip("_")


def family_of(relative: str) -> str:
    parts = relative.split("/")
    return parts[1] if len(parts) > 2 else parts[0]


def scope_files(manifest):
    listed = subprocess.run(["git", "ls-files", "*.c", "*.h", "*.inc"], cwd=ROOT,
                            capture_output=True, text=True, check=True).stdout.split()
    kernel = {e["path"] for e in manifest["kernel_style"]}
    exempt = [e["pattern"] for e in manifest["naming_exemptions"]]
    files = []
    for name in listed:
        if not any(fnmatch.fnmatch(name, p) for p in manifest["include"]):
            continue
        if any(fnmatch.fnmatch(name, e["pattern"]) for e in manifest["exclude"]):
            continue
        if name in kernel or any(fnmatch.fnmatch(name, p) for p in exempt):
            continue
        files.append(name)
    return files


# --------------------------------------------------------------------------
# Typedef discovery


def statement_end(tokens, start):
    """Index of the ';' ending the declaration that starts at `start`."""
    braces = parens = 0
    for j in range(start, len(tokens)):
        t = tokens[j].text
        if t == "{":
            braces += 1
        elif t == "}":
            braces -= 1
        elif t == "(":
            parens += 1
        elif t == ")":
            parens -= 1
        elif t == ";" and braces == 0 and parens == 0:
            return j
    return None


def split_declarators(tokens, start, end):
    """Split tokens[start:end] on top-level commas into index ranges."""
    ranges, depth, begin = [], 0, start
    for j in range(start, end):
        t = tokens[j].text
        if t in "([{":
            depth += 1
        elif t in ")]}":
            depth -= 1
        elif t == "," and depth == 0:
            ranges.append((begin, j))
            begin = j + 1
    ranges.append((begin, end))
    return ranges


def declared_name(tokens, start, end):
    """Name declared by tokens[start:end] (a declarator or a whole typedef)."""
    for j in range(start, end - 2):
        if tokens[j].text == "(" and tokens[j + 1].text == "*" and tokens[j + 2].kind == "ident":
            return j + 2, True
    depth, candidate = 0, None
    j = start
    while j < end:
        t = tokens[j]
        if t.kind == "ident" and t.text in ("__attribute__", "__attribute"):
            # skip the attribute's double parentheses
            k, level = j + 1, 0
            while k < end:
                if tokens[k].text == "(":
                    level += 1
                elif tokens[k].text == ")":
                    level -= 1
                    if level == 0:
                        break
                k += 1
            j = k + 1
            continue
        if t.text in "([":
            depth += 1
        elif t.text in ")]":
            depth -= 1
        elif depth == 0 and t.kind == "ident" and t.text not in TYPE_KEYWORDS:
            candidate = j
        j += 1
    return candidate, False


def typedefs_in(text):
    """Yield dicts describing every typedef declaration in `text`."""
    tokens = significant(tokenize(text))
    for i, token in enumerate(tokens):
        if token.kind != "ident" or token.text != "typedef":
            continue
        end = statement_end(tokens, i)
        if end is None:
            continue
        head = tokens[i + 1].text if i + 1 < end else ""
        if head in AGGREGATES:
            tag = tokens[i + 2].text if tokens[i + 2].kind == "ident" else None
            brace = i + 3 if tag else i + 2
            if brace < end and tokens[brace].text == "{":
                depth, close = 0, None
                for j in range(brace, end):
                    if tokens[j].text == "{":
                        depth += 1
                    elif tokens[j].text == "}":
                        depth -= 1
                        if depth == 0:
                            close = j
                            break
                declarators = split_declarators(tokens, close + 1, end)
                yield dict(kind=head, form="definition", tag=tag, typedef=i, close=close, end=end,
                           declarators=declarators, tokens=tokens)
                continue
            if tag:
                declarators = split_declarators(tokens, i + 3, end)
                yield dict(kind=head, form="forward", tag=tag, typedef=i, close=None, end=end,
                           declarators=declarators, tokens=tokens)
                continue
        index, function_pointer = declared_name(tokens, i + 1, end)
        if index is not None:
            yield dict(kind="other", form="function-pointer" if function_pointer else "other",
                       tag=None, typedef=i, name_index=index, end=end, tokens=tokens)


# --------------------------------------------------------------------------
# Planning


def module_prefixes(files, texts):
    """Most common function-name prefix per family, from its headers."""
    counts = defaultdict(Counter)
    for name in files:
        if not name.endswith(".h"):
            continue
        for match in re.finditer(r"\b([a-z][a-z0-9]{0,7})_[a-z0-9_]+\s*\(", texts[name]):
            counts[family_of(name)][match.group(1)] += 1
    prefixes = {}
    for family in {family_of(n) for n in files}:
        if counts[family]:
            prefixes[family] = counts[family].most_common(1)[0][0]
        else:
            prefixes[family] = "".join(word[0] for word in family.split("_"))
    return prefixes


def new_name(old, kind, form, prefix, overrides):
    if old in overrides.get("names", {}):
        return overrides["names"][old]
    base = snake(old)
    if "_" not in base and len(old) >= 4 and old.isupper():
        base = f"{base[:2]}_{base[2:]}"   # FHIO -> fh_io
    if "_" not in base:
        base = f"{prefix}_{base}"
    if form == "function-pointer":
        base = re.sub(r"_(fn|func|function)$", "", base) + "_fn"
    return base


def resolve_includes(files):
    """Map path suffixes to in-scope files (headers and included .c files)."""
    by_suffix = defaultdict(list)
    for name in files:
        parts = PurePosixPath(name).parts
        for k in range(1, len(parts) + 1):
            by_suffix["/".join(parts[-k:])].append(name)
    return by_suffix


def plan(args):
    manifest = json.loads(MANIFEST.read_text())
    overrides = json.loads(OVERRIDES.read_text()) if OVERRIDES.is_file() else {}
    files = scope_files(manifest)
    texts = {n: (ROOT / n).read_text(errors="replace") for n in files}
    prefixes = module_prefixes(files, texts)
    prefixes.update(overrides.get("family_prefixes", {}))
    by_suffix = resolve_includes(files)

    definitions = defaultdict(list)    # old name -> [definition]
    tag_definitions = defaultdict(list)  # old tag -> [definition]
    for name in files:
        for entry in typedefs_in(texts[name]):
            tokens = entry["tokens"]
            family = family_of(name)
            if entry["kind"] in AGGREGATES:
                plain = []
                for begin, end in entry["declarators"]:
                    if end - begin == 1 and tokens[begin].kind == "ident":
                        plain.append(tokens[begin].text)
                anchor_name = plain[0] if plain else entry["tag"]
                if not anchor_name:
                    continue
                target = overrides.get("file_names", {}).get(name, {}).get(anchor_name) or \
                    new_name(anchor_name, entry["kind"], entry["form"], prefixes[family], overrides)
                record = dict(file=name, kind=entry["kind"], form=entry["form"], old=anchor_name,
                              tag=entry["tag"], new=target)
                for old in plain:
                    definitions[old].append(dict(record, old=old))
                if entry["tag"]:
                    tag_definitions[entry["tag"]].append(record)
            else:
                old = tokens[entry["name_index"]].text
                definitions[old].append(dict(file=name, kind="other", form=entry["form"], old=old,
                                             tag=None, new=new_name(old, "other", entry["form"],
                                                                    prefixes[family], overrides)))
    # Standalone CamelCase tags (struct Foo {...}; without typedef).
    for name in files:
        tokens = significant(tokenize(texts[name]))
        for j in range(len(tokens) - 2):
            if tokens[j].text in AGGREGATES and tokens[j + 1].kind == "ident" and \
                    tokens[j + 2].text == "{" and tokens[j + 1].text != tokens[j + 1].text.lower() and \
                    not (j > 0 and tokens[j - 1].text == "typedef"):
                old = tokens[j + 1].text
                if old not in tag_definitions:
                    target = overrides.get("names", {}).get(old) or new_name(
                        old, tokens[j].text, "definition", prefixes[family_of(name)], overrides)
                    tag_definitions[old].append(dict(file=name, kind=tokens[j].text, form="tag-only",
                                                      old=old, tag=old, new=target))

    # Copies of the same declaration in several families (for example an
    # imported board header) must keep one name: give every textually
    # identical definition the name chosen for its first (labs/-preferred)
    # occurrence.
    for old, defs in definitions.items():
        groups = defaultdict(list)
        for d in defs:
            # Only full definitions can be copies of one type; identical
            # forward declarations (`typedef struct Context Context;`) in
            # different files name unrelated structures.
            if d["form"] == "forward":
                continue
            groups[definition_text(texts[d["file"]], old)].append(d)
        for group in groups.values():
            group.sort(key=lambda d: (not d["file"].startswith("labs/"), d["file"]))
            for d in group[1:]:
                d["new"] = group[0]["new"]

    problems = []
    # Already-lowercase names (for example `wide`, `split`) are ordinary words
    # that also name variables and members; renaming them by token would be
    # unsafe. They stay for the semantic rewrite, where the style check keeps
    # reporting any remaining aggregate typedef.
    unchanged = {old for old, defs in definitions.items()
                 if old == old.lower() or all(d["kind"] == "other" and d["new"] == old for d in defs)}
    distinctive = {old for old, defs in definitions.items()
                   if len({d["new"] for d in defs}) == 1 and re.search(r"[A-Z]", old)
                   and not re.fullmatch(r"[A-Z][a-z0-9]*", old)}
    closures = dependency_closures(files)
    # Tags spelled in each file today (including untouched lowercase ones such
    # as `enum cl_owner`), so a new tag never collides with an existing one.
    file_tags = {}
    for name in files:
        toks = significant(tokenize(texts[name]))
        file_tags[name] = {toks[j + 1].text for j in range(len(toks) - 1)
                           if toks[j].text in AGGREGATES and toks[j + 1].kind == "ident"}
    # Per-file resolution of every renamed identifier.
    file_maps = {}
    for name in files:
        text = texts[name]
        visible = visible_headers(name, text, texts, by_suffix) | closures.get(name, set())
        all_tokens = significant(tokenize(text))
        idents = {t.text for t in all_tokens if t.kind == "ident"}
        # Namespaces: an identifier after struct/union/enum is a tag; after
        # '.' or '->' a member; otherwise an ordinary identifier.
        tag_uses, ordinary = set(), set()
        for j, t in enumerate(all_tokens):
            if t.kind != "ident":
                continue
            before = all_tokens[j - 1].text if j else ""
            if before in AGGREGATES:
                tag_uses.add(t.text)
            elif before not in (".", "->"):
                ordinary.add(t.text)
        mapping, tag_mapping = {}, {}
        for old in ordinary & set(definitions):
            if old in unchanged:
                continue
            own = {(d["new"], d["kind"]) for d in definitions[old] if d["file"] == name}
            choices = own or {(d["new"], d["kind"]) for d in definitions[old] if d["file"] in visible}
            if not choices and old in distinctive:
                choices = {(d["new"], d["kind"]) for d in definitions[old]}
            if not choices:
                continue
            if len(choices) == 1:
                mapping[old] = choices.pop()
            else:
                problems.append(dict(file=name, name=old, problem="ambiguous", choices=sorted(choices)))
        for old in tag_uses & set(tag_definitions):
            choices = {d["new"] for d in tag_definitions[old]
                       if d["file"] == name or d["file"] in visible}
            if not choices and len({d["new"] for d in tag_definitions[old]}) == 1:
                choices = {d["new"] for d in tag_definitions[old]}
            if not choices:
                continue
            if len(choices) == 1:
                tag_mapping[old] = choices.pop()
            else:
                problems.append(dict(file=name, name=old, problem="ambiguous-tag", choices=sorted(choices)))
        # Capture check. Tags live in their own namespace, so `struct ad_flags`
        # may coexist with a function `ad_flags` (as `struct stat` and `stat()`
        # do); only an existing tag of that spelling is a clash. Typedef names
        # that remain ordinary identifiers must not collide with any identifier.
        tokens = significant(tokenize(text))
        existing_tags = set(file_tags[name])
        for other in visible:
            existing_tags |= file_tags.get(other, set())
        for old, (target, kind) in mapping.items():
            if target == old:
                continue
            if kind == "other" and target in idents:
                problems.append(dict(file=name, name=old, new=target, problem="capture"))
            elif kind != "other" and target in existing_tags and target not in tag_mapping.values() \
                    and target not in tag_mapping and old not in existing_tags:
                problems.append(dict(file=name, name=old, new=target, problem="tag-clash"))
        # Two different old names must not land on the same new tag or name in
        # one file (for example a local `Sparse` and a header's `DSparse`).
        targets = defaultdict(set)
        for old, (target, kind) in mapping.items():
            targets[target].add((old, kind))
        # Tags defined by visible headers count even when this file never
        # names them: a local `Owner` must not become a header enum's tag.
        for old, defs in definitions.items():
            for d in defs:
                if d["kind"] in AGGREGATES and d["file"] in visible and d["file"] != name \
                        and d["new"] in targets and old not in mapping:
                    targets[d["new"]].add((old, d["kind"]))
        for old, target in tag_mapping.items():
            targets[target].add((old, "tag"))
        for target, sources in targets.items():
            names = {o for o, _ in sources}
            kinds = {k for _, k in sources if k != "tag"}
            if len(kinds) > 1 or (len(names) > 1 and not tag_and_typedef_pair(names, definitions, tag_definitions)):
                problems.append(dict(file=name, name=sorted(names), new=target, problem="collision"))
        if mapping or tag_mapping:
            file_maps[name] = dict(types={k: list(v) for k, v in sorted(mapping.items())},
                                   tags=dict(sorted(tag_mapping.items())))
    result = dict(schema_version=1, prefixes=dict(sorted(prefixes.items())),
                  definitions={k: v for k, v in sorted(definitions.items()) if k not in unchanged},
                  tags={k: v for k, v in sorted(tag_definitions.items())},
                  files=file_maps, problems=problems)
    Path(args.output).write_text(json.dumps(result, indent=1) + "\n")
    print(json.dumps(dict(files=len(file_maps), type_names=len(result["definitions"]),
                          tags=len(tag_definitions), problems=Counter(p["problem"] for p in problems))))
    return 1 if problems else 0


def tag_and_typedef_pair(names, definitions, tag_definitions):
    """True when the names are one struct's tag and its typedef name."""
    for name in names:
        for record in definitions.get(name, []):
            if record.get("tag") and set(names) <= {name, record["tag"]}:
                return True
    return False


def dependency_closures(files):
    """Ask the compiler which in-scope headers each translation unit sees.

    A header's visibility is the union of the closures of every unit that
    includes it, because headers often rely on what their includer included
    first. Units the heuristic flags cannot preprocess contribute nothing.
    """
    from concurrent.futures import ThreadPoolExecutor
    sys.path.insert(0, str(ROOT / "tools"))
    from source_equivalence import include_dirs

    in_scope = set(files)

    def deps(relative):
        command = ["gcc", "-std=gnu17", "-MM", "-MG", "-w",
                   *("-I" + str(d) for d in include_dirs(relative, ROOT)), relative]
        result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
        if result.returncode:
            return relative, set()
        names = result.stdout.replace("\\\n", " ").split()[1:]
        found = set()
        for item in names:
            path = os.path.relpath(os.path.normpath(os.path.join(ROOT, item)), ROOT)
            if path in in_scope and path != relative:
                found.add(path)
        return relative, found

    units = [f for f in files if f.endswith(".c")]
    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        closures = dict(pool.map(deps, units))
    for unit, headers in list(closures.items()):
        for header in headers:
            closures.setdefault(header, set()).update(headers | {unit} - {header})
    return closures


def definition_text(text, name):
    """Token text of the typedef that declares `name` in `text` (for copy detection)."""
    for entry in typedefs_in(text):
        toks = entry["tokens"]
        if any(t.kind == "ident" and t.text == name for t in toks[entry["typedef"]:entry["end"] + 1]):
            return " ".join(t.text for t in toks[entry["typedef"]:entry["end"] + 1])
    return None


def visible_headers(name, text, texts, by_suffix, depth=4):
    """Headers transitively included by `name`, preferring its own family."""
    seen, frontier = set(), [(name, text)]
    for _ in range(depth):
        following = []
        for current, body in frontier:
            for include in INCLUDE.findall(body):
                local = os.path.normpath(str(PurePosixPath(current).parent / include))
                if local in texts:
                    chosen = [local]
                else:
                    candidates = by_suffix.get(os.path.normpath(include).lstrip("./").lstrip("../"), [])
                    same = [c for c in candidates if family_of(c) == family_of(current)]
                    chosen = same or candidates
                for header in chosen:
                    if header not in seen:
                        seen.add(header)
                        following.append((header, texts.get(header, "")))
        frontier = following
    return seen


# --------------------------------------------------------------------------
# Applying


def rewrite(text, file_map):
    tokens = significant(tokenize(text))
    types = {k: tuple(v) for k, v in file_map.get("types", {}).items()}
    tags = file_map.get("tags", {})
    edits = []          # (start, end, replacement)
    claimed = set()     # token indices already handled

    for entry in typedefs_in(text):
        toks = entry["tokens"]
        if entry["kind"] in AGGREGATES:
            plain = [b for b, e in entry["declarators"] if e - b == 1 and toks[b].kind == "ident"]
            anchor = toks[plain[0]].text if plain else entry["tag"]
            # Convert the declaration only when its typedef name is renamed;
            # a tag-only rename is handled at the tag uses below.
            if not anchor or anchor not in types:
                continue
            target = types[anchor][0] if anchor in types else tags[entry["tag"]]
            first = entry["typedef"]
            last_head = first + (2 if entry["tag"] else 1)
            if entry["form"] == "definition":
                edits.append((toks[first].start, toks[last_head].end, f"{entry['kind']} {target}"))
                claimed.update(range(first, last_head + 1))
                extra = []
                for begin, end in entry["declarators"]:
                    if begin == (plain[0] if plain else None):
                        continue
                    piece = text[toks[begin].start:toks[end - 1].end]
                    extra.append(rename_fragment(piece, types, tags))
                # Remove the declarators between '}' and ';'.
                close, semicolon = toks[entry["close"]], toks[entry["end"]]
                edits.append((close.end, semicolon.start, ""))
                claimed.update(range(entry["close"] + 1, entry["end"]))
                if extra:
                    edits.append((semicolon.end, semicolon.end,
                                  "".join(f" typedef {entry['kind']} {target} {d};" for d in extra)))
            else:  # forward: typedef struct Tag Name[, *Ptr];
                pieces = []
                for begin, end in entry["declarators"]:
                    if end - begin == 1 and toks[begin].kind == "ident":
                        continue
                    pieces.append(rename_fragment(text[toks[begin].start:toks[end - 1].end], types, tags))
                replacement = f"{entry['kind']} {target};" + "".join(
                    f" typedef {entry['kind']} {target} {d};" for d in pieces)
                edits.append((toks[first].start, toks[entry["end"]].end, replacement))
                claimed.update(range(first, entry["end"] + 1))
    for j, token in enumerate(tokens):
        if j in claimed or token.kind != "ident":
            continue
        previous = tokens[j - 1].text if j else ""
        if previous in (".", "->"):
            continue
        if previous in AGGREGATES:
            if token.text in tags:
                edits.append((token.start, token.end, tags[token.text]))
            continue
        if token.text in types:
            target, kind = types[token.text]
            replacement = target if kind == "other" else f"{kind} {target}"
            edits.append((token.start, token.end, replacement))
    edits.sort()
    for (s1, e1, _), (s2, _, _) in zip(edits, edits[1:]):
        if s2 < e1:
            raise ValueError("overlapping rename edits")
    for start, end, replacement in reversed(edits):
        text = text[:start] + replacement + text[end:]
    return text


def rename_fragment(piece, types, tags):
    out = piece
    for old, (target, kind) in types.items():
        out = re.sub(rf"\b{re.escape(old)}\b", target if kind == "other" else f"{kind} {target}", out)
    return out


def apply(args):
    data = json.loads(Path(args.plan).read_text())
    if data["problems"] and not args.allow_problems:
        raise SystemExit("plan has unresolved problems; fix overrides first")
    changed = 0
    for name, file_map in data["files"].items():
        if args.family and family_of(name) not in args.family:
            continue
        path = ROOT / name
        before = path.read_text(errors="surrogateescape")
        after = rewrite(before, file_map)
        if after != before:
            if before.count("\n") != after.count("\n"):
                raise SystemExit(f"line count changed in {name}")
            path.write_text(after, errors="surrogateescape")
            changed += 1
    print(json.dumps(dict(changed=changed)))
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("plan")
    p.add_argument("--output", required=True)
    a = sub.add_parser("apply")
    a.add_argument("--plan", required=True)
    a.add_argument("--family", action="append", default=[])
    a.add_argument("--allow-problems", action="store_true")
    args = parser.parse_args()
    return plan(args) if args.command == "plan" else apply(args)


if __name__ == "__main__":
    raise SystemExit(main())
