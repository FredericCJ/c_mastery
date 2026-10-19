"""Source-bound equal-contract timer counters, counter-free timing and code evidence."""
import argparse
from datetime import datetime,timezone
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile
import time

ROOT=Path(__file__).resolve().parents[1]
INPUTS=['tools/run_resilience_timer_cost.py','tools/verify_resilience_timers.py',
        'labs/resilience/followthrough/timers.h','labs/resilience/followthrough/timers_test.c',
        'labs/resilience/followthrough/TIMERS.md','assessor/resilience/followthrough/timers.c',
        'labs/resilience/include/resilience.h','labs/messaging/include/messaging.h',
        'labs/execution/include/cmx.h','assessor/execution/cmx.c',
        'research/v5/modern/resilience_timer_decision.json','research/v5/modern/resilience_timer_decision_followup.json',
        'curriculum/projects/PR-T50-05.md']

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',default='research/v5/modern/resilience_timer_cost_validation.json')
    args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_resilience_timer_cost.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode
    before={p:sha(ROOT/p) for p in INPUTS};stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('resilience-timer-cost-'+stamp);logs.mkdir(parents=True)
    executions=[];profiles=[];passed=True
    def record(text,label):
        path=logs/(f'{len(executions):04d}-{label}.log');path.write_text(text,encoding='utf8')
        return {'log':path.relative_to(ROOT).as_posix(),'log_sha256':sha(path)}
    def execute(command,label,cwd=ROOT):
        start=time.monotonic();result=subprocess.run(list(map(str,command)),cwd=cwd,text=True,capture_output=True,timeout=180)
        row={'command':list(map(str,command)),'cwd':str(cwd),'exit_code':result.returncode,'seconds':time.monotonic()-start,
             **record(result.stdout+'\n'+result.stderr,label)}
        executions.append(row)
        if result.returncode:raise RuntimeError(f'{label} failed: '+result.stderr)
        return result.stdout,row
    try:
        execute(['uname','-a'],'environment');execute(['getconf','LONG_BIT'],'data-model')
        with tempfile.TemporaryDirectory(prefix='cmastery-resilience-cost-') as temporary:
            for compiler in ['gcc','clang']:
                execute([compiler,'--version'],compiler+'-version')
                for optimization in ['0','3']:
                    label=compiler+'-O'+optimization;work=Path(temporary)/label;work.mkdir()
                    flags=['-std=c17','-D_POSIX_C_SOURCE=200809L','-O'+optimization,'-g','-fstack-usage',
                           '-Wall','-Wextra','-Wpedantic','-Werror','-pthread']
                    for directory in ['labs/resilience/include','labs/resilience/followthrough','labs/messaging/include','labs/execution/include']:
                        flags+=['-I'+str(ROOT/directory)]
                    objects=[]
                    for index,source in enumerate(['labs/resilience/followthrough/timers_test.c','assessor/resilience/followthrough/timers.c','assessor/execution/cmx.c']):
                        obj=work/f'{index}.o';objects.append(obj)
                        execute([compiler,*flags,'-DRZT_NO_WORK','-c',ROOT/source,'-o',obj],label+'-compile-'+str(index),work)
                    executable=work/'timers';counter=work/'timers-counters'
                    execute([compiler,*objects,'-pthread','-Wl,-Map,'+str(work/'timers.map'),'-o',executable],label+'-link',work)
                    execute([compiler,*flags,'-c',ROOT/'assessor/resilience/followthrough/timers.c','-o',work/'count.o'],label+'-counter-compile',work)
                    execute([compiler,objects[0],work/'count.o',objects[2],'-pthread','-o',counter],label+'-counter-link',work)
                    execute([sys.executable,ROOT/'tools/verify_resilience_timers.py','--driver',executable],label+'-oracle')
                    stack_text='\n'.join(p.name+'\n'+p.read_text(encoding='utf8') for p in sorted(work.glob('*.su')))
                    stack=record(stack_text,label+'-stack');executions.append({'kind':'captured-static-stack','exit_code':0,**stack})
                    map_record=record((work/'timers.map').read_text(encoding='utf8'),label+'-link-map');executions.append({'kind':'captured-map','exit_code':0,**map_record})
                    _,size_record=execute(['size',executable],label+'-size')
                    _,assembly_record=execute(['objdump','-drwC',objects[1]],label+'-assembly')
                    _,undefined_record=execute(['nm','-u',objects[1]],label+'-undefined')
                    modes=[]
                    for workload in range(5):
                        per_round=sum((63 if workload==1 else 7 if workload==4 else (i*17)%63+1)+2*i for i in range(16)
                                      if workload and (workload!=3 or i%2))
                        for kind in range(3):
                            raw,_=execute([counter,'benchmark',kind,workload,1],label+f'-work-{workload}-{kind}')
                            counters=json.loads(raw);assert counters['checksum']==per_round
                            durations=[];raw_samples=[]
                            for repetition in range(8):
                                raw,row=execute([executable,'benchmark',kind,workload,3000],label+f'-timing-{workload}-{kind}-{repetition}')
                                observation=json.loads(raw);assert observation['checksum']==per_round*3000
                                assert observation['comparisons']==observation['ticks']==observation['link_writes']==observation['cascade_visits']==0
                                if repetition:durations.append(observation['nanoseconds']);raw_samples.append(row['log'])
                            raw,latency_record=execute([executable,'latency',kind,workload,128],label+f'-latency-{workload}-{kind}')
                            lines=raw.splitlines();summary=json.loads(lines[-1]);assert summary['checksum']==per_round*128
                            samples=[list(map(int,line.split(','))) for line in lines[:-1]]
                            assert len(samples)==128*64
                            all_latency=sorted(row[-1] for row in samples)
                            burst=[row[-1] for row in samples if row[-2]]
                            modes.append({'kind':kind,'workload':workload,'counters':counters,'warmups':1,'repetitions':7,
                                          'nanoseconds':durations,'median_nanoseconds':statistics.median(durations),'raw_timing_logs':raw_samples,
                                          'latency':{'raw_log':latency_record['log'],'raw_log_sha256':latency_record['log_sha256'],
                                                     'samples':len(samples),'median_ns':statistics.median(all_latency),
                                                     'p95_ns':all_latency[(len(all_latency)*95)//100],
                                                     'max_ns':max(all_latency),'max_nonempty_expiry_ns':max(burst) if burst else None}})
                    profiles.append({'configuration':label,'modes':modes,'sizeof_runtime_selected_object':modes[0]['counters']['sizeof'],
                                     'assembly':assembly_record,'static_stack':stack,'size':size_record,'undefined_symbols':undefined_record,'map':map_record})
                    print(label+': passed equal-contract counters, 105 measured timings and 122,880 per-advance observations',flush=True)
    except (AssertionError,RuntimeError,subprocess.TimeoutExpired) as error:
        passed=False;executions.append({'kind':'failure','error':repr(error),'exit_code':1});print(repr(error),flush=True)
    stable=before=={p:sha(ROOT/p) for p in INPUTS};passed=passed and stable and len(profiles)==4
    data={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'source_stable':stable,
          'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':before,'profiles':profiles,'executions':executions,
          'physical_validation':'not-run','learner_mastery':'not-assessed',
          'limitations':['N16 runtime-selected object has identical physical size across alternatives.',
                         'Counter-free timing is unsanitized; the separate diagnostic build reports only the stated logical maintenance counts.',
                         'Per-advance observations include clock-read overhead and prior output can perturb caches; sample maxima are not hard WCET or physical interrupt jitter.',
                         'Host load/affinity/clock variability are not controlled. No general performance winner follows from these finite workloads.',
                         'CMX is imported only for a bounded due/cancel outcome intersection, not the stronger stable event/identity contract.']}
    (ROOT/args.report).write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
