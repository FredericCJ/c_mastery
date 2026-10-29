"""Layout-independent views of C source text.

Mutation catalogs, instrumenters and source slicers used to locate code by
exact text, which breaks whenever whitespace or line breaks change. This module
locates code by its C token sequence instead, so the same anchor keeps matching
after the house-style reformat. It is a lexer, not a parser: it understands
comments, string/character literals (with encoding prefixes), preprocessing
numbers, punctuators, line splices and preprocessing-directive boundaries,
which is all that token-equivalence proofs and anchors need.

Typical use:

    from c_source import replace_unique
    mutated = replace_unique(source, "if(!grant)return -1;", "if(grant>1)return -1;")

The anchor must match exactly one place in the source; zero or several matches
raise AnchorError rather than silently producing an unmutated file.
"""
from __future__ import annotations

import hashlib
import json
import os
import re
from dataclasses import dataclass

__all__ = ["Token", "AnchorError", "tokenize", "significant", "comments",
           "find_all", "find_unique", "replace_unique", "replace_all_unique",
           "function_span", "same_tokens"]

PUNCTUATORS = sorted("""
%:%: ... <<= >>= -> ++ -- << >> <= >= == != && || *= /= %= += -= &= ^= |= ##
<: :> <% %> %: [ ] ( ) { } . & * + - ~ ! / % < > ^ | ? : ; = , #
""".split(), key=len, reverse=True)

_SPLICE = re.compile(r"\\\r?\n")
_IDENT = re.compile(r"[A-Za-z_$][A-Za-z0-9_$]*")
_PPNUMBER = re.compile(r"\.?[0-9](?:[eEpP][+-]|[0-9A-Za-z_.']|\\\r?\n)*")
_PREFIX = re.compile(r"(?:u8|u|U|L)?(?=[\"'])")


@dataclass(frozen=True)
class Token:
    kind: str      # ident, number, string, char, punct, comment, eol, other
    text: str
    start: int
    end: int


class AnchorError(ValueError):
    """An anchor matched zero or several places."""


def _scan_quoted(text, i, quote):
    """Return the index after the closing quote, honouring escapes."""
    i += 1
    while i < len(text):
        c = text[i]
        if c == "\\":
            i += 2
            continue
        if c == quote:
            return i + 1
        if c == "\n":
            return i  # unterminated: stop at the line end like a compiler would
        i += 1
    return i


def tokenize(text: str) -> list[Token]:
    """Tokenize C source text, keeping positions into the original string.

    A synthetic `eol` token marks the end of every preprocessing directive so
    that `#define A 1\\nint b;` and `#define A 1 int b;` differ.
    """
    tokens: list[Token] = []
    i, n = 0, len(text)
    line_start = True       # only whitespace/comments seen since the newline
    in_directive = False
    while i < n:
        c = text[i]
        if c == "\\" and _SPLICE.match(text, i):
            i = _SPLICE.match(text, i).end()
            continue
        if c == "\n":
            if in_directive:
                tokens.append(Token("eol", "", i, i))
                in_directive = False
            line_start = True
            i += 1
            continue
        if c in " \t\r\f\v":
            i += 1
            continue
        if text.startswith("//", i):
            j = i + 2
            while j < n:
                if text[j] == "\\" and _SPLICE.match(text, j):
                    j = _SPLICE.match(text, j).end()
                    continue
                if text[j] == "\n":
                    break
                j += 1
            tokens.append(Token("comment", text[i:j], i, j))
            i = j
            continue
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            tokens.append(Token("comment", text[i:j], i, j))
            i = j
            continue
        prefix = _PREFIX.match(text, i)
        if prefix and prefix.end() < n and text[prefix.end()] in "\"'":
            quote = text[prefix.end()]
            j = _scan_quoted(text, prefix.end(), quote)
            tokens.append(Token("string" if quote == '"' else "char", text[i:j], i, j))
            i = j
            line_start = False
            continue
        if c.isdigit() or (c == "." and i + 1 < n and text[i + 1].isdigit()):
            m = _PPNUMBER.match(text, i)
            tokens.append(Token("number", _SPLICE.sub("", m.group()), i, m.end()))
            i = m.end()
            line_start = False
            continue
        m = _IDENT.match(text, i)
        if m:
            tokens.append(Token("ident", m.group(), i, m.end()))
            i = m.end()
            line_start = False
            continue
        for p in PUNCTUATORS:
            if text.startswith(p, i):
                if p in ("#", "%:") and line_start:
                    in_directive = True
                tokens.append(Token("punct", p, i, i + len(p)))
                i += len(p)
                break
        else:
            tokens.append(Token("other", c, i, i + 1))
            i += 1
        line_start = False
    if in_directive:
        tokens.append(Token("eol", "", n, n))
    return tokens


