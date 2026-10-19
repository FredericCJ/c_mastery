"""Separate assignable passage gates from explicitly retained author evidence.

This classification does not accept an implementation or replace its checker.
Unclassified malformed gates still reach the strict passage validator and fail.
"""
from pathlib import Path
from common import ROOT, read_json, sha


def discover(root=ROOT):
    root = Path(root)
    policy = read_json(root/'research/v5/expansion/research_gate_reconciliation/policy.json')
    sources = read_json(root/'research/v5/resource_merge_policy.json')['evidence_only']
    if policy.get('schema_version') != 1 or not isinstance(policy.get('author_evidence'), dict):
        raise ValueError('Invalid research gate classification policy')
    classified = policy['author_evidence']; found = set(); batches = []; evidence = []
    for gate in sorted((root/'research/v5/expansion').glob('*/research_gate.json')):
        registers = sorted(gate.parent.glob('*resources.json'))
        if not registers:
            continue
        name = gate.relative_to(root).as_posix()
        if name not in classified:
            batches.append(gate.parent.name.upper())
            continue
        entry = classified[name]
        if (entry.get('role') != 'author-evidence-only' or not entry.get('reason')
                or entry.get('gate_sha256') != sha(gate)):
            raise ValueError('Stale or unsupported author gate classification: '+name)
        actual = {p.relative_to(root).as_posix(): sha(p) for p in registers}
        if actual != entry.get('resource_sha256'):
            raise ValueError('Changed author evidence resources: '+name)
        for path, digest in actual.items():
            source = sources.get(path, {})
            data = read_json(root/path)
            rows = data if isinstance(data, list) else data['resources']
            if (source.get('role') != 'author-evidence-only' or source.get('sha256') != digest
                    or source.get('ids') != [r['id'] for r in rows]):
                raise ValueError('Source/gate classification conflict: '+name)
        guard = entry.get('guard_required', '')
        if (not guard.startswith('tools/check_') or not guard.endswith('.py')
                or Path(guard).is_absolute() or '..' in Path(guard).parts
                or not (root/guard).is_file()):
            raise ValueError('Missing author evidence guard: '+name)
        declared = read_json(gate).get('guard_required')
        if declared and declared != guard:
            raise ValueError('Author guard identity conflict: '+name)
        evidence.append(dict(batch=gate.parent.name.upper(), gate=name, **entry))
        found.add(name)
    if found != set(classified):
        raise ValueError('Classified author gate disappeared')
    return batches, evidence
