"""Retain already observed guard JSON with exact command and immutable log evidence."""
import json
from pathlib import Path
from common import ROOT, sha

BASE = ROOT/'reviews/v5/expansion/envelope_refresh_lab_01'


def main():
    manifest = json.loads((BASE/'bulk_merge_ready.json').read_text(encoding='utf8'))
    mapped = {}
    for path in sorted(BASE.glob('*/result.json')):
        result = json.loads(path.read_text(encoding='utf8'))
        if not result.get('passed'):
            continue
        for row in result.get('results', []):
            for run in row['executions']:
                if '--self-test' not in run['command']:
                    continue
                assert run['exit_code'] == 0 and run['passed']
                assert sha(ROOT/run['log']) == run['log_sha256']
                assert sha(ROOT/run['command'][1]) == run['producer_sha256']
                observed = json.loads((ROOT/run['log']).read_text(encoding='utf8'))
                assert observed['passed']
                observed['observed_execution'] = run
                observed['source_result'] = path.relative_to(ROOT).as_posix()
                observed['source_result_sha256'] = sha(path)
                out = BASE/'guards'/(row['scope']+'.json')
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_text(json.dumps(observed, indent=2)+'\n', encoding='utf8')
                mapped[row['scope']] = out.relative_to(ROOT).as_posix()
    for item in manifest['packages']:
        path = Path(item['path'])
        name = path.name.removesuffix('_completion_dispositions.json')
        group = ('b02' if '/b02_followthrough/' in item['path'] else
                 'portability' if '/legacy_portability_followthrough/' in item['path'] else 'storage')
        key = group+'_'+name
        if key in mapped:
            item['guard'] = mapped[key]
            item['guard_sha256'] = sha(ROOT/mapped[key])
    (BASE/'bulk_merge_ready.json').write_text(json.dumps(manifest, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': True, 'observed_guards':len(mapped),
                      'portability_map':{p['path']:p['guard'] for p in manifest['packages']
                                         if '/legacy_portability_followthrough/' in p['path']}}))


if __name__ == '__main__':
    main()
