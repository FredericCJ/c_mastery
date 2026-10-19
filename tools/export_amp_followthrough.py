"""Ten relocated public packages; complete references only in validation trees."""
from pathlib import Path
import argparse,hashlib,json,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_amp_followthrough.author_support import public_paths,records
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];packages=[]
 def run(name,cmd,expected,marker):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);okay=r.returncode==expected and marker in out;rows.append(dict(name=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=okay,log=log.relative_to(ROOT).as_posix(),sha256=sha(log)));print(name,okay,flush=True)
  if not okay:raise RuntimeError(name+'\n'+out[-5000:])
 try:
  for record in records():
   project=record['project'];number=int(project[-2:]);public=work/'public'/project;validation=work/'validation'/project
   for rel in public_paths():
    dst=public/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dst)
   (public/'ASSESSMENT.json').write_text(json.dumps(record,indent=2)+'\n');before={p.relative_to(public).as_posix():sha(p) for p in public.rglob('*') if p.is_file()}
   assert not any('assessor' in p.parts or 'reference' in p.parts for p in public.rglob('*'))
   shutil.copytree(public,validation);shutil.copytree(ROOT/'assessor/amp_followthrough',validation/'assessor/amp_followthrough',ignore=shutil.ignore_patterns('__pycache__'))
   for rel in json.loads((ROOT/'assessor/amp_followthrough/providers.json').read_text()).values():
    dst=validation/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dst)
   (validation/'artifacts').mkdir();(public/'artifacts').mkdir()
   group={1:'images',2:'boot',3:'ring',4:'doorbell',5:'cache',6:'buffers',7:'models',8:'ledger',9:'ledger',10:'boot'}[number]
   for cc in ('gcc','clang'):
    for kind,tree,source in [('public',public,public/'labs/amp_followthrough'),('reference',validation,validation/'assessor/amp_followthrough')]:
     name=project+'-'+cc+'-'+kind;expected=1 if kind=='public' else 0;build=tree/'artifacts'/cc
     if group=='models':cmd=[sys.executable,source/'models/analysis.py','--output',tree/'artifacts'/(cc+'.json')];marker='AF_UNIMPLEMENTED:' if expected else 'AF_PASS:analysis'
     elif group=='ledger':
      build.mkdir();exe=build/'ledger';run(name+'-compile',[cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-I'+str(tree/'labs/amp_followthrough/core'),tree/'labs/amp_followthrough/core/ledger_tests.c',source/'core/ledger.c','-o',exe],0,'');cmd=[exe];marker='AF_LEDGER_CHECK:' if expected else 'AF_PASS:ledger'
     else:cmd=[sys.executable,tree/'labs/amp_followthrough/run.py','--source',source,'--providers',source/'providers.json','--build',build,'--cc',cc,'--group',group];marker=('AF_DOOR_CHECK:' if group=='doorbell' else 'AF_BUFFER_CHECK:' if group=='buffers' else 'AF_CHECK:') if expected else 'AF_PASS:'
     run(name,cmd,expected,marker)
     if number in (9,10):
      run(name+'-target',[sys.executable,tree/'labs/amp_followthrough/target/build.py','--source',source,'--providers',source/'providers.json','--build',tree/'artifacts'/(cc+'-target'),'--cc',cc,'--mode','pipeline','--qemu','--burst','4'],expected,'missing executed boundary' if expected else '"passed": true')
   assert all(sha(public/p)==h for p,h in before.items())
   packages.append(dict(project=project,public_root=public.relative_to(ROOT).as_posix(),public_sha256=before,private_reference_in_public=False,group=group,original_ids=[c['id'] for c in record['criteria']]))
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 pins={p:sha(ROOT/p) for p in public_paths()};pins[Path(__file__).relative_to(ROOT).as_posix()]=sha(Path(__file__))
 report=dict(passed=passed,error=error,rows=rows,packages=packages,source_sha256=pins,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Relocated incomplete public sources compile and reject semantic checks. Complete references and target providers exist only in separate validation trees; author use is not learner completion.')
 (work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),packages=len(packages))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
