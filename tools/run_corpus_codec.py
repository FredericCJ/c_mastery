"""Execute the original bounded codec, independent oracle and runtime audits."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import run_expansion_family as base
from common import ROOT, dump, sha

DEFAULT='reviews/v5/expansion/corpus_codec/current_validation.json'

def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_corpus_codec.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    extras=[Path(__file__),ROOT/'tools/check_corpus_codec.py']
    extra_pins={p.relative_to(ROOT).as_posix():sha(p) for p in extras}
    result=base.run('corpus_codec',report)
    path=ROOT/report;d=json.loads(path.read_text());d['inspected_paths'].update(extra_pins)
    run_dir=path.parent/'target-and-measurement';run_dir.mkdir(parents=True,exist_ok=True)
    outputs={};d['target_audits']=[];d['measurements']=[];d['independent_oracles']=[]
    def execute(args,name):
        args=[str(x) for x in args]
        p=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=180)
        log=run_dir/(name+'.log');log.write_text(p.stdout+'\n'+p.stderr)
        d['executions'].append(dict(command=args,cwd=str(ROOT),exit_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        if p.returncode:raise RuntimeError(name+': exit '+str(p.returncode))
        return p.stdout
    try:
        if result:raise RuntimeError('Hosted campaign failed; target claims withheld')
        for cc in ['arm-none-eabi-gcc','clang']:
            execute([cc,'--version'],cc+'.version')
            for opt in ['-O0','-O3']:
                tag=cc+opt;obj=run_dir/(tag+'.o');image=run_dir/(tag+'.elf');mapping=run_dir/(tag+'.map')
                flags=[*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17','-Wall','-Wextra','-Werror','-pedantic-errors',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I',ROOT/'labs/corpus_codec']
                execute([cc,*flags,'-c',ROOT/'assessor/corpus_codec/codec.c','-o',obj],tag+'.compile')
                undefined=execute(['arm-none-eabi-nm','-u',obj],tag+'.object-undefined')
                if undefined.strip():raise RuntimeError('Implicit object helpers: '+undefined)
                execute(['arm-none-eabi-ld','--entry=cc_compress','-Map='+str(mapping),obj,'-o',image],tag+'.link')
                undefined=execute(['arm-none-eabi-nm','-u',image],tag+'.image-undefined')
                if undefined.strip():raise RuntimeError('Implicit linked helpers: '+undefined)
                assembly=execute(['arm-none-eabi-objdump','-dr',image],tag+'.disassembly')
                if '<cc_expand>' not in assembly or '<cc_lzoptimal>' not in assembly:raise RuntimeError('Missing linked mechanisms')
                execute(['arm-none-eabi-size',image],tag+'.size')
                stack=obj.with_suffix('.su')
                if not stack.is_file() or 'cc_compress' not in stack.read_text():raise RuntimeError('Missing actual stack report')
                for p in [obj,image,mapping,stack]:outputs[p.relative_to(ROOT).as_posix()]=sha(p)
                d['target_audits'].append(dict(compiler=cc,optimization=opt,language='c17',undefined_symbols=[],libraries=[],execution='not-run',stack_report=stack.relative_to(ROOT).as_posix()))
        with tempfile.TemporaryDirectory(prefix='codec-measure-') as tmp:
            for cc in ['gcc','clang']:
                binary=Path(tmp)/cc
                execute([cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic-errors','-I',ROOT/'labs/corpus_codec',ROOT/'labs/corpus_codec/measure.c',ROOT/'assessor/corpus_codec/codec.c','-o',binary],cc+'.measure-build')
                raw=execute([binary],cc+'.measure')
                if len(raw.splitlines())!=8 or 'optimal-token' not in raw:raise RuntimeError('Vacuous measurement')
                d['measurements'].append(dict(compiler=cc,workloads=4,algorithms=2,trials_per_pair=1000,raw=raw,limits='Hosted process struct lr_cpu ticks include function calls and bounded scratch; no board latency, distribution inference or universal speed claim.'))
                driver=Path(tmp)/(cc+'-driver')
                execute([cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic-errors','-I',ROOT/'labs/corpus_codec',ROOT/'labs/corpus_codec/driver.c',ROOT/'assessor/corpus_codec/codec.c','-o',driver],cc+'.oracle-build')
                oracle=json.loads(execute(['python3',ROOT/'assessor/corpus_codec/oracle.py',driver],cc+'.independent-oracle'))
                if not oracle['passed'] or oracle['cases']<10000 or not oracle['greedy_counterexample']:raise RuntimeError('Vacuous oracle')
                d['independent_oracles'].append(dict(compiler=cc,**oracle))
        for p,h in d['inspected_paths'].items():
            if sha(ROOT/p)!=h:raise RuntimeError('Input changed during run: '+p)
    except Exception as e:
        d['passed']=False;d['failure']=str(e)
    d['scope_input_sha256']=hashlib.sha256(json.dumps(d['inspected_paths'],sort_keys=True).encode()).hexdigest()
    d['generated_artifact_sha256']=outputs;d['status']='passed' if d['passed'] else 'failed'
    d['scope']='Six host profiles with independent prefix/Kraft/shortest-path oracle and four compiling controls; four strict Cortex-M4 object/link/stack audits and two hosted cost observations. Physical execution and independent learner work absent.'
    dump(path,d);print(json.dumps(dict(passed=d['passed'],commands=len(d['executions']),failure=d.get('failure'))));return int(not d['passed'])

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();raise SystemExit(run(a.report))
