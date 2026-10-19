"""Twelve isolated public packages; references only in separate validation trees."""
import argparse,hashlib,json,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_mcu_followthrough.author_support import public_paths,records
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];packages=[]
 def run(name,command,expected,marker):
  r=subprocess.run(list(map(str,command)),cwd=ROOT,text=True,capture_output=True,timeout=60);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out)
  okay=(r.returncode==0 if expected==0 else r.returncode!=0) and marker in out;rows.append({'name':name,'command':list(map(str,command)),'returncode':r.returncode,'expected':expected,'marker':marker,'passed':okay,'log':log.relative_to(ROOT).as_posix()})
  if not okay:raise RuntimeError(name+'\n'+out[-4000:])
  print('PASS '+name,flush=True)
 try:
  for r in records():
   number=int(r['project'][-2:]);name=r['project'];public=work/'learner'/name;validation=work/'validation'/name
   for rel in public_paths():
    dst=public/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dst)
   (public/'ASSESSMENT.json').write_text(json.dumps(r,indent=2)+'\n')
   assert not any('assessor' in f.parts or 'reference' in f.parts for f in public.rglob('*'))
   public_hashes={f.relative_to(public).as_posix():sha(f) for f in public.rglob('*') if f.is_file()}
   shutil.copytree(public,validation);private=validation/'reference/mcu_followthrough';shutil.copytree(ROOT/'assessor/mcu_followthrough',private)
   runtime=validation/'reference/execution';runtime.mkdir(parents=True)
   for file in ('cmx.c','support.c'):shutil.copy2(ROOT/'assessor/execution'/file,runtime/file)
   mode={1:'direct',2:'preemptive',3:'direct',4:'preemptive',5:'sleep',6:'preemptive',7:'preemptive',8:'preemptive',9:'preemptive',10:'preemptive',11:'sleep',12:'integrated'}[number]
   group={1:'helper',2:'admission',3:'scheduling',4:'scheduling',5:'timers',6:'scheduling',7:'masks-sleep',8:'admission',9:'lifecycle',10:'faults',11:'timing',12:'lifecycle'}[number]
   for cc in ('gcc','clang'):
    prefix=name+'-'+cc
    run(prefix+'-public-native',[sys.executable,public/'labs/mcu_followthrough/run.py','--cmx',public/'labs/mcu_followthrough/baseline/cmx.c','--cc',cc,'--group',group,'--build',work/'builds'/(prefix+'-public-native')],1,'MF_CHECK:')
    run(prefix+'-reference-native',[sys.executable,validation/'labs/mcu_followthrough/run.py','--source',private/'core/mcu.c','--cmx',runtime/'cmx.c','--cc',cc,'--group',group,'--build',work/'builds'/(prefix+'-reference-native')],0,'MF_PASS:'+group)
    run(prefix+'-public-target',[sys.executable,public/'labs/mcu_followthrough/target/build.py','--runtime-source',public/'labs/mcu_followthrough/baseline','--cc',cc,'--mode',mode,'--run','--build',work/'builds'/(prefix+'-public-target')],1,'FAIL MF fault')
    cmd=[sys.executable,validation/'labs/mcu_followthrough/target/build.py','--source',private/'target','--runtime-source',runtime,'--core-source',private/'core/mcu.c','--cc',cc,'--mode',mode,'--run','--build',work/'builds'/(prefix+'-reference-target')]
    if number in (3,8):cmd+=['--fp']
    if number==10:cmd+=['--define','MF_MPU_FAULT']
    if number==12:cmd+=['--define','MF_UART','--define','MF_BORROWED']
    run(prefix+'-reference-target',cmd,0,'PASS MF '+mode)
   assert all(sha(public/p)==h for p,h in public_hashes.items())
   packages.append({'project':name,'public_root':public.relative_to(ROOT).as_posix(),'public_sha256':public_hashes,'original_ids':[c['id'] for c in r['criteria']],'private_reference_in_public':False,'mode':mode,'native_group':group})
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 sources={p:sha(ROOT/p) for p in public_paths()};sources[Path(__file__).relative_to(ROOT).as_posix()]=sha(Path(__file__))
 result={'passed':passed,'error':error,'rows':rows,'packages':packages,'source_sha256':sources,'artifacts':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Public trees retain incomplete providers; explicit references are confined to separate validation trees. Author execution is not learner completion.'}
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(rows),'packages':len(packages)}));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
