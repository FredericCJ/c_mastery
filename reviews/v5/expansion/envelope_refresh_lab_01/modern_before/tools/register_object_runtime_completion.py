"""Refresh only the canonical T74 completion envelope from exact ready evidence."""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    path = ROOT/'research/v5/modern/object_runtime_completion_dispositions.json'
    result = json.loads(path.read_text(encoding='utf8'))
    readiness_path = 'research/v5/modern/object_runtime_objective_readiness.json'
    readiness = json.loads((ROOT/readiness_path).read_text(encoding='utf8'))
    inputs = {readiness_path,
              'research/v5/modern/object_runtime_gap_ledger.json',
              'research/v5/modern/object_runtime_resources.json',
              'research/v5/modern/object_runtime_sources.md',
              'research/v5/modern/object_runtime_research_gate.json',
              'tools/check_object_runtime_followthrough.py',
              'tools/register_object_runtime_completion.py'}
    reports = set()
    for row in readiness['criteria']:
        for item, expected in row['pins'].items():
            assert hashlib.sha256((ROOT/item).read_bytes()).hexdigest() == expected, item
            inputs.add(item)
            if item.endswith('.json'):
                value = json.loads((ROOT/item).read_text(encoding='utf8'))
                if 'inspected_paths' in value:
                    assert value['passed'] and value['status'] == 'passed', item
                    reports.add(item)
                    for source, source_hash in value['inspected_paths'].items():
                        assert hashlib.sha256((ROOT/source).read_bytes()).hexdigest() == source_hash, source
                        inputs.add(source)
    result.update(domain='T74',
        scope='Exact original-ID author-support dispositions; individual criteria are bound in the referenced readiness map. No independent learner, external reviewer or physical claim.',
        current_reports=sorted(reports),
        input_sha256={item: hashlib.sha256((ROOT/item).read_bytes()).hexdigest() for item in sorted(inputs)},
        summary={'updates':len(result['updates']), 'criteria':len(readiness['criteria']),
                 'ready_bounded':sum(row['author_readiness']=='ready-bounded' for row in readiness['criteria'])})
    path.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'path':path.relative_to(ROOT).as_posix(),'current_reports':len(reports),'input_pins':len(inputs),**result['summary']}))


if __name__=='__main__':
    main()
