"""Actual C17 native/sanitizer profiles, finite RC11 and strict M4 closure."""
import argparse,json,subprocess,re
from pathlib import Path
import run_expansion_family as base
from common import ROOT,sha
DEFAULT='reviews/v5/expansion/corpus_treiber/current_validation.json'
GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc')
def run(report):
 extra=['tools/run_corpus_treiber.py','tools/check_corpus_treiber.py','tools/verify_corpus_treiber.py','research/v5/modern/messaging_genmc_toolchain.json']
 pins={p:sha(ROOT/p) for p in extra}
 result=base.run('corpus_treiber',report);path=ROOT/report;d=json.loads(path.read_text());d['inspected_paths'].update(pins)
 d['model_checks']=[];d['target_audits']=[];d['generated_artifact_sha256']={}
 build=ROOT/'build/corpus-treiber-target';build.mkdir(parents=True,exist_ok=True)
 def execute(args,name,negative=False):
  args=list(map(str,args))
  try:r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=180);code=r.returncode;out=r.stdout+r.stderr
  except subprocess.TimeoutExpired as e:code=None;out=str(e)
  ok=(code==1 and 'Error: Non-atomic race!' in out and 'Compilation complete.' in out and 'Verification unsuccesful.' in out) if negative else code==0
  d['executions'].append(dict(id=name,command=args,exit_code=code,stdout=out,passed=ok,expected_exit=1 if negative else 0))
  assert ok,(name,code,out);return out
 try:
  assert result==0,'hosted campaign'
  version=execute([GENMC,'--version'],'genmc.version');tool=json.loads((ROOT/extra[-1]).read_text())
  assert tool['binary_sha256']==sha(GENMC) and 'v0.19.0' in version and 'LLVM 21.1.8' in version
  d['model_tool']=dict(path=str(GENMC),sha256=sha(GENMC),version=version,model='RC11',limits='Two finite actual-implementation models; not unbounded histories or liveness.')
  for model,mutation in [(1,0),(2,0),(1,5),(2,6)]:
   name=f'model{model}.mutation{mutation}';dot=build/(name+'.dot')
   out=execute([GENMC,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','-dump-error-graph='+str(dot),'--','-std=c17','-I'+str(ROOT/'labs/corpus_treiber'),'-include',ROOT/'assessor/corpus_treiber/treiber.c','-DTS_MODEL='+str(model),'-DTS_MUTANT='+str(mutation),ROOT/'labs/corpus_treiber/model.c'],name,bool(mutation))
   if not mutation:assert 'No errors were detected.' in out and 'Verification complete.' in out
   else:assert dot.is_file();d['generated_artifact_sha256'][dot.relative_to(ROOT).as_posix()]=sha(dot)
   count=int(re.search(r'Number of complete executions explored: (\d+)',out)[1])
   d['model_checks'].append(dict(model=model,mutation=mutation,complete_executions=count,outcome='data-race-counterexample' if mutation else 'bounded-no-error',passed=True))
  for cc in ['arm-none-eabi-gcc','clang']:
   execute([cc,'--version'],cc+'.version')
   for opt in ['-O0','-O3']:
    tag=cc+opt;obj=build/(tag+'.o');elf=build/(tag+'.elf');mapping=build/(tag+'.map')
    execute([cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-I',ROOT/'labs/corpus_treiber','-c',ROOT/'assessor/corpus_treiber/treiber.c','-o',obj],tag+'.compile')
    assert not execute(['arm-none-eabi-nm','-u',obj],tag+'.undefined').strip()
    execute(['arm-none-eabi-ld','--entry=ts_push','-Map='+str(mapping),obj,'-o',elf],tag+'.link')
    assert not execute(['arm-none-eabi-nm','-u',elf],tag+'.link-undefined').strip()
    asm=execute(['arm-none-eabi-objdump','-dr',elf],tag+'.disassembly');assert all(x in asm for x in ['<ts_push>','<ts_commit>','ldrex','strex','dmb'])
    for p in [obj,elf,mapping]:d['generated_artifact_sha256'][p.relative_to(ROOT).as_posix()]=sha(p)
    d['target_audits'].append(dict(compiler=cc,optimization=opt,language='c17',execution='not-run',undefined_symbols=[],libraries=[],section_sizes=execute(['arm-none-eabi-size','-A',elf],tag+'.sizes'),stack_usage=obj.with_suffix('.su').read_text(),atomic_instructions=['ldrex','strex','dmb'],limits='Actual emitted core frames/instructions, not target execution, exclusive-monitor progress or physical timing.'))
  assert all(sha(ROOT/p)==h for p,h in d['inspected_paths'].items())
 except Exception as error:d['passed']=False;d['failure']=str(error)
 d['status']='passed' if d['passed'] else 'failed';d['scope']='Actual bounded Treiber stack, native histories and finite RC11, with strict M4 translation/link only.'
 path.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=d['passed'],commands=len(d['executions']),models=d['model_checks'],targets=len(d['target_audits']),failure=d.get('failure'))));return int(not d['passed'])
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();return run(a.report)
if __name__=='__main__':raise SystemExit(main())
