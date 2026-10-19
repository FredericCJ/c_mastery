"""Reject ambiguous research/author evidence classifications without promoting them."""
import copy
import json
from pathlib import Path
import tempfile
from common import ROOT, dump, sha
from expansion_evidence import require_research
from research_batches_v5 import discover


def main():
    require_research()
    batches, authors = discover()
    controls = []
    with tempfile.TemporaryDirectory(prefix='research-gate-') as temp:
        root = Path(temp)
        folder = root/'research/v5/expansion/example'
        folder.mkdir(parents=True)
        gate = folder/'research_gate.json'; resources = folder/'resources.json'
        dump(gate, dict(state='author-ready', guard_required='tools/check_example.py'))
        dump(resources, dict(resources=[dict(id='R-EXAMPLE')]))
        guard = root/'tools/check_example.py'; guard.parent.mkdir(); guard.write_text('# fixture\n')
        resource_name = resources.relative_to(root).as_posix(); gate_name = gate.relative_to(root).as_posix()
        source_policy = dict(evidence_only={resource_name: dict(role='author-evidence-only', sha256=sha(resources), ids=['R-EXAMPLE'])})
        policy = dict(schema_version=1, author_evidence={gate_name: dict(role='author-evidence-only', reason='Explicit fixture evidence, no passage claim', gate_sha256=sha(gate), resource_sha256={resource_name: sha(resources)}, guard_required='tools/check_example.py')})
        source_path = root/'research/v5/resource_merge_policy.json'
        policy_path = root/'research/v5/expansion/research_gate_reconciliation/policy.json'
        dump(source_path, source_policy); dump(policy_path, policy)
        assert discover(root)[0] == [] and len(discover(root)[1]) == 1
        for name in ['wrong-role', 'missing-reason', 'stale-gate', 'stale-register', 'missing-guard', 'conflicting-guard', 'unknown-resource', 'wrong-source-role', 'wrong-resource-id', 'disappeared-gate']:
            p = copy.deepcopy(policy); s = copy.deepcopy(source_policy); item = p['author_evidence'][gate_name]
            if name == 'wrong-role': item['role'] = 'passage-inspected'
            elif name == 'missing-reason': item['reason'] = ''
            elif name == 'stale-gate': item['gate_sha256'] = '0'*64
            elif name == 'stale-register': item['resource_sha256'][resource_name] = '0'*64
            elif name == 'missing-guard': item['guard_required'] = 'tools/check_missing.py'
            elif name == 'conflicting-guard':
                (root/'tools/check_other.py').write_text('# fixture\n'); item['guard_required'] = 'tools/check_other.py'
            elif name == 'unknown-resource': s['evidence_only'].clear()
            elif name == 'wrong-source-role': s['evidence_only'][resource_name]['role'] = 'reading'
            elif name == 'wrong-resource-id': s['evidence_only'][resource_name]['ids'] = ['OTHER']
            else: p['author_evidence'][gate_name.replace('example/', 'absent/')] = item; del p['author_evidence'][gate_name]
            dump(source_path, s); dump(policy_path, p)
            try:
                discover(root)
            except ValueError:
                controls.append(dict(name=name, rejected=True))
            else:
                raise AssertionError('Accepted '+name)
        dump(source_path, source_policy); dump(policy_path, dict(schema_version=1, author_evidence={}))
        assert discover(root)[0] == ['EXAMPLE'], 'Unclassified malformed gate must reach strict passage validation'
    paths = ['tools/research_batches_v5.py', 'tools/check_research_batches_v5.py',
             'tools/expansion_evidence.py', 'tools/reconcile_expansion_manifest.py',
             'research/v5/expansion/research_gate_reconciliation/policy.json',
             'research/v5/resource_merge_policy.json']
    report = dict(passed=True, passage_batches=len(batches), author_evidence_batches=len(authors),
                  negative_controls=controls, source_sha256={p: sha(ROOT/p) for p in paths},
                  scope='Classification and actual passage-gate verification only. Named author guards remain separately required; no implementation acceptance is inferred.')
    dump(ROOT/'reviews/v5/expansion/research_gate_reconciliation/classification.json', report)
    print(json.dumps(dict(passed=True, passages=len(batches), author_evidence=len(authors), controls=len(controls))))


if __name__ == '__main__':
    main()