def significant(tokens):
    """Tokens that affect translation: everything except comments."""
    return [t for t in tokens if t.kind != "comment"]


def _normal_comment(text):
    body = text[2:-2] if text.startswith("/*") else text[2:]
    lines = [line.strip() for line in _SPLICE.sub("", body).splitlines()]
    return " ".join(" ".join(line.lstrip("*").split()) if line.startswith("*")
                    else " ".join(line.split()) for line in lines).strip()


def comments(tokens):
    """Comment contents with layout (indentation, line breaks) normalized."""
    return [_normal_comment(t.text) for t in tokens if t.kind == "comment"]


def _key(tokens):
    return [(t.kind, t.text) for t in significant(tokens)]


def same_tokens(old: str, new: str) -> bool:
    """True when two texts have identical significant token sequences."""
    return _key(tokenize(old)) == _key(tokenize(new))


def find_all(text: str, anchor: str):
    """Character spans of every token-sequence match of `anchor` in `text`."""
    needle = _key(tokenize(anchor))
    if not needle:
        raise AnchorError("empty anchor")
    hay = significant(tokenize(text))
    keys = [(t.kind, t.text) for t in hay]
    spans = []
    first = needle[0]
    for i in range(len(keys) - len(needle) + 1):
        if keys[i] == first and keys[i:i + len(needle)] == needle:
            spans.append((hay[i].start, hay[i + len(needle) - 1].end))
    return spans


def _log_anchor(anchor, count):
    """Anchor census: CM_ANCHOR_LOG=path appends one JSON line per resolution."""
    path = os.environ.get("CM_ANCHOR_LOG")
    if path:
        with open(path, "a", encoding="utf-8") as log:
            log.write(json.dumps({"anchor_sha256": hashlib.sha256(anchor.encode()).hexdigest(),
                                  "anchor": anchor[:200], "matches": count}) + "\n")


def find_unique(text: str, anchor: str):
    spans = find_all(text, anchor)
    _log_anchor(anchor, len(spans))
    if len(spans) != 1:
        raise AnchorError(f"anchor matched {len(spans)} times: {anchor[:80]!r}")
    return spans[0]


def replace_unique(text: str, anchor: str, replacement: str) -> str:
    """Replace the single token-sequence match of `anchor` with `replacement`."""
    start, end = find_unique(text, anchor)
    return text[:start] + replacement + text[end:]


def replace_all_unique(text: str, pairs) -> str:
    """Apply several (anchor, replacement) pairs; each must match exactly once
    in the original text and the matches must not overlap."""
    spans = []
    for anchor, replacement in pairs:
        start, end = find_unique(text, anchor)
        spans.append((start, end, replacement))
    spans.sort()
    for (_, end_a, _), (start_b, _, _) in zip(spans, spans[1:]):
        if start_b < end_a:
            raise AnchorError("overlapping anchors")
    for start, end, replacement in reversed(spans):
        text = text[:start] + replacement + text[end:]
    return text


def function_span(text: str, name: str):
    """Span of the definition of function `name`, from the start of its
    declaration (return type) to its closing brace.

    The definition is the unique `name (` at brace depth zero whose matching
    parenthesis is followed by `{`.
    """
    tokens = significant(tokenize(text))
    depth = 0
    found = []
    for i, t in enumerate(tokens):
        if t.kind == "punct" and t.text == "{":
            depth += 1
        elif t.kind == "punct" and t.text == "}":
            depth -= 1
        elif depth == 0 and t.kind == "ident" and t.text == name and \
                i + 1 < len(tokens) and tokens[i + 1].text == "(":
            j, parens = i + 1, 0
            while j < len(tokens):
                if tokens[j].text == "(":
                    parens += 1
                elif tokens[j].text == ")":
                    parens -= 1
                    if parens == 0:
                        break
                j += 1
            if j + 1 < len(tokens) and tokens[j + 1].text == "{":
                found.append((i, j + 1))
    if len(found) != 1:
        raise AnchorError(f"function {name!r} defined {len(found)} times")
    name_index, brace_index = found[0]
    begin = name_index
    while begin > 0 and not (tokens[begin - 1].kind == "eol" or
                             tokens[begin - 1].text in (";", "}")):
        begin -= 1
    k, level = brace_index, 0
    while k < len(tokens):
        if tokens[k].text == "{":
            level += 1
        elif tokens[k].text == "}":
            level -= 1
            if level == 0:
                break
        k += 1
    return tokens[begin].start, tokens[k].end
