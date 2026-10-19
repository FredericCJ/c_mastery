"""Refresh only the canonical T74 completion envelope from exact ready evidence."""
import hashlib
import json
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh-current-campaign', action='store_true',
                        help='Bind an actually rerun current campaign after validating every criterion and report.')
    args = parser.parse_args()
    path = ROOT/'research/v5/modern/object_runtime_completion_dispositions.json'
    result = json.loads(path.read_text(encoding='utf8'))
    readiness_path = 'research/v5/modern/object_runtime_objective_readiness.json'
    readiness = json.loads((ROOT/readiness_path).read_text(encoding='utf8'))
    if args.refresh_current_campaign:
        from check_object_runtime_followthrough import inspect
        campaign_path = 'reviews/v5/expansion/modern_completion/object_runtime_current_campaign.json'
        compiler_path = 'reviews/v5/expansion/modern_completion/object_runtime_statepoints.json'
        campaign = json.loads((ROOT/campaign_path).read_text(encoding='utf8'))
        compiler = json.loads((ROOT/compiler_path).read_text(encoding='utf8'))
        campaign_hash = hashlib.sha256((ROOT/campaign_path).read_bytes()).hexdigest()
        for row in readiness['criteria']:
            if campaign_path in row['pins']:
                row['pins'][campaign_path] = campaign_hash
        # No source, criterion, other report or independence field is repinned.
        # The existing guard checks observed profiles, assertions, controls,
        # current source/log hashes and every untouched criterion before writing.
        inspect(readiness, campaign, compiler)
        (ROOT/readiness_path).write_text(json.dumps(readiness, indent=2)+'\n', encoding='utf8')
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
