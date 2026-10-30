"""Source-bound finite, native and measured deadline-collection author campaign.

The benchmark's paired summaries are judged by tools/measurement_gates.py: each
heap-versus-scan and wheel-versus-scan comparison gets acquisition (timer only:
not applicable), timing, comparability and interpretation gates. The bench runs
a fixed 256 operations per trial, so a regime whose shortest trial is below the
timing target (computed from a clock probe built with the same compiler) is
`inadequate` and keeps its raw rows without a ranking.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess

from c_source import replace_unique
import measurement_gates as gates

ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/hosted_systems/timer_depth'
REF=ROOT/'assessor/hosted_systems/timer_depth'
SUPPORT=ROOT/'labs/support'
KINDS={0:'scan',1:'heap',2:'wheel'}
TRIALS=gates.REQUIRED_PAIRS  # collection_bench.c runs this many trials per regime


def load_snapshot():
    """Host load at one moment: the bench shares the machine, so its receipt
    records the load before and after the measured runs."""
    meminfo=Path('/proc/meminfo').read_text()
    available=next((line.split()[1] for line in meminfo.splitlines() if line.startswith('MemAvailable:')),None)
    return dict(utc=datetime.now(timezone.utc).isoformat(),loadavg=Path('/proc/loadavg').read_text().strip(),
                mem_available_kb=int(available) if available else None)


def clock_target(probes):
    """Timing target from the clock probe runs bracketing the bench: the same
    robust clock-cost statistic as the PMU collector rows."""
    return gates.timing_target_ns(resolution_ns=max(p['resolution_ns'] for p in probes),
                                  brackets_ns=[b for p in probes for b in p['brackets_ns']])


def gated_summaries(groups,target):
    """Per (n, pattern): heap and wheel each compared with scan, trial by trial.
    One rule with the PMU path: every row must reach its own target (here the
    probe target) at the calibrated repetitions. The workload is fixed (no
    repetition control), so an inadequate regime is `inadequate`, never
    `exhausted`; raw rows stay in the observations."""
    summaries=[]
    for n in (16,256,4096):
        for pattern in range(3):
            trials=[groups[n,pattern,t] for t in range(TRIALS)]
            comparisons={}
            for kind in (1,2):
                rows=[dict(pair=t,variant=v,elapsed_ns=g[k]['elapsed_ns'],checksum=g[k]['checksum'],repetitions=1,
                           target_ns=target,valid=g[k]['valid'])
                      for t,g in enumerate(trials) for v,k in ((0,0),(1,kind))]
                calibration=gates.calibration_state(
                    [dict(repetitions=1,min_elapsed_ns=min(r['elapsed_ns'] for r in rows),target_ns=target)],
                    max_repetitions=None)
                comparisons[KINDS[kind]+'-vs-scan']=gates.evaluate_regime(rows,calibration,required_pairs=gates.REQUIRED_PAIRS,pmu=False)
            summaries.append(dict(n=n,pattern=pattern,timing_target_ns=target,comparisons=comparisons))
    return summaries


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',type=Path,default=ROOT/'reviews/v5/expansion/native_revision/deadlines.json')
    parser.add_argument('--no-measure',action='store_true')
    args=parser.parse_args()
    report=args.report.resolve()
    if report.exists():
        parser.error('Use a new receipt path; existing evidence is immutable')
    work=report.with_suffix('')
    work.mkdir(parents=True,exist_ok=False)
    inputs=[Path(__file__),*LAB.glob('collection*'),LAB/'timer.h',LAB/'CONTRACTS.md',REF/'collection.c',REF/'timer.c',
            ROOT/'tools/measurement_gates.py',SUPPORT/'measurement_gates.py',SUPPORT/'cm_clock.h',
            SUPPORT/'cm_clock.c',SUPPORT/'cm_clock_probe.c']
    def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
    hashes={str(p.relative_to(ROOT)):digest(p) for p in inputs}
    commands=[]
    observations={}
    def run(label,argv,expected=0):
        command=list(map(str,argv))
        try:
            result=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=120)
            log=work/(label+'.txt')
            log.write_text(result.stdout+result.stderr)
            commands.append(dict(label=label,command=command,exit_code=result.returncode,
                                 expected_exit_code=expected,log=str(log),sha256=digest(log)))
        except subprocess.TimeoutExpired as exc:
            commands.append(dict(label=label,command=command,outcome='timeout'))
            raise RuntimeError(label+': timeout') from exc
        if result.returncode!=expected:
            raise RuntimeError(f'{label}: expected {expected}, got {result.returncode}')
        return result.stdout+result.stderr
    error=None
    try:
        common=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(LAB)]
        for cc in ('gcc','clang'):
            run(cc+'-version',[cc,'--version'])
            for name,flags in [('debug',['-O0','-g']),('optimized',['-O2','-DNDEBUG']),
                               ('asan-ubsan',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:
                binary=work/(cc+'-'+name)
                run(binary.name+'-build',[cc,*common,*flags,'-DTC_COUNT_WORK',LAB/'collection_tests.c',REF/'collection.c',REF/'timer.c','-o',binary])
                if 'COLLECTION_PASS:' not in run(binary.name,[binary]):
                    raise RuntimeError('Missing complete finite test marker')
            native=work/(cc+'-native')
            run(native.name+'-build',[cc,*common,'-O2',LAB/'collection_native.c',REF/'collection.c',REF/'timer.c','-o',native])
            observations[native.name]=[json.loads(line) for line in run(native.name,[native]).splitlines()]
            if len(observations[native.name])!=3:
                raise RuntimeError('Missing native representation')
            if not args.no_measure:
                binary=work/(cc+'-bench')
                run(binary.name+'-build',[cc,*common,*gates.MEASUREMENT_CFLAGS,LAB/'collection_bench.c',REF/'collection.c',REF/'timer.c','-o',binary])
                symbols=run(binary.name+'-symbols',['nm','-a',binary])
                if '__asan_init' in symbols or '__tsan_init' in symbols:
                    raise RuntimeError('Instrumented performance binary')
                run(binary.name+'-disassembly',['objdump','-d',binary])
                probe=work/(cc+'-clock-probe')
                run(probe.name+'-build',[cc,*common,'-I'+str(SUPPORT),*gates.MEASUREMENT_CFLAGS,SUPPORT/'cm_clock_probe.c',SUPPORT/'cm_clock.c','-o',probe])
                load_before=load_snapshot()
                probes=[json.loads(run(probe.name+'-before',[probe,'monotonic']))]
                rows=[json.loads(line) for line in run(binary.name,[binary]).splitlines()]
                probes.append(json.loads(run(probe.name+'-after',[probe,'monotonic'])))
                if len(rows)!=837 or not all(row['valid'] for row in rows):
                    raise RuntimeError('Missing or incorrect benchmark trace')
                groups={}
                for row in rows:
                    groups.setdefault((row['n'],row['pattern'],row['trial']),{})[row['kind']]=row
                if len(groups)!=279 or not all(set(g)=={0,1,2} and len({r['checksum'] for r in g.values()})==1 for g in groups.values()):
                    raise RuntimeError('Unmatched workloads')
                target=clock_target(probes)
                summaries=gated_summaries(groups,target)
                observations[cc+'-bench']=dict(raw=rows,summaries=summaries,binary_sha256=digest(binary),
                    clock_probes=probes,timing_target_ns=target,load=dict(before=load_before,after=load_snapshot()),
                    boundary='256 mutations and minimum queries; setup/free and independent oracle excluded; checksum fold included',
                    claims='Measured elapsed time on this host only; rankings only for supported comparisons; no universal ranking or WCET')
        source=(REF/'collection.c').read_text()
        mutations={
            'stale-identity':('if (q->timers[token.slot].generation != token.generation) return 0;','/* wrong: ignores generation */'),
            'inverse-position':('q->position[q->index[b]] = b;','/* wrong: stale inverse position */'),
            'periodic-reinsert':('if (result == 1 && q->timers[token.slot].state == TD_ARMED) insert(q, token.slot);','/* wrong: lost periodic registration */')}
        for name,pair in mutations.items():
            old,new=pair
            mutant=work/(name+'.c'); mutant.write_text(replace_unique(source,old,new))
            binary=work/name
            run(name+'-build',['gcc',*common,'-O2',LAB/'collection_tests.c',mutant,REF/'timer.c','-o',binary])
            text=run(name,[binary],expected=1)
            if 'COLLECTION_CHECK:' not in text: raise RuntimeError('Wrong rejection: '+name)
        binary=work/'learner'
        run('learner-build',['gcc',*common,'-O2',LAB/'collection_tests.c',LAB/'collection.c',REF/'timer.c','-o',binary])
        if 'COLLECTION_CHECK:' not in run('learner',[binary],expected=1): raise RuntimeError('Wrong learner rejection')
        if hashes!={str(p.relative_to(ROOT)):digest(p) for p in inputs}: raise RuntimeError('Source changed during campaign')
    except (OSError,ValueError,RuntimeError) as exc:
        error=str(exc)
    interpretation={}
    for name,bench in observations.items():
        if name.endswith('-bench'):
            for summary in bench['summaries']:
                for comparison in summary['comparisons'].values():
                    state=comparison['gates']['interpretation']
                    interpretation[state]=interpretation.get(state,0)+1
    value=dict(schema_version=2,recorded_at=datetime.now(timezone.utc).isoformat(),passed=error is None,
        error=error,source_sha256=hashes,commands=commands,observations=observations,
        reference_provider='Explicit separate assessor collection and retained struct td_timer implementation',
        gates=dict(author_execution='passed' if error is None else 'failed',hardware_pmu='separate PMU campaign',
                   bench_comparisons=interpretation if interpretation else 'not-run',
                   independent_learner='not-assessed',external_review='not-run'))
    report.write_text(json.dumps(value,indent=2)+'\n')
    print(json.dumps(dict(passed=value['passed'],error=error,commands=len(commands),report=str(report))))
    return 0 if error is None else 1


if __name__=='__main__':
    raise SystemExit(main())
