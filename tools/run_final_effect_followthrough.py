"""Actual original-provider final-builder failure and publication probes."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from c_source import AnchorError, replace_unique  # noqa: E402
SOURCES = ['tools/run_final_effect_followthrough.py',
           'labs/modern_followthrough/final/probe.c',
           'labs/modern_followthrough/final/CONTRACTS.md',
           'assessor/modern_followthrough/final/oracle.py',
           'assessor/modern_followthrough/final/ANALYSIS.md',
           'labs/higher_order/include/higher_order.h',
           'labs/higher_order/CONTRACTS.md', 'labs/higher_order/src/final.c',
           'assessor/higher_order/final.c', 'curriculum/v5/exercises/X-T69-012.md']


def sha(path):
    return hashlib.sha256((ROOT / path).read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--report', required=True)
    parser.add_argument('--build', required=True)
    args = parser.parse_args()
    if os.name == 'nt':
        raise SystemExit('Run this Linux hosted campaign in WSL.')
    report = (ROOT / args.report).resolve()
    build = (ROOT / args.build).resolve()
    assert report.is_relative_to(ROOT / 'reviews/v5/expansion/modern_followthrough')
    assert build.is_relative_to(ROOT / 'build/modern-followthrough') and not build.exists()
    build.mkdir(parents=True)
    report.parent.mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location('final_oracle', ROOT / SOURCES[3])
    oracle = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(oracle)
    pins = {p: sha(p) for p in SOURCES}
    records, observations = [], []
    result = dict(passed=False, status='failed', inspected_paths=pins,
                  executions=records, observations=observations,
                  scope='Original C17 free-effect API, exact final-builder failure and publication boundaries; six native profiles and isolated public export.',
                  external_review='not-run', independent_learner='not-assessed',
                  physical_observation='not-run')

    def run(command, expected=0, marker=None):
        start = time.monotonic()
        proc = subprocess.run(list(map(str, command)), cwd=build, capture_output=True,
                              text=True, timeout=90)
        output = proc.stdout + proc.stderr
        log = build / f'{len(records):03}.log'
        log.write_text(output, encoding='utf8')
        records.append(dict(command=list(map(str, command)), cwd=str(build),
                            exit_code=proc.returncode, seconds=time.monotonic()-start,
                            log=log.relative_to(ROOT).as_posix(), log_sha256=sha(log)))
        if proc.returncode != expected or (marker and marker not in output):
            raise ValueError({'command': records[-1], 'output': output[-2000:]})
        return proc.stdout

    include = ROOT / 'labs/higher_order/include'
    probe = ROOT / 'labs/modern_followthrough/final/probe.c'
    provider = ROOT / 'assessor/higher_order/final.c'
    common = ['-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror']
    try:
        for cc in ('gcc', 'clang'):
            run([cc, '--version'])
            for label, flags in [('O0', ['-O0']), ('O2', ['-O2']),
                                 ('asan', ['-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])]:
                exe = build / f'{cc}-{label}'
                run([cc, *common, *flags, '-I'+str(include), probe, provider, '-o', exe])
                count = oracle.check(run([exe]))
                observations.append(dict(profile=f'{cc}-{label}', oracle_rows=count,
                                         binary=exe.relative_to(ROOT).as_posix(), sha256=sha(exe)))
            learner = build / f'{cc}-learner'
            run([cc, *common, '-O2', '-I'+str(include), probe, ROOT/'labs/higher_order/src/final.c', '-o', learner])
            run([learner], expected=1, marker='MFT_CHECK:')
            source = provider.read_text(encoding='utf8')
            changes = [
                ('eval-failure-publication', 'if(s==HO_OK)*out=r;return s;', '*out=r;return s;'),
                ('lambda-partial-publication', 'if(!s){struct hp_fun f=', 'if(1){struct hp_fun f='),
                ('application-partial-publication', 'if(!s)*out=r;return s;\n}\nHPOps', '*out=r;return s;\n}\nHPOps')]
            for name, old, new in changes:
                staged = build / f'{cc}-{name}.c'
                try:
                    staged.write_text(replace_unique(source, old, new), encoding='utf8')
                except AnchorError as error:
                    raise ValueError('Provider changed: '+name) from error
                exe = build / f'{cc}-{name}'
                run([cc, *common, '-O2', '-I'+str(include), probe, staged, '-o', exe])
                run([exe], expected=1, marker='MFT_CHECK:')
                observations.append(dict(profile=cc, semantic_control=name, rejected=True,
                                         source=staged.relative_to(ROOT).as_posix(), sha256=sha(staged)))
        export = build / 'export'
        public = export / 'public'
        private = export / 'assessor'
        public.mkdir(parents=True); private.mkdir()
        for source, target in [('labs/higher_order/include/higher_order.h','higher_order.h'),
                               ('labs/higher_order/src/final.c','final.c'),
                               ('labs/modern_followthrough/final/probe.c','probe.c'),
                               ('labs/modern_followthrough/final/CONTRACTS.md','CONTRACTS.md'),
                               ('labs/higher_order/CONTRACTS.md','PROVIDER_CONTRACTS.md')]:
            shutil.copyfile(ROOT/source, public/target)
        shutil.copyfile(provider, private/'final.c')
        if set(p.name for p in public.iterdir()) != {'higher_order.h','final.c','probe.c','CONTRACTS.md','PROVIDER_CONTRACTS.md'}:
            raise ValueError('Unexpected public export member')
        for cc in ('gcc', 'clang'):
            for kind, implementation in [('learner', public/'final.c'), ('assessor', private/'final.c')]:
                exe = export / f'{cc}-{kind}'
                run([cc, *common, '-O2', '-I'+str(public), public/'probe.c', implementation, '-o', exe])
                output = run([exe], expected=1 if kind == 'learner' else 0,
                             marker='MFT_CHECK:' if kind == 'learner' else None)
                if kind == 'assessor': oracle.check(output)
        result['export_files'] = {p.relative_to(ROOT).as_posix(): sha(p) for p in public.iterdir()}
        result['scope_source_stable'] = pins == {p: sha(p) for p in SOURCES}
        assert result['scope_source_stable']
        result.update(passed=True, status='passed', profiles=6, oracle_rows_per_profile=4,
                      semantic_controls=6, learner_rejections=4, exports=4)
    except Exception as error:
        result['error'] = str(error)
    report.write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': result['passed'], 'commands': len(records), 'report': args.report, 'error': result.get('error')}))
    return 0 if result['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
