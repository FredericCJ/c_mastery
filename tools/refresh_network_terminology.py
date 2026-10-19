"""Rerun declared guide-bound producers, then their exact current envelopes.

The three guide qualifications are teaching-only changes, but the original
producer contracts include those bytes. Preserve the original reports and run
the actual producers; never relabel old source hashes as newly executed.
"""
import concurrent.futures
import argparse
from datetime import datetime, timezone
import json
import subprocess
import sys
from common import ROOT, dump, sha

BASE = ROOT/'reviews/v5/expansion/network_terminology_refresh_01'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--resume', action='store_true')
    options = parser.parse_args()
    if options.resume:
        prior = json.loads((BASE/'execution.json').read_text())
        assert not (BASE/'result.json').exists()
        assert prior['executions'][-1]['label'] in {'remaining-envelopes','aggregate','aggregate-guard'}
        for row in prior['executions']:
            assert sha(ROOT/row['log']) == row['log_sha256']
        stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
        (BASE/('prior_execution_'+stamp+'.json')).write_bytes((BASE/'execution.json').read_bytes())
        rows = prior['executions'][:-1] if prior['executions'][-1]['exit_code'] else prior['executions']
        assert all(row['exit_code'] == 0 for row in rows)
    else:
        BASE.mkdir(exist_ok=False)
        before = {}
        for family in ['arp', 'ip', 'tcp']:
            path = ROOT/f'reviews/v5/expansion/legacy_network_{family}/current_validation.json'
            saved = BASE/(family+'_previous_validation.json')
            saved.write_bytes(path.read_bytes())
            before[family] = dict(path=path.relative_to(ROOT).as_posix(), sha256=sha(saved),
                                  preserved=saved.relative_to(ROOT).as_posix())
        arp = ROOT/'research/v5/expansion/legacy_network_arp/completion_dispositions.json'
        (BASE/'arp_previous_completion.json').write_bytes(arp.read_bytes())
        dump(BASE/'preservation.json', dict(reports=before, arp_support_sha256=sha(arp)))

    def run(label, args):
        cmd = [sys.executable, *args]
        p = subprocess.run(cmd, cwd=ROOT, text=True, capture_output=True, timeout=3600)
        log = BASE/(label+('-resumed' if options.resume else '')+'.log')
        log.write_text(p.stdout+'\n'+p.stderr, encoding='utf8', newline='\n')
        row = dict(label=label, command=cmd, exit_code=p.returncode,
                   log=log.relative_to(ROOT).as_posix(), log_sha256=sha(log))
        print(label, p.returncode, flush=True)
        return row

    if not options.resume:
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
            rows = list(pool.map(lambda family: run(family+'-actual',
                ['tools/run_network_'+family+'_depth.py', '--report', before[family]['path']]),
                ['arp', 'ip', 'tcp']))
        dump(BASE/'execution.json', dict(passed=all(r['exit_code']==0 for r in rows), executions=rows))
        if any(r['exit_code'] for r in rows):
            return 1
    commands = [
        ('arp-export', ['tools/check_network_arp_exports.py']),
        ('arp-guard', ['tools/check_network_arp_depth.py', '--self-test', '--output',
                       'reviews/v5/expansion/legacy_network_arp/support_validation.json']),
        ('arp-close', ['tools/close_network_arp_depth.py']),
        ('arp-package', ['tools/check_network_arp_package.py', '--self-test', '--output',
                         'reviews/v5/expansion/legacy_network_arp/package_validation.json']),
        ('remaining-envelopes', ['tools/refresh_network_envelopes.py', '--base',
                                 'reviews/v5/expansion/network_composition_refresh_03', '--begin']),
        ('aggregate', ['tools/run_network_followthrough.py', '--reuse-current', '--report',
                       'reviews/v5/expansion/integrated/network_followthrough_campaign.json']),
        ('aggregate-guard', ['tools/check_network_followthrough.py', '--self-test', '--output',
                             'reviews/v5/expansion/followthrough/network_terminology_guard.json']),
    ]
    if options.resume:
        commands = commands[4:]
        commands[0][1][-1] = '--resume'
    for label, command_args in commands:
        if any(row['label'] == label and row['exit_code'] == 0 for row in rows):
            continue
        downstream = ROOT/'reviews/v5/expansion/network_composition_refresh_03/result.json'
        retained = json.loads(downstream.read_text()) if downstream.is_file() else {}
        if options.resume and label == 'remaining-envelopes' and retained.get('passed') is True:
            # A parent log-write defect occurred after this child completed.
            # Reuse its exact source/log-bound result, not an invented rerun.
            assert {r['name'] for r in retained['packages']} == {'congestion','ip','tcp','http'}
            for name, digest in retained['source_sha256'].items():
                assert sha(ROOT/name) == digest
            for package in retained['packages']:
                for name, digest in package['support_sha256'].items():
                    assert sha(ROOT/name) == digest
                for execution in package['executions']:
                    assert execution['exit_code'] == 0
                    assert sha(ROOT/execution['log']) == execution['log_sha256']
            log = BASE/'remaining-envelopes-verified-retained.log'
            log.write_text(json.dumps(dict(result=downstream.relative_to(ROOT).as_posix(),
                sha256=sha(downstream), mode='Exact retained child completion after parent log-write failure; no new export execution claimed.'), indent=2)+'\n', encoding='utf8')
            rows.append(dict(label=label, command=[sys.executable,*command_args], exit_code=0,
                mode='verified retained child report; not reexecuted',
                log=log.relative_to(ROOT).as_posix(), log_sha256=sha(log)))
        else:
            rows.append(run(label,command_args))
        dump(BASE/'execution.json', dict(passed=all(r['exit_code']==0 for r in rows), executions=rows))
        if rows[-1]['exit_code']:
            return 1
    packets = ['research/v5/expansion/legacy_network_'+n+'/completion_dispositions.json'
               for n in ['arp','ip','tcp','congestion','http']]
    packets.append('research/v5/expansion/legacy_network_ip/exercise_completion_dispositions.json')
    dump(BASE/'result.json', dict(passed=True, executed_at=datetime.now(timezone.utc).isoformat(),
        executions=rows, support_sha256={p:sha(ROOT/p) for p in packets},
        updates=sum(len(json.loads((ROOT/p).read_text())['updates']) for p in packets),
        source_sha256={'tools/refresh_network_terminology.py':sha(__import__('pathlib').Path(__file__))},
        limitations='Actual declared source-current author campaigns and selected exports; no physical or independent learner observation.'))
    return 0


if __name__=='__main__':
    raise SystemExit(main())
