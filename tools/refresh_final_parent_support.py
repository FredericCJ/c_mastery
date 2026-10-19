"""Refresh the three existing scoped parent producers; never merge inventory.

Each invocation preserves the prior inputs/maps/envelopes in a fresh directory.
The residual intake keeps its original scope: rerunning its discovery against
an already reconciled inventory would silently discard its original parents.
"""
import concurrent.futures
import argparse
from datetime import datetime, timezone
import json
import subprocess
import sys
from common import ROOT, dump, sha


def main():
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    base = ROOT/'reviews/v5/expansion/followthrough'/('final_parent_refresh_'+stamp)
    base.mkdir(exist_ok=False)
    specs = {
        'inventory_alias_followthrough': ('audit_inventory_aliases', 'build_inventory_aliases', 'check_inventory_aliases'),
        'named_parent_followthrough': ('audit_named_parents', 'build_named_parents', 'check_named_parents'),
        'residual_followthrough': (None, 'build_residual_followthrough_support', 'check_residual_followthrough'),
    }
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--packages',nargs='+',choices=list(specs))
    args=parser.parse_args()
    if args.packages:specs={name:specs[name] for name in args.packages}
    prior = []
    for name in specs:
        for leaf in ('inspection_input.json', 'evidence_map.json', 'support.json'):
            source = ROOT/'research/v5/expansion'/name/leaf
            target = base/'history'/name/leaf
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(source.read_bytes())
            assert sha(target) == sha(source)
            prior.append(dict(path=source.relative_to(ROOT).as_posix(), sha256=sha(source),
                              preserved=target.relative_to(ROOT).as_posix()))
    dump(base/'preservation.json', dict(files=prior))

    def refresh(item):
        name, (audit, build, check) = item
        guard = 'reviews/v5/expansion/'+name+'/final_guard_'+stamp+'.json'
        commands = ([[sys.executable, '-X', 'utf8', 'tools/'+audit+'.py']] if audit else [])
        commands += [[sys.executable, '-X', 'utf8', 'tools/'+build+'.py'],
                     [sys.executable, '-X', 'utf8', 'tools/'+check+'.py', '--self-test', '--report', guard]]
        records = []
        for index, command in enumerate(commands):
            result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                    encoding='utf8', errors='strict', timeout=1800)
            log = base/(name+'-'+str(index)+'.log')
            log.write_text(result.stdout+'\n'+result.stderr, encoding='utf8', newline='\n')
            records.append(dict(command=command, exit_code=result.returncode,
                                log=log.relative_to(ROOT).as_posix(), log_sha256=sha(log)))
            print(name, index, result.returncode, flush=True)
            if result.returncode:
                return dict(name=name, passed=False, executions=records)
        support = 'research/v5/expansion/'+name+'/support.json'
        value = json.loads((ROOT/support).read_text(encoding='utf8'))
        return dict(name=name, passed=True, support=support, support_sha256=sha(ROOT/support),
                    guard=guard, guard_sha256=sha(ROOT/guard), rows=len(value['updates']),
                    executions=records)

    # Named-parent scope pins the residual envelope as an intake source even
    # though it is not itself a selected child provider. Respect that concrete
    # package edge before taking the named-parent snapshot.
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
        first=[item for item in specs.items() if item[0]!='named_parent_followthrough']
        rows=list(pool.map(refresh,first))
        if 'named_parent_followthrough' in specs:
            if all(row['passed'] for row in rows):
                rows.append(refresh(('named_parent_followthrough',specs['named_parent_followthrough'])))
            else:rows.append(dict(name='named_parent_followthrough',passed=False,reason='Upstream package failed'))
    for row in rows:
        if not row['passed']:continue
        value=json.loads((ROOT/row['support']).read_text(encoding='utf8'))
        stale=[p for p,h in value['source_sha256'].items()
               if not (ROOT/p).is_file() or sha(ROOT/p)!=h]
        row['post_refresh_stale_inputs']=stale
        row['passed']=not stale
    result = dict(passed=all(row['passed'] for row in rows), packages=rows,
                  source_sha256={'tools/refresh_final_parent_support.py': sha(ROOT/'tools/refresh_final_parent_support.py')},
                  scope='Exact existing parent origins and all selected ordinary/delayed criteria with strict transitive guards; no new child execution or external observation inferred.')
    dump(base/'result.json', result)
    print(base.relative_to(ROOT).as_posix(), flush=True)
    return int(not result['passed'])


if __name__ == '__main__':
    raise SystemExit(main())
