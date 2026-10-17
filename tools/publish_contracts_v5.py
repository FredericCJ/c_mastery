"""Shared learner contracts have real publication targets, independent of paths."""
from pathlib import PurePosixPath
import posixpath
import re

from common import ROOT, anchor, read_json

FOUNDATION = 'curriculum/v5/exercises/shared-contract.md'
LEGACY = 'curriculum/v5/expansion/legacy_assessments/shared-contract.md'


def target(name):
    if name == FOUNDATION:
        return 'exercise-shared-contract', 'shared-contract'
    if name == LEGACY:
        # Owned by publish_legacy_assessments_v5, not duplicated here.
        return 'legacy-assessment-shared-contract', None
    identifier = 'exercise-contract-' + anchor(name)
    return identifier, 'contracts/' + anchor(name)


def registered():
    path = ROOT/'curriculum/v5/exercises.json'
    records = read_json(path)['exercises'] if path.is_file() else []
    records += [read_json(p) for p in
                (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')]
    authored = [p.relative_to(ROOT).as_posix() for p in
                (ROOT/'curriculum/v5/contracts').glob('*.md')]
    return sorted({FOUNDATION, *authored,
                   *[name for row in records for name in row['shared_contracts']]})


def reference(name, label=None):
    return '{ref}`' + (label or name) + ' <' + target(name)[0] + '>`'


def links(body, names, current=None):
    names = set(names)

    def replace(match):
        address = match[2].split('#', 1)[0]
        candidate = address
        if candidate not in names and current and '://' not in address:
            candidate = posixpath.normpath(str(PurePosixPath(current).parent / address))
        return reference(candidate, match[1]) if candidate in names else match[0]

    return re.sub(r'\[([^\]]+)\]\(([^)]+)\)', replace, body)
