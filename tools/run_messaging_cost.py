"""Measure explicit equal-contract workloads; never infer mastery or fairness."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
SOURCE=[
    'labs/messaging/followthrough/cost_driver.c','labs/messaging/followthrough/cost_reference.c',
    'assessor/messaging/followthrough/registration.c','assessor/messaging/followthrough/weak.c',
    'assessor/messaging/followthrough/stream.c','assessor/messaging/followthrough/owned.c',
    'assessor/messaging/followthrough/operation.c','assessor/messaging/followthrough/supervisor.c',
    'assessor/messaging/messaging.c']


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def source_pins():
    paths=[]
    for base in ('labs/messaging','assessor/messaging'):
        paths += [p for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    paths += [ROOT/'tools/run_messaging_cost.py',ROOT/'tools/verify_messaging_registration.py']
    paths += [ROOT/'research/v5/modern'/p for p in ('messaging_cost_resources.json','messaging_cost_sources.md','messaging_cost_research_gate.json')]
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)}


def summarize(values):
    ordered=sorted(values)
    if not ordered:return dict(count=0)
    return dict(count=len(values),minimum=min(values),median=statistics.median(values),
                p95=ordered[(95*(len(ordered)-1))//100],p99=ordered[(99*(len(ordered)-1))//100],maximum=max(values))


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',default='research/v5/modern/messaging_cost_validation.json')
    parser.add_argument('--repetitions',type=int,default=7);parser.add_argument('--operations',type=int,default=1000)
    args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_messaging_cost.py','--report',args.report,'--repetitions',str(args.repetitions),'--operations',str(args.operations),platform='wsl'),cwd=ROOT).returncode
    assert args.repetitions>=3 and 128<=args.operations<=100000
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/f'{stamp}-messaging-cost';logs.mkdir(parents=True)
    build=ROOT/'build/modern_completion/messaging-cost'/stamp;build.mkdir(parents=True)
    before=source_pins();records=[];observations=[];artifacts=[];failure=None
    specification=importlib.util.spec_from_file_location('messaging_cost_check',ROOT/'labs/messaging/followthrough/cost_check.py')
    checks=importlib.util.module_from_spec(specification);specification.loader.exec_module(checks)
    def execute(command,label,cwd=ROOT):
        start=time.monotonic();result=subprocess.run([str(x) for x in command],cwd=cwd,capture_output=True,text=True,timeout=90)
        path=logs/(f'{len(records):04d}-{label}.log');path.write_text(result.stdout+'\n'+result.stderr,encoding='utf8')
        records.append(dict(command=[str(x) for x in command],cwd=str(cwd),exit_code=result.returncode,
                            seconds=time.monotonic()-start,log=path.relative_to(ROOT).as_posix(),log_sha256=sha(path)))
        if result.returncode:raise AssertionError((label,result.returncode,result.stdout[-2000:],result.stderr[-2000:]))
        return result.stdout
    try:
        execute(['uname','-a'],'host');execute(['lscpu'],'cpu')
        for compiler in ('gcc','clang'):
            executable=shutil.which(compiler);assert executable
            execute([executable,'--version'],compiler+'-version')
            for optimization in ('-O0','-O3'):
                label=compiler+optimization[1:];folder=build/label;folder.mkdir()
                flags=['-std=c17',optimization,'-g','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-fstack-usage',
                       '-I'+str(ROOT/'labs/messaging/include'),'-I'+str(ROOT/'labs/messaging/followthrough')]
                objects=[]
                for index,source in enumerate(SOURCE):
                    obj=folder/f'{index}-{Path(source).stem}.o';objects.append(obj)
                    execute([executable,*flags,'-c',ROOT/source,'-o',obj],label+'-compile-'+str(index),folder)
                driver=folder/'measure';link_map=folder/'link.map'
                execute([executable,*flags,*objects,'-Wl,-Map,'+str(link_map),'-o',driver],label+'-link',folder)
                execute(['size',driver],label+'-size')
                execute(['nm','-S','--size-sort',driver],label+'-symbols')
                execute(['objdump','-dr',driver],label+'-disassembly')
                for path in sorted(folder.glob('*.su'))+[link_map]:
                    destination=logs/(label+'-'+path.name);shutil.copyfile(path,destination)
                    artifacts.append(dict(profile=label,path=destination.relative_to(ROOT).as_posix(),sha256=sha(destination),kind='stack-usage' if path.suffix=='.su' else 'link-map'))
                for mode in checks.MODES:
                    repetitions=1 if mode in ('representation','fairness') else args.repetitions+1
                    for repetition in range(repetitions):
                        raw=execute([driver,mode,args.operations],label+'-'+mode+'-'+str(repetition))
                        row=json.loads(raw);assert row['mode']==mode;checks.validate(row,args.operations)
                        row.update(profile=label,repetition=repetition,warmup=repetition==0 and repetitions>1)
                        if 'threads' in row:
                            row['thread_latency_summary_ns']=[summarize(t['latency_ns']) for t in row['threads']]
                        observations.append(row)
                print('messaging cost '+label+': actual equal-work checks and measurements passed',flush=True)
    except (AssertionError,OSError,subprocess.TimeoutExpired,ValueError) as error:
        failure=repr(error)
    stable=before==source_pins();passed=failure is None and stable
    report=dict(schema_version=1,passed=passed,status='passed' if passed else 'failed',producer='tools/run_messaging_cost.py',
                recorded_at=datetime.now(timezone.utc).isoformat(),inspected_paths=before,source_stable=stable,
                input_sha256=hashlib.sha256(json.dumps(before,sort_keys=True).encode()).hexdigest(),
                workload=dict(operations=args.operations,repetitions=args.repetitions,process_warmup_runs=1,
                              timing_clock='CLOCK_MONOTONIC',affinity_observed=sorted(os.sched_getaffinity(0)),
                              affinity_changed=False,sanitizers=False,logging_inside_timed_region=False,
                              platform=platform.platform(),machine=platform.machine()),
                executions=records,artifacts=artifacts,observations=observations,error=failure,
                limitations=['Actual WSL/Linux host observations; no physical MCU, weak ISA, universal ranking or worst-case latency claim.',
                             'Raw per-operation samples include clock overhead; observer mode measures that overhead separately, without unsafe blanket subtraction.',
                             'Warmup is a separate process. Scheduling, host load, frequency and caches are not controlled; repetitions and ranges expose variability, not a confidence proof.',
                             'The same observable value/atomicity contract is compared only on each specified workload subset; blocking and progress guarantees remain different.',
                             'Fairness is a controlled finite atomic interleaving plus separate argument, not a proof inferred from measured throughput.',
                             'Compiler stack-usage output is per function; dynamic pthread stacks, allocator internals and whole-program recursive bounds are separate.'],
                learner_mastery='not-assessed',physical_validation='not-run')
    path=ROOT/args.report;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    if failure:print(failure)
    return 0 if passed else 1


if __name__=='__main__':raise SystemExit(main())
