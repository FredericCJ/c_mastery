"""Actual token-ring campaigns, original runtime closure and cost observations."""
import argparse,json,os,subprocess,tempfile
from pathlib import Path
from common import ROOT,dump,sha
import run_expansion_family as base
DEFAULT='reviews/v5/expansion/corpus_ring/current_validation.json'

def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_corpus_ring.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    extra=[Path(__file__),ROOT/'tools/check_corpus_ring.py',ROOT/'assessor/execution/support.c',ROOT/'assessor/replicated/hashing/hashing.c',ROOT/'labs/replicated/hashing/hashing.h']
    extra_pins={p.relative_to(ROOT).as_posix():sha(p) for p in extra}
    result=base.run('corpus_ring',report);path=ROOT/report;d=json.loads(path.read_text());d['inspected_paths'].update(extra_pins)
    folder=path.parent/'target-and-cost';folder.mkdir(parents=True,exist_ok=True)
    d['target_audits']=[];d['independent_oracles']=[];d['measurements']=[];d['hash_integrations']=[];artifacts={}
    def execute(argv,label):
        argv=list(map(str,argv));p=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,timeout=180)
        log=folder/(label+'.log');log.write_text(p.stdout+'\n'+p.stderr)
        d['executions'].append(dict(command=argv,cwd=str(ROOT),exit_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        if p.returncode:raise RuntimeError(label+' exit '+str(p.returncode))
        return p.stdout
    try:
        if result:raise RuntimeError('Host campaign failed')
        for cc in ['arm-none-eabi-gcc','clang']:
            execute([cc,'--version'],cc+'.version')
            for opt in ['-O0','-O3']:
                tag=cc+opt;obj=folder/(tag+'.o');helper=folder/(tag+'.helpers.o');elf=folder/(tag+'.elf');mapping=folder/(tag+'.map')
                flags=[*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17',opt,'-Wall','-Wextra','-Werror','-pedantic-errors','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I',ROOT/'labs/corpus_ring']
                execute([cc,*flags,'-c',ROOT/'assessor/corpus_ring/ring.c','-o',obj],tag+'.compile')
                imports=execute(['arm-none-eabi-nm','-u',obj],tag+'.imports')
                names={line.split()[-1] for line in imports.splitlines() if line.strip()}
                assert names <= {'memcpy','memset','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8','__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8'},names
                execute([cc,*flags,'-c',ROOT/'assessor/execution/support.c','-o',helper],tag+'.helpers')
                assert not execute(['arm-none-eabi-nm','-u',helper],tag+'.helper-imports').strip()
                execute(['arm-none-eabi-ld','--entry=cr_plan','-Map='+str(mapping),obj,helper,'-o',elf],tag+'.link')
                assert not execute(['arm-none-eabi-nm','-u',elf],tag+'.image-imports').strip()
                assembly=execute(['arm-none-eabi-objdump','-dr',elf],tag+'.assembly');assert '<cr_plan>' in assembly and '<cr_replicas>' in assembly
                execute(['arm-none-eabi-size',elf],tag+'.size')
                for p in [obj,helper,elf,mapping,obj.with_suffix('.su'),helper.with_suffix('.su')]:assert p.is_file();artifacts[p.relative_to(ROOT).as_posix()]=sha(p)
                d['target_audits'].append(dict(compiler=cc,optimization=opt,core_imports=sorted(names),original_provider='assessor/execution/support.c',final_imports=[],libraries=[],execution='not-run'))
        with tempfile.TemporaryDirectory(prefix='ring-cost-') as tmp:
            for cc in ['gcc','clang']:
                flags=['-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic-errors','-I',ROOT/'labs/corpus_ring']
                for name,source in [('measure','measure.c'),('oracle','driver.c')]:
                    binary=Path(tmp)/(cc+'-'+name)
                    execute([cc,*flags,ROOT/'labs/corpus_ring'/source,ROOT/'assessor/corpus_ring/ring.c','-o',binary],cc+'.'+name+'-build')
                    if name=='measure':
                        raw=execute([binary],cc+'.measure');assert len(raw.splitlines())==16 and 'load_max=65536' in raw
                        d['measurements'].append(dict(compiler=cc,raw=raw,keys_per_layout=65536,limits='Bounded host CPU ticks and exact examined-slot costs; not board latency or a universal speed claim.'))
                    else:
                        obs=json.loads(execute(['python3',ROOT/'assessor/corpus_ring/oracle.py',binary],cc+'.oracle'));assert obs['passed'] and obs['observations']==14414
                        d['independent_oracles'].append(dict(compiler=cc,**obs))
                binary=Path(tmp)/(cc+'-hash-adapter')
                execute([cc,*flags,'-I',ROOT/'labs/replicated/hashing',ROOT/'labs/corpus_ring/hash_driver.c',ROOT/'labs/corpus_ring/sha_adapter.c',ROOT/'assessor/corpus_ring/ring.c',ROOT/'assessor/replicated/hashing/hashing.c','-o',binary],cc+'.hash-adapter-build')
                obs=json.loads(execute(['python3',ROOT/'assessor/corpus_ring/hash_oracle.py',binary],cc+'.hash-adapter'));assert obs['passed'] and obs['observations']==513
                d['hash_integrations'].append(dict(compiler=cc,**obs))
                binary=Path(tmp)/(cc+'-helpers')
                execute([cc,*flags,'-ffreestanding','-fno-builtin',ROOT/'labs/corpus_ring/helper_test.c',ROOT/'assessor/execution/support.c','-o',binary],cc+'.helper-test-build')
                assert 'CR_HELPERS:1040 passed' in execute([binary],cc+'.helper-test')
        for p,h in d['inspected_paths'].items():assert sha(ROOT/p)==h,('changed source',p)
    except Exception as e:d['passed']=False;d['failure']=str(e)
    d['status']='passed' if d['passed'] else 'failed';d['generated_artifact_sha256']=artifacts
    d['scope']='Actual six-profile token-ring/refusal campaigns, independent modular-distance oracle, native cost observations, explicit original-helper Cortex-M4 object/link closure; no target execution or learner assessment.'
    dump(path,d);print(json.dumps(dict(passed=d['passed'],commands=len(d['executions']),failure=d.get('failure'))));return int(not d['passed'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();raise SystemExit(run(a.report))
