"""Measure bounded pool/FIFO composition; allocator backing is an explicit hosted variant."""
import argparse,hashlib,json,platform,statistics,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def run(report):
    destination=(ROOT/report).resolve();base=ROOT/'reviews/v5/expansion/legacy_completion'
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use completion evidence')
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'memory_runs'/stamp;logs.mkdir(parents=True)
    sources=[Path(__file__).resolve(),ROOT/'labs/checkpoint_depth/tests/memory_experiment.c',ROOT/'labs/checkpoint_depth/include/checkpoint_memory.h',ROOT/'assessor/checkpoint_depth/checkpoint_memory.c']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'environment':platform.uname()._asdict(),
        'scope':'Actual local host elapsed samples of2000 initialize/admit/drain cycles; five warmups,31 repetitions, correctness checks inside every mode. malloc/free backing allocation is outside timed region. No whole-process fragmentation, board timing, universal speedup or mastery claim.',
        'learner_mastery':'not-assessed','independent_review':'not-performed'}
    try:
        with tempfile.TemporaryDirectory(prefix='checkpoint-memory-') as temporary:
            for compiler,opt in [('gcc','-O2'),('gcc','-O3'),('clang','-O2')]:
                name=compiler+opt;exe=Path(temporary)/name;argv=[compiler,'-std=c17',opt,'-Wall','-Wextra','-Werror','-I'+str(sources[2].parent),str(sources[1]),str(sources[3]),'-o',str(exe)]
                built=subprocess.run(argv,capture_output=True,text=True,timeout=30)
                if built.returncode:raise RuntimeError(built.stderr)
                observed=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
                (logs/(name+'.jsonl')).write_text(observed.stdout,encoding='utf-8',newline='\n')
                if observed.returncode:raise RuntimeError(observed.stderr)
                rows=[json.loads(line) for line in observed.stdout.splitlines()];samples=rows[1:];groups=[]
                if len(samples)!=248:raise ValueError('Missing sample')
                for heap in (0,1):
                    for failure in (0,3):
                        for instrument in (0,1):
                            selected=[x for x in samples if (x['heap_backing'],x['failure_ordinal'],x['instrumented'])==(heap,failure,instrument)]
                            ns=sorted(x['ns'] for x in selected)
                            if len(ns)!=31 or ns[0]<=0 or any(x['peak_live']!=(2 if failure else 4) for x in selected if instrument):raise ValueError('Invalid measurement/instrumentation')
                            groups.append({'heap_backing':heap,'failure_ordinal':failure,'instrumented':instrument,'count':len(ns),'median_ns':statistics.median(ns),'p10_ns':ns[3],'p90_ns':ns[27],'observed_max_ns':ns[-1]})
                result['profiles'].append({'name':name,'command':argv,'version':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'layout':rows[0],'distributions':groups,'raw_samples':len(samples),'passed':True})
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
    result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==3
    result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    payload=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(payload,encoding='utf-8',newline='\n');destination.write_text(payload,encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'samples':sum(x['raw_samples'] for x in result['profiles']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(run(p.parse_args().report))
