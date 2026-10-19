"""Actual independent-profile campaigns for all T56 followthrough mechanisms."""
import argparse,concurrent.futures,hashlib,json,os,subprocess,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--jobs',type=int,choices=[1,2],default=2);a=p.parse_args();work=a.work;work.mkdir(parents=True,exist_ok=False);rows=[];components=[]
 inputs={p for base in ('labs/services_followthrough','assessor/services_followthrough') for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts};inputs.add(Path(__file__));before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(inputs)}
 env=os.environ.copy()
 for key in ('CFLAGS','CPPFLAGS','LDFLAGS','CPATH','LIBRARY_PATH','C_INCLUDE_PATH'):env.pop(key,None)
 def run(name,cmd,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,cmd)),cwd=cwd,env=env,capture_output=True,text=True,timeout=900);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);okay=r.returncode==expected and (marker is None or marker in out);row=dict(name=name,command=list(map(str,cmd)),returncode=r.returncode,expected=expected,marker=marker,passed=okay,log=log.as_posix(),sha256=sha(log));return row
 def guest(case,cc,opt,cpus,mutant='',negative=False):
  name=f'guest-{case}-{cc}-{opt}-{cpus}'+('-'+mutant if mutant else '')+('-learner' if negative else '');dest=work/name;cmd=[sys.executable,'labs/services_followthrough/guest.py','--kernel-source','assessor/kernel_smp','--kernel-lab','labs/kernel_smp','--sdk-source','assessor/service_depth/kernel','--sdk-public','labs/service_depth/adapters','--source','labs/services_followthrough' if negative else 'assessor/services_followthrough','--work',dest,'--case',case,'--cc',cc,'--opt',opt,'--cpus',cpus]
  if mutant:cmd+=['--define',mutant]
  if negative:cmd+=['--expected',35,'--marker','SF_CHECK:typed current process']
  if mutant:cmd+=['--expected',35,'--marker',{'SF_MUTANT_CONSUME_COPY':'SF_CHECK:no partial bytes or message consumption','SF_MUTANT_THREAD16':'SF_CHILD_CHECK:','SDK_MUTANT_LOAD_LEAK':'SF_CHECK:all actual ELF allocation cuts','SDK_MUTANT_OFFSET':'SF_CHECK:first offset three','SDK_MUTANT_PAYLOAD_BINDING':'SF_CHECK:same operation ID different payload rejected'}[mutant]]
  row=run(name,cmd,0,'"passed": true');return row,dict(path=(dest/'result.json').as_posix(),sha256=sha(dest/'result.json'),case=case,compiler=cc,optimization=opt,cpus=cpus,mutant=mutant,learner=negative)
 try:
  for case,mutant in ((9,'SF_MUTANT_CONSUME_COPY'),(12,'SF_MUTANT_THREAD16'),(16,'SDK_MUTANT_LOAD_LEAK'),(17,'SDK_MUTANT_OFFSET'),(21,'SDK_MUTANT_PAYLOAD_BINDING')):
   row,component=guest(case,'gcc','O2',2,mutant);rows.append(row);components.append(component);assert row['passed'],row['name']
  for cc in ('gcc','clang'):
   row,component=guest(1,cc,'O2',2,negative=True);rows.append(row);components.append(component);assert row['passed'],row['name']
  vm=['labs/services_followthrough/vm/tests.c','assessor/services_followthrough/vm/service.c','assessor/runtime/cvm.c','assessor/runtime_followthrough/verifier/verifier.c','assessor/runtime_followthrough/execution/execution.c'];vminc=['-Ilabs/services_followthrough/vm','-Ilabs/runtime/include','-Ilabs/runtime_followthrough/verifier','-Ilabs/runtime_followthrough/execution']
  store=['labs/services_followthrough/store/tests.c','assessor/services_followthrough/store/service.c','assessor/service_depth/kernel/receipt.c','assessor/storage/store.c'];storeinc=['-Ilabs/services_followthrough/store','-Ilabs/service_depth/adapters','-Ilabs/storage/include']
  workload=['labs/services_followthrough/workload/main.c','assessor/services_followthrough/workload/workload.c','assessor/amp_followthrough/core/amp.c'];workinc=['-Ilabs/services_followthrough/workload','-Ilabs/amp_followthrough/core']
  for cc in ('gcc','clang'):
   for profile,flags in [('O0',['-O0']),('O2',['-O2']),('san',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
    common=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags]
    for name,sources,inc,define,marker in [('vm',vm,vminc,['-DSFV_MAIN'],'PASS sfv:'),('store',store,storeinc,[],'PASS sfs:'),('workload',workload,workinc,[],'SFW 4 128 47 81 45 2 1980')]:
     stem=f'native-{name}-{cc}-{profile}';binary=work/stem
     for row in [run(stem+'-compile',[*common,*inc,*define,*sources,'-o',binary]),run(stem+'-run',[binary],0,marker)]:rows.append(row);assert row['passed'],row['name']
   for opt in ('O0','O2'):
    stem=f'target-{cc}-{opt}';dest=work/stem;row=run(stem,[sys.executable,'labs/services_followthrough/workload/target.py','--source','assessor/services_followthrough','--ring-source','assessor/amp_followthrough/core/amp.c','--support-source','assessor/execution/support.c','--work',dest,'--cc',cc,'--opt',opt],0,'"passed": true');rows.append(row);assert row['passed'];components.append(dict(path=(dest/'result.json').as_posix(),sha256=sha(dest/'result.json'),target=True))
  for name,sources,inc,define,marker in [('vm',vm,vminc,['-DSFV_MAIN','-DSFV_MUTANT_FOREIGN'],'SFV_CHECK:'),('vm-learner',[vm[0],'labs/services_followthrough/vm/service.c',*vm[2:]],vminc,['-DSFV_MAIN'],'SFV_CHECK:'),('store-learner',[store[0],'labs/services_followthrough/store/service.c',*store[2:]],storeinc,[],'SFS_CHECK:'),('workload-learner',[workload[0],'labs/services_followthrough/workload/workload.c',workload[2]],workinc,[],None)]:
   binary=work/('negative-'+name);row=run('negative-'+name+'-compile',['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*inc,*define,*sources,'-o',binary]);rows.append(row);assert row['passed'];row=run('negative-'+name+'-run',[binary],99 if name=='workload-learner' else 1,marker);rows.append(row);assert row['passed']
  specs=[(case,cc,opt,cpus) for case in range(1,28) for cc in ('gcc','clang') for opt in ('O0','O2') for cpus in (2,4)]
  with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as pool:
   futures=[pool.submit(guest,*spec) for spec in specs]
   for future in concurrent.futures.as_completed(futures):
    row,component=future.result();rows.append(row);components.append(component);print(row['name'],row['passed'],flush=True)
    if not row['passed']:raise RuntimeError('guest failure '+row['name'])
  passed=True;error=None
 except Exception as e:passed=False;error=repr(e)
 after={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(inputs)}
 if before!=after:passed=False;error='source changed during campaign'
 result=dict(passed=passed,error=error,rows=rows,components=components,source_sha256=after,artifacts={p.resolve().relative_to(ROOT).as_posix():sha(p) for p in sorted(work.rglob('*')) if p.is_file()},scope='Actual native, original SMP guest and H745 image gates. No independent learner, delayed execution or physical capture.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),components=len(components))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
