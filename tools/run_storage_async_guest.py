"""Compose the new read-lifetime adapter with the unchanged actual CMK2 kernel."""
from pathlib import Path
from datetime import datetime,timezone
import importlib.util,json,os,shutil,subprocess,time
from common import ROOT,dump,read_json,sha

REPORT='research/v5/expansion/legacy_storage_followthrough/async_guest_validation.json'
def main():
 runs=ROOT/'reviews/v5/expansion/legacy_storage_followthrough'/('async-guest-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 original=ROOT/'labs/kernel_smp';source=ROOT/'assessor/kernel_smp';public=ROOT/'labs/storage_followthrough/async'
 inputs={}
 for folder in [original,source,public,ROOT/'assessor/storage_followthrough/async']:
  for p in folder.rglob('*'):
   if p.is_file() and not any(x in p.parts for x in ['__pycache__','build','.cache']):inputs[p.relative_to(ROOT).as_posix()]=sha(p)
 for p in ['tools/run_storage_async_guest.py','tools/common.py']:inputs[p]=sha(ROOT/p)
 spec=importlib.util.spec_from_file_location('storage_async_guest_provider',original/'tools/guest.py');g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
 records=[];builds=[];observations=[];external={}
 def execute(cmd,label,timeout=120):
  cmd=list(map(str,cmd));start=time.monotonic()
  try:p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=timeout);code=p.returncode;out=p.stdout+p.stderr
  except subprocess.TimeoutExpired as e:code=None;out='TIMEOUT '+str(e)
  path=runs/(f'{len(records):04d}-'+label+'.log');path.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=code,seconds=time.monotonic()-start,log=path.relative_to(ROOT).as_posix(),log_sha256=sha(path)));return code,out
 for tool in ['gcc','ld','nm','readelf','grub-file','grub-mkrescue','qemu-system-x86_64','xorriso']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path)
  assert execute([tool,'--help' if tool=='grub-file' else '--version'],'version-'+tool)[0]==0
 oldenv=os.environ.copy()
 try:
  for key in list(os.environ):
   if key.startswith('CMASTERY_STORAGE') or key.startswith('CMASTERY_RUNTIME'):os.environ.pop(key)
  for mode in ['reference','mutant1','mutant3','learner']:
   home=runs/mode;lab=home/'lab';work=home/'guest';shutil.copytree(original,lab,ignore=shutil.ignore_patterns('__pycache__','build','.cache'))
   shutil.copyfile(public/'async.h',lab/'include/async.h')
   impl=public/'async.c' if mode=='learner' else ROOT/'assessor/storage_followthrough/async/async.c'
   body=impl.read_text(encoding='utf8');body=('#define SA_MUTANT '+mode[-1]+'\n' if mode.startswith('mutant') else '')+body
   (lab/'tests/sa_impl.c').write_text(body,encoding='utf8');shutil.copyfile(public/'guest.c',lab/'tests/sa_guest.c')
   p=lab/'tests/user_v2.c';text=p.read_text(encoding='utf8');a='static void producer(uintptr_t index){';assert text.count(a)==1;text=text.replace(a,'#include "sa_impl.c"\n#include "sa_guest.c"\n'+a)
   a='message("PASS v2-process-slot-generation-reuse\\n");die(0);';assert text.count(a)==1;text=text.replace(a,'sa_guest();'+a);p.write_text(text,encoding='utf8')
   copied={p.relative_to(lab).as_posix():sha(p) for p in lab.rglob('*') if p.is_file()}
   def record_run(cmd,**kw):
    cmd=list(map(str,cmd))
    if cmd[0]=='gcc' and '-c' in cmd:cmd+=['-MD','-MF',cmd[cmd.index('-o')+1]+'.d']
    code,out=execute(cmd,mode+'-build');assert code==0,out;return out
   g.run=record_run;g.build(lab,source,work)
   dependencies={}
   for dep in work.glob('*.d'):
    for token in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
     path=Path(token)
     if path.is_file():
      dependencies[str(path)]=sha(path)
      if not path.is_relative_to(ROOT):external.setdefault(str(path),sha(path))
   build=read_json(work/'build.json');assert build['built'] and '-std=c17' in build['flags'] and not build['runtime_linked'] and not build['storage_linked']
   artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file() and p.name!='disk.raw'}
   builds.append(dict(mode=mode,build_report=(work/'build.json').relative_to(ROOT).as_posix(),build_sha256=sha(work/'build.json'),original_workload_sha256=sha(original/'tests/user_v2.c'),copied_inputs=copied,compiler_dependencies=dependencies,artifacts=artifacts))
   for cpus in ([1,2,4] if mode=='reference' else [2]):
    (work/'disk.raw').write_bytes(bytes(4*1024*1024));os.environ['CMASTERY_CPUS']=str(cpus);cmd=g.command(work,'write');code,out=execute(cmd,mode+'-cpu'+str(cpus),90)
    if mode=='reference':ok=code==33 and 'SA_GUEST_PASS actual-CMK2-read-borrow-completion-before-reuse bytes4096' in out and 'SA_GUEST retained-after-abandon-before-positive-wait' in out and 'PASS smp-suite' in out and 'PASS kernel-suite' in out and 'FAIL' not in out and 'SA_GUEST_CHECK' not in out
    else:ok=code==35 and 'SA_GUEST_CHECK' in out and 'SA_GUEST_PASS' not in out
    observations.append(dict(mode=mode,cpus=cpus,passed=ok,execution=len(records)-1,expected_exit=33 if mode=='reference' else 35,kind='actual-QEMU-CMK2-user-read-lifetime' if mode=='reference' else 'compiled-guest-semantic-rejection'))
    assert ok,out
 finally:os.environ.clear();os.environ.update(oldenv)
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,passed=True,status='passed',producer='tools/run_storage_async_guest.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,executions=records,builds=builds,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Actual original kernel and user regressions plus isolated additive lifetime adapter; no provider edit.','One/two/four TCG virtual CPUs are not physical timing observations.','Abandonment does not force the device still to be active; no cancellation acknowledgement or error-quiescence claim.','The wait-error quarantine mutant is native-model evidence, not an actual device timeout here.']))
 print(dict(passed=True,commands=len(records),observations=len(observations)))
if __name__=='__main__':main()
