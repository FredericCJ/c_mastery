"""Execute original six-stack copied-byte queue guests and discriminating task-port controls."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/concurrency_lifecycle_depth/cmx'
PRIVATE=ROOT/'assessor/concurrency_lifecycle_depth'
spec=importlib.util.spec_from_file_location('queue_task_public',PUBLIC/'task_run.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)

def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_queue_port';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use queue port evidence directory')
    logs=base/'task_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[PUBLIC/name for name in ('task_run.py','task_workload.c','task.ld','cl_cmx.h','cl_cmx_task.h','cl_cmx_task.c')],
      PRIVATE/'cl_cmx.c',PRIVATE/'cl_cmx_task.c',PRIVATE/'core.c',ROOT/'labs/concurrency_lifecycle_depth/include/cl.h',
      *[ROOT/'assessor/execution'/name for name in ('startup.S','context.S','cortex_port.c','board.c','support.c','cmx.c')],
      ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/include/cmx_port.h']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'outcomes':[],
      'scope':'Actual retained SVC/PendSV/SysTick runtime with six disjoint PSP stacks: copied messages, timeout/abort, condition timeout/reacquisition and close/reap/restart.',
      'limits':['Emulator observations only; no physical board, WCET, SMP, asynchronous task kill, peripheral DMA quiescence, or exhaustive execution claim.','The context image includes FP save/restore instructions; this experiment does not exercise floating-point payload registers.']}
    def run(command):
        command=[str(x) for x in command];result=subprocess.run(command,capture_output=True,text=True,timeout=60)
        output=result.stdout+result.stderr;log=logs/(str(len(value['commands'])).zfill(4)+'.log');log.write_text(output,encoding='utf-8',newline='\n')
        value['commands'].append({'command':command,'exit_code':result.returncode,'log':log.relative_to(ROOT).as_posix()})
        if result.returncode:raise ValueError('Build/audit command failed: '+output)
        return output
    try:
        value['toolchains']={name:run([name,'--version']).splitlines()[0] for name in ('arm-none-eabi-gcc','clang','qemu-system-arm')}
        with tempfile.TemporaryDirectory(prefix='cmx-queue-tasks-') as directory:
            for compiler,optimization in (('gcc','-O0'),('gcc','-O3'),('clang','-O3')):
                for scenario in range(3):
                    variants=('reference','task-stub')+ (('no-dispatch','masked-entry') if scenario==0 else ())
                    for variant in variants:
                        label=compiler+optimization+'-'+str(scenario)+'-'+variant
                        args=SimpleNamespace(build=Path(directory)/label,compiler=compiler,optimization=optimization,scenario=scenario,
                          mutant={'no-dispatch':1,'masked-entry':2}.get(variant,0),runtime=ROOT/'assessor/execution',
                          execution_source=ROOT/'assessor/execution/cmx.c',execution_include=ROOT/'labs/execution/include',
                          lifecycle_source=PRIVATE/'core.c',lifecycle_include=ROOT/'labs/concurrency_lifecycle_depth/include',
                          adapter_source=PRIVATE/'cl_cmx.c',task_source=(PUBLIC if variant=='task-stub' else PRIVATE)/'cl_cmx_task.c')
                        elf,audit=runner.image(args,run)
                        command=[str(x) for x in runner.qemu(elf)];result=subprocess.run(command,capture_output=True,text=True,timeout=30)
                        output=result.stdout+result.stderr;log=logs/(label+'.log');log.write_text(output,encoding='utf-8',newline='\n')
                        expected=0 if variant=='reference' else 1;marker='CC_TASK_PASS:' if variant=='reference' else 'CC_TASK_CHECK:'
                        passed=result.returncode==expected and marker in output
                        for suffix in ('map','txt'):
                            for source in args.build.glob('*.'+suffix):(logs/(label+'-'+source.name)).write_bytes(source.read_bytes())
                        value['outcomes'].append({'profile':compiler+optimization,'scenario':scenario,'variant':variant,'passed':passed,
                          'command':command,'exit_code':result.returncode,'log':log.relative_to(ROOT).as_posix(),'output':output,
                          'image_sha256':hashlib.sha256(elf.read_bytes()).hexdigest(),'audit':audit})
                        if not passed:raise ValueError(label+': '+output)
    except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']='error' not in value and value['source_stable'] and len(value['outcomes'])==24
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':len(value['outcomes']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
