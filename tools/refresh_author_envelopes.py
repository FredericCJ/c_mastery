"""Refresh owned target-package metadata through actual exports and existing guards.

Original mechanism campaigns are preserved byte-for-byte. This does not update
the global completion ledger, run a source campaign, or relax a package guard.
"""
import argparse
import datetime
import json
import os
import subprocess
import sys
from pathlib import Path
from common import ROOT, sha, dump

BASE = 'reviews/v5/expansion/author_envelope_refresh_01'
NAMES = ['adc', 'bringup', 'diagnostic', 'dma', 'frame', 'g474_uart',
         'i2c', 'layers', 'mmio', 'spi', 'timer', 'uart']


def prepare():
    manifest = ROOT / BASE / 'manifest.json'
    assert not manifest.exists(), 'Do not overwrite the prepared history'
    initial = json.loads((ROOT / 'reviews/v5/expansion/envelope_refresh_01/provider_refresh_list.json').read_text())
    packages, history = [], {}
    for name in NAMES:
        folder = 'legacy_advanced_' + name
        item = next(x for x in initial['envelopes'] if Path(x['envelope']).parent.name == folder)
        support = item['envelope']; value = json.loads((ROOT/support).read_text())
        candidates = item['producer_candidates']
        exporter = next(p for p in candidates if p.endswith('_exports.py'))
        closer = next(p for p in candidates if Path(p).name.startswith('close_'))
        package_guard = next(p for p in candidates if p.endswith('_package.py'))
        guard = next(p for p in candidates if Path(p).name.startswith('check_') and p not in [exporter, package_guard])
        campaign = next(p for p in value['current_reports'] if p.endswith('/current_validation.json'))
        review = Path(campaign).parent.as_posix()
        for path in {support, exporter, *value['current_reports'], review+'/package_validation.json'}:
            source = ROOT/path
            if not source.is_file(): continue
            dest = ROOT/BASE/'history'/path; dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(source.read_bytes()); history[path] = sha(source)
        text = (ROOT/exporter).read_text(encoding='utf-8')
        assert 'envelope_export_inputs' not in text, exporter
        if name == 'bringup':
            old = ';out=dict(passed=passed,errors=errors,commands=rows,'
            assert old in text
            text = text.replace(old, '\n from envelope_export_inputs import bind_project_exports\n pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)\n passed=not errors and all(r[\'passed\'] for r in rows);out=dict(passed=passed,errors=errors,commands=rows,', 1)
        else:
            lines = text.splitlines()
            index = next(i for i, line in enumerate(lines) if line.lstrip().startswith(('out=dict(passed=', 'd=dict(passed=')))
            indent = lines[index][:-len(lines[index].lstrip())]
            lines[index:index] = [indent+'from envelope_export_inputs import bind_project_exports',
                                  indent+'pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)']
            text = '\n'.join(lines)+'\n'
        compile(text, exporter, 'exec')
        (ROOT/exporter).write_text(text, encoding='utf-8', newline='\n')
        packages.append(dict(name=name, support=support, exporter=exporter, close=closer,
                             guard=guard, package_guard=package_guard, review=review,
                             campaign=campaign, campaign_sha256=sha(ROOT/campaign),
                             rows=len(value['updates'])))
    dump(manifest, dict(packages=packages, history_sha256=history,
         scope='Actual project-asset exports and existing mechanism/package guards; original C campaigns unchanged.'))
    print('PREPARED', len(packages))


def run():
    assert os.name != 'nt', 'Run target exports under WSL'
    manifest = json.loads((ROOT/BASE/'manifest.json').read_text())
    run_dir = ROOT/BASE/'runs'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run_dir.mkdir(parents=True)
    progress = ROOT/BASE/'progress.json'
    old = json.loads(progress.read_text()) if progress.exists() else {'completed': []}
    if progress.exists(): (run_dir/'previous-progress.json').write_bytes(progress.read_bytes())
    completed = old['completed']
    for package in manifest['packages']:
        assert sha(ROOT/package['campaign']) == package['campaign_sha256'], 'Original C campaign changed'
        prior = next((x for x in completed if x['support']==package['support']), None)
        if prior:
            assert sha(ROOT/prior['support']) == prior['sha256'], 'Completed support changed'
            continue
        steps = [[sys.executable, package['exporter']],
                 [sys.executable, package['guard'], '--self-test', '--output', package['review']+'/support_validation.json'],
                 [sys.executable, package['close']],
                 [sys.executable, package['package_guard'], '--self-test', '--output', package['review']+'/package_validation.json']]
        if package['name'] == 'bringup':
            steps[1] += ['--report', package['campaign'], '--images', package['review']+'/current_images.json']
        if package['name'] == 'g474_uart':
            steps[1][steps[1].index('--output')] = '--report'
            steps[3][steps[3].index('--output')] = '--report'
        executions = []
        for index, command in enumerate(steps):
            process = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=1200)
            log = run_dir/(package['name']+'-'+str(index)+'.log')
            log.write_text(process.stdout+'\n'+process.stderr, encoding='utf-8')
            executions.append(dict(command=command, exit_code=process.returncode,
                                   log=log.relative_to(ROOT).as_posix(), log_sha256=sha(log)))
            if process.returncode:
                dump(progress, dict(passed=False, completed=completed, failed=package, executions=executions))
                print(process.stdout+process.stderr, flush=True)
                return 1
        assert sha(ROOT/package['campaign']) == package['campaign_sha256'], 'Original campaign overwritten'
        completed.append(dict(support=package['support'], sha256=sha(ROOT/package['support']),
                              guard=package['review']+'/package_validation.json', rows=package['rows'],
                              campaign=package['campaign'], campaign_sha256=package['campaign_sha256'],
                              campaign_unchanged=True, executions=executions))
        dump(progress, dict(passed=True, completed=completed))
        print('REFRESH_PASS', package['name'], flush=True)
    dump(ROOT/BASE/'refreshed_packages.json', dict(passed=True, packages=completed,
         source_sha256={p:sha(ROOT/p) for p in ['tools/refresh_author_envelopes.py',
          'tools/envelope_export_inputs.py', BASE+'/manifest.json']},
         limits='Metadata and actual export refresh only; prior author C execution preserved; external dimensions unchanged.'))
    return 0


if __name__ == '__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--prepare', action='store_true');args=parser.parse_args()
    if args.prepare: prepare()
    else: raise SystemExit(run())
