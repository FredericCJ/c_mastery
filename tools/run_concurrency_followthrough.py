"""Exact native mechanisms, existing protection composition, models and target audit."""
import argparse,hashlib,importlib.util,json,os,subprocess,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('cf_public',ROOT/'labs/concurrency_followthrough/run.py');public=importlib.util.module_from_spec(spec);spec.loader.exec_module(public)
SOURCE=ROOT/'assessor/concurrency_followthrough';PROVIDERS=json.loads((SOURCE/'providers.json').read_text())
PROFILES={'O0':['-O0','-g'],'O3':['-O3'],'asan':['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'],'tsan':['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer']}
CASES={'core':['ring','batch','cas','pool','diagnostic'],'history':['tests'],'epoch':['registration','threads','retention','arrivals'],'protocol':['pause','stress','baseline'],'protection':['validation','protection','collector','generation','registration','retention','departure'],'reservation':['tests'],'versions':['epoch','versions','pressure']}
MUTANTS=[('CF_MUTANT_PUBLISH','core','batch','r == (pop ? CF_FULL : CF_EMPTY)'),('CF_MUTANT_REUSE','core','batch','r == (pop ? CF_FULL : CF_EMPTY)'),('CF_MUTANT_CANCEL_LINKED','core','pool','cf_cancel(&p, b) == CF_STALE'),('CH_MUTANT_PRECEDENCE','history','tests','== CH_NONLINEAR'),('CH_MUTANT_DROP_PENDING','history','tests','w.selected_pending == 1')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 files={Path(__file__),ROOT/'labs/concurrency_followthrough/run.py',SOURCE/'providers.json',ROOT/'labs/concurrency_followthrough/providers.json'}
 for family in ('labs/concurrency_followthrough','assessor/concurrency_followthrough'):
  files|={p for p in (ROOT/family).rglob('*') if p.is_file() and p.suffix in ('.c','.h','.py') and '__pycache__' not in p.parts}
 for part in CASES:files|=set(public.recipe(part,SOURCE,PROVIDERS))
 for folder in public.INCLUDES:files|={p for p in (ROOT/folder).glob('*.h')}
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);before=sources();rows=[];checks=[]
 env=os.environ.copy()
 for key in ('CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS'):env.pop(key,None)
 env.update(ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1',TSAN_OPTIONS='halt_on_error=1:exitcode=66')
 def run(name,cmd,expected=0,marker=None,timeout=120):
  cmd=list(map(str,cmd));start=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);passed=r.returncode==expected and (marker is None or ''.join(marker.split()) in ''.join(out.split()));rows.append(dict(name=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=passed,log=log.relative_to(ROOT).as_posix(),sha256=sha(log),elapsed_seconds=time.monotonic()-start));print(name,passed,flush=True)
  if not passed:raise RuntimeError(name+'\n'+out[-4000:])
  return out
 try:
  run('uname',['uname','-a']);run('libc',['ldd','--version'])
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version'])
   for profile,flags in PROFILES.items():
    tag=cc+'-'+profile
    for part,cases in CASES.items():
     exe=work/(tag+'-'+part);run(tag+'-'+part+'-compile',public.compile_command(cc,flags,part,SOURCE,PROVIDERS,exe))
     for case in cases:run(tag+'-'+part+'-'+case,[exe,case],marker={'core':'CF_PASS:','history':'CH_PASS:','epoch':'CE_PASS:','protocol':'CP_PASS:','protection':'OBS ','reservation':'OBS reservation','versions':'OBS '}[part])
    if profile=='O3':
     run(cc+'-history-oracle',[sys.executable,SOURCE/'history/oracle.py','--exe',work/(tag+'-history'),'--output',work/(cc+'-oracle'),'--protocol',work/(tag+'-protocol-pause.txt')],marker='"passed": true',timeout=180)
   for define,part,case,marker in MUTANTS:
    exe=work/(cc+'-'+define);run(cc+'-'+define+'-compile',public.compile_command(cc,['-O2','-D'+define],part,SOURCE,PROVIDERS,exe));run(cc+'-'+define,[exe,case],41,marker)
   for part,case in [('core','ring'),('history','tests'),('epoch','registration'),('protocol','baseline')]:
    exe=work/(cc+'-learner-'+part);providers=json.loads((ROOT/'labs/concurrency_followthrough/providers.json').read_text());run(cc+'-learner-'+part+'-compile',public.compile_command(cc,['-O2'],part,ROOT/'labs/concurrency_followthrough',providers,exe));run(cc+'-learner-'+part,[exe,case],41,'_CHECK:')
  run('models',[sys.executable,SOURCE/'models.py','--output',work/'models.json'],marker='CF_PASS:models')
  # New ring/counter/pool code: audit the selected32-bit atomic target only.
  # Wider/M0 rejection and alternative evidence remain in pinned providers.
  for cc in ('arm-none-eabi-gcc','clang'):
   for opt in ('O0','O3'):
    name=cc+'-'+opt+'-m4';obj=work/(name+'.o');exe=work/(name+'.elf')
    command=[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-mcpu=cortex-m4','-mthumb','-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-idirafter','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib','-I'+str(ROOT/'labs/concurrency_followthrough/core'),'-c',SOURCE/'core/core.c','-o',obj]
    run(name+'-compile',command);imports=run(name+'-imports',['arm-none-eabi-nm','-u',obj]);checks.append(dict(name=name+' no implicit runtime',passed=not imports.strip()));assert not imports.strip()
    names=['cf_ring_init','cf_push','cf_pop','cf_increment','cf_pool_init','cf_take','cf_publish','cf_cancel','cf_retire','cf_reclaim'];run(name+'-link',['arm-none-eabi-ld','--gc-sections','--entry=cf_ring_init',*['--undefined='+n for n in names],obj,'-Map='+str(work/(name+'.map')),'-o',exe]);run(name+'-disassembly',['arm-none-eabi-objdump','-dr',exe]);assert not run(name+'-closed',['arm-none-eabi-nm','-u',exe]).strip()
 except Exception as e:error=str(e);passed=False
 else:error=None;passed=True
 after=sources()
 if after!=before:passed=False;error='source changed during mechanism campaign'
 result=dict(passed=passed,error=error,rows=rows,checks=checks,source_sha256=after,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Exact original C17 author execution, distinct premise models, independent finite history oracle and selected M4 image audit; not physical, independent learner, unbounded progress or all-history proof.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),checks=len(checks))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
