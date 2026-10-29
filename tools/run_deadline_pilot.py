"""Source-bound finite, native and measured deadline-collection author campaign."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import statistics
import subprocess

ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/hosted_systems/timer_depth'
REF=ROOT/'assessor/hosted_systems/timer_depth'


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
    inputs=[Path(__file__),*LAB.glob('collection*'),LAB/'timer.h',LAB/'CONTRACTS.md',REF/'collection.c',REF/'timer.c']
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
                               ('asan-ubsan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
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
                run(binary.name+'-build',[cc,*common,'-O3','-g',LAB/'collection_bench.c',REF/'collection.c',REF/'timer.c','-o',binary])
                symbols=run(binary.name+'-symbols',['nm','-a',binary])
                if '__asan_init' in symbols or '__tsan_init' in symbols:
                    raise RuntimeError('Instrumented performance binary')
                run(binary.name+'-disassembly',['objdump','-d',binary])
                rows=[json.loads(line) for line in run(binary.name,[binary]).splitlines()]
                if len(rows)!=837 or not all(row['valid'] for row in rows):
                    raise RuntimeError('Missing or incorrect benchmark trace')
                groups={}
                for row in rows:
                    groups.setdefault((row['n'],row['pattern'],row['trial']),{})[row['kind']]=row
                if len(groups)!=279 or not all(set(g)=={0,1,2} and len({r['checksum'] for r in g.values()})==1 for g in groups.values()):
                    raise RuntimeError('Unmatched workloads')
                summaries=[]
                for n in (16,256,4096):
                    for pattern in range(3):
                        paired=[groups[n,pattern,t] for t in range(31)]
                        summaries.append(dict(n=n,pattern=pattern,
                            median_ns={str(k):statistics.median(g[k]['elapsed_ns'] for g in paired) for k in range(3)},
                            median_difference_from_scan_ns={str(k):statistics.median(g[k]['elapsed_ns']-g[0]['elapsed_ns'] for g in paired) for k in (1,2)}))
                observations[cc+'-bench']=dict(raw=rows,summaries=summaries,binary_sha256=digest(binary),
                    boundary='256 mutations and minimum queries; setup/free and independent oracle excluded; checksum fold included',
                    claims='Measured elapsed time on this host only; no universal ranking or WCET')
        source=(REF/'collection.c').read_text()
        mutations={
            'stale-identity':('if (q->timers[token.slot].generation != token.generation) return 0;','/* wrong: ignores generation */'),
            'inverse-position':('q->position[q->index[b]] = b;','/* wrong: stale inverse position */'),
            'periodic-reinsert':('if (result == 1 && q->timers[token.slot].state == TD_ARMED) insert(q, token.slot);','/* wrong: lost periodic registration */')}
        for name,pair in mutations.items():
            old,new=pair
            if source.count(old)!=1: raise RuntimeError('Mutation anchor changed: '+name)
            mutant=work/(name+'.c'); mutant.write_text(source.replace(old,new))
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
    value=dict(schema_version=1,recorded_at=datetime.now(timezone.utc).isoformat(),passed=error is None,
        error=error,source_sha256=hashes,commands=commands,observations=observations,
        reference_provider='Explicit separate assessor collection and retained TdTimer implementation',
        gates=dict(author_execution='passed' if error is None else 'failed',hardware_pmu='separate PMU campaign',
                   independent_learner='not-assessed',external_review='not-run'))
    report.write_text(json.dumps(value,indent=2)+'\n')
    print(json.dumps(dict(passed=value['passed'],error=error,commands=len(commands),report=str(report))))
    return 0 if error is None else 1


if __name__=='__main__':
    raise SystemExit(main())
