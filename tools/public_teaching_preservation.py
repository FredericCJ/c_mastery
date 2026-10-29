"""Exact public teaching successors, without claiming historical author evidence.

B06's strict author validator also checks retained execution receipts. This
separate proof checks only the two additive project attachments and the readable
public documents they supply. Its pinned map preserves the original byte ranges;
it never imports the author validator or opens assessor/review/private content.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re

from common import ROOT
from native_teaching_revision import qualifies as native_qualifies

BASE = 'research/v5/expansion/b06_document_utf8'
GUIDE = 'curriculum/guides/b06-current-documents.md'
F439 = 'labs/firmware_lifecycle/targets/F439_UPDATE_UTF8.md'
PINS = {
    BASE+'/encoding_map.json': 'e5a2d162583e29926970ff646463f3370754983a171013d443652199e65bf567',
    BASE+'/bindings.json': '4a560073b61f990358be328c093a5502e8afbb980011f30364590d374f0d82de',
    GUIDE: '498f2c80ce2b44c1773a71b692de06e233c3b1c99da437eb1ffbb0c11f77d2a0',
    F439: '41de9125bd5b2641d14ddf62d1ee9029eb2ce0a7c500da48e960448be89ecb1c',
}
PROJECTS = {'curriculum/projects/PR-T28-06.md': 'PR-T28-06',
            'curriculum/projects/PR-T29-04.md': 'PR-T29-04'}
HEADING = '\n## Current document renditions\n'
BODY = (HEADING + '\nUse the [current document guide](../guides/b06-current-documents.md) '
        'for the required readable teaching documents. Its UTF-8 renditions preserve '
        'the specified technical requirements; the original byte sequences remain '
        'available solely for historical source and execution identities. '
        'This publication correction adds no execution or learner evidence.\n')


def _read(root, name):
    path = Path(name)
    if (path.is_absolute() or '..' in path.parts
            or {'assessor', 'reviews', 'private', 'tools', 'planning'} & set(path.parts)):
        raise ValueError('Nonpublic preservation input: '+name)
    target = root/path
    if target.resolve() != root.resolve()/path:
        raise ValueError('Redirected public preservation input: '+name)
    return target.read_bytes()


def _hash(raw):
    return hashlib.sha256(raw).hexdigest()


def _transform(raw, row):
    if _hash(raw) != row['original_sha256'] or len(raw) != row['original_bytes']:
        raise ValueError('Original public document changed')
    cursor = 0
    result = bytearray()
    for patch in row['patches']:
        start, end = patch['byte_range']
        expected = bytes.fromhex(patch['original_hex'])
        replacement = bytes.fromhex(patch['replacement_utf8_hex'])
        if start < cursor or end != start + len(expected) or raw[start:end] != expected:
            raise ValueError('Recorded byte range differs')
        raw[cursor:start].decode('utf-8')
        if replacement.decode('utf-8') != chr(int(patch['codepoint'][2:], 16)):
            raise ValueError('Recorded replacement differs')
        result.extend(raw[cursor:start])
        result.extend(replacement)
        cursor = end
    raw[cursor:].decode('utf-8')
    result.extend(raw[cursor:])
    transformed = bytes(result)
    transformed.decode('utf-8')
    if len(transformed) != row['current_bytes'] or _hash(transformed) != row['current_sha256']:
        raise ValueError('Transformed public document identity differs')
    return transformed


def qualifies(name, expected_sha, *, root=ROOT):
    """Qualify the exact public successor, never the stronger author receipt."""
    root = Path(root)
    if name not in PROJECTS:
        return native_qualifies(name, expected_sha, root=root)
    try:
        # Pin the scope of these existing corrections, not newly recomputed hashes.
        pinned = {path: _read(root, path) for path in PINS}
        if any(_hash(pinned[path]) != digest for path, digest in PINS.items()):
            return False
        mapping = json.loads(pinned[BASE+'/encoding_map.json'])
        binding = json.loads(pinned[BASE+'/bindings.json'])
        public_rows = [row for row in mapping['documents'] if row['original'].startswith('labs/')]
        expected_assets = [row['current'] for row in public_rows
                           if row['original'].startswith('labs/firmware_lifecycle/')]
        expected_assets += [F439]
        expected_assets += [row['current'] for row in public_rows
                            if row['original'] == 'labs/subset_compiler/extension/README.md']
        project = PROJECTS[name]
        if binding['guide'] != GUIDE or binding['projects'][project] != expected_assets:
            return False
        for row in public_rows:
            if _read(root, row['current']) != _transform(_read(root, row['original']), row):
                return False
        predecessors = json.loads(_read(root, BASE+'/predecessor_projects.json'))['projects']
        original = predecessors[project]
        if _hash(original.encode('utf-8')) != expected_sha:
            return False
        match = re.match(r'\A```json\s*\n(.*?)\n```\n?', original, re.S)
        if match is None:
            return False
        meta, body = json.loads(match[1]), original[match.end():]
        meta['shared_contracts'] = list(dict.fromkeys(meta.get('shared_contracts', []) + [GUIDE]))
        meta['supplied_artifacts'] = list(dict.fromkeys(meta.get('supplied_artifacts', []) + expected_assets))
        meta['document_rendition_support'] = BASE+'/completion_dispositions.json'
        if HEADING not in body:
            body = body.rstrip() + '\n' + BODY
        current = '```json\n'+json.dumps(meta, indent=2, ensure_ascii=False)+'\n```\n'+body
        return _read(root, name) == current.encode('utf-8')
    except (OSError, KeyError, ValueError, TypeError, UnicodeError):
        return False
