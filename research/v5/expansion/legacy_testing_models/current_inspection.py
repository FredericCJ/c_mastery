"""Current, bounded fir/queue inspection; the original inspection stays immutable."""
from __future__ import annotations

import ast
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[4]
BASE = 'research/v5/expansion/legacy_testing_models'
ORIGINAL = BASE + '/source_access.json'
ORIGINAL_SHA = '7de0bec231f73190f5012c1affd4b30d1d2cf6a33818bdd68e68835e11667ba3'
ORIGINAL_GATE_SHA = 'cbbbd6241f0753aa7a1cf589e50c3a95a03287b6e8acc0a9b44e754416465729'
RESOURCES = BASE + '/resources.json'
RESOURCES_SHA = '1abc4d42dd7df3bf0520bbfe01494acb4fe3247e0493cab5481fd287b5475459'
CURRENT = BASE + '/source_access_current.json'
VERIFIER = 'tools/verify_embedded_fixtures.py'
FUNCTIONS = {
    'fir': '63f4d504e6c7b912929e688fc7bdad9b5ee35d153e1e022439c4962ca4d2c68f',
    'queue': '12d9a61a3855a1b345febea279d2681fbfb35229c099a04a42658e5298f5e791',
}
INSPECTION_DATE = '2026-10-18'
sys.path.insert(0, str(ROOT / 'tools'))
import fixture_inspection_successor as fixture_successor


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def raw(root, name):
    return fixture_successor.local(root, name).read_bytes()


def expected(root=ROOT):
    root = Path(root).resolve()
    original_raw = raw(root, ORIGINAL)
    if sha(original_raw) != ORIGINAL_SHA:
        raise ValueError('Original dated source inspection changed')
    original = fixture_successor.read_json(original_raw)
    if sha(raw(root, RESOURCES)) != RESOURCES_SHA:
        raise ValueError('Original resources changed')
    errors = fixture_successor.validate(root)
    if errors:
        raise ValueError('Fixture predecessor proof: ' + '; '.join(errors))
    inspected = deepcopy(original['inspected_paths'])
    for name, digest in inspected.items():
        actual = sha(raw(root, name))
        if name == VERIFIER:
            if digest != fixture_successor.ORIGINALS[VERIFIER]['sha256']:
                raise ValueError('Wrong original verifier identity')
            inspected[name] = actual
        elif actual != digest:
            raise ValueError('Unrelated historical inspection input changed: ' + name)
    for identity in original['private_source_identities']:
        fixture_successor.pinned(root, identity)
    text = raw(root, VERIFIER).decode('utf8')
    functions = {n.name: sha(ast.get_source_segment(text, n).encode('utf8'))
                 for n in ast.parse(text).body
                 if isinstance(n, ast.FunctionDef) and n.name in FUNCTIONS}
    if functions != FUNCTIONS:
        raise ValueError('Inspected fir/queue functions changed')
    return {
        'schema_version': 2,
        'historical_record': {'record_path': ORIGINAL, 'record_sha256': ORIGINAL_SHA,
                              'original_date': original['date']},
        'historical_inspections_rewritten': False,
        'new_c_execution': False,
        'physical_observations': 0,
        'inspected_paths': inspected,
        'private_source_identities': deepcopy(original['private_source_identities']),
        'retained_historical_inspections': deepcopy(original['actual_inspections']),
        'original_analysis': original['original_analysis'],
        'limits': original['limits'],
        'fresh_inspection': {
            'date': INSPECTION_DATE,
            'source': VERIFIER,
            'edition': 'Current original curriculum fixture-category successor',
            'method': 'Read complete fir() and queue() bodies; compare retained predecessor functions',
            'function_source_sha256': functions,
            'scope': ['fir()', 'queue()'],
            'findings': [
                'fir returns one Fraction convolution result per input sample; n >= j omits unavailable prior samples. This is finite zero-extended list arithmetic, without streaming state or saturation.',
                'queue reserves one of slots entries, increments dropped on a full push, removes index zero on a nonempty pop, leaves an empty pop unchanged, and rejects unknown event kinds.',
            ],
            'limits': 'Only these two unchanged Python functions were freshly inspected. The other dated inspections are retained historical records. No whole-file inspection, C execution, arbitrary concurrent queue behavior, floating-point accuracy, or physical observation is established.',
        },
        'qualification': 'Original inspection bytes/date retained; current fir/queue passage identity and fixture-category successor proof validated separately from unchanged model C campaign evidence.',
        'source_sha256': {
            name: sha(raw(root, name)) for name in (
                fixture_successor.PACKET, 'tools/fixture_inspection_successor.py',
                BASE + '/current_inspection.py', 'tools/check_testing_models_inspection.py')
        },
    }


def validate(record=None, root=ROOT):
    try:
        if record is None:
            record = fixture_successor.read_json(raw(Path(root).resolve(), CURRENT))
        if json.dumps(record, sort_keys=True) != json.dumps(expected(root), sort_keys=True):
            raise ValueError('Current inspection differs from the exact bounded successor')
        return []
    except (OSError, ValueError, KeyError, TypeError, SyntaxError) as error:
        return [str(error)]


def refresh(root=ROOT):
    root = Path(root).resolve()
    record = expected(root)
    gate_path = root / BASE / 'research_gate.json'
    gate_raw = gate_path.read_bytes()
    gate = fixture_successor.read_json(gate_raw)
    # The first transition is tied to the original gate. Subsequent refreshes
    # must already name this exact successor, not an unrelated replacement.
    if sha(gate_raw) != ORIGINAL_GATE_SHA:
        if (gate.get('current_scope_successor') != CURRENT or
                gate.get('historical_gate_sha256') != ORIGINAL_GATE_SHA):
            raise ValueError('Unrecognized predecessor research gate')
    for path in (root / CURRENT, gate_path):
        if path.exists():
            old = path.read_bytes()
            saved = root / BASE / 'history' / 'source_inspection' / sha(old) / path.name
            saved.parent.mkdir(parents=True, exist_ok=True)
            if saved.exists() and saved.read_bytes() != old:
                raise ValueError('Changed preserved inspection version')
            saved.write_bytes(old)
    destination = root / CURRENT
    destination.write_text(json.dumps(record, indent=2) + '\n', encoding='utf8', newline='\n')
    gate = {
        'status': 'passage-inspected', 'domains': ['T32'],
        'inspection_kind': 'Retained historical passages plus current bounded fir/queue inspection',
        'current_scope_successor': CURRENT,
        'historical_gate_sha256': ORIGINAL_GATE_SHA,
        'research_input_sha256': {name: sha(raw(root, name)) for name in (RESOURCES, CURRENT)},
    }
    gate_path.write_text(json.dumps(gate, indent=2) + '\n', encoding='utf8', newline='\n')
    return {'passed': not validate(root=root), 'record': CURRENT,
            'historical_record_unchanged': sha(raw(root, ORIGINAL)) == ORIGINAL_SHA,
            'new_c_execution': False}
