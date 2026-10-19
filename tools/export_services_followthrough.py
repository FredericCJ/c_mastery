"""Relocate eleven public packages; accepted prerequisites stay in private validation."""
import argparse,hashlib,json,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_services_followthrough.author_support import public_paths,records
CASES={1:26,2:16,3:15,4:27,5:8,6:10,8:17,9:19,10:21,11:23,12:25}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--mechanisms',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];packages=[]
 m=json.loads(a.mechanisms.read_text());assert m['passed'];providers={}
 for row in m['components']:
  d=json.loads((ROOT/row['path']).read_text());providers.update(d['source_sha256'])
 def run(name,cmd,cwd,expected=0,marker=None):
  r=subprocess.run(list(map(str,cmd)),cwd=cwd,capture_output=True,text=True,timeout=180);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);okay=r.returncode==expected and (marker is None or marker in out);rows.append(dict(name=name,command=list(map(str,cmd)),cwd=cwd.relative_to(ROOT).as_posix(),returncode=r.returncode,expected=expected,marker=marker,passed=okay,log=log.relative_to(ROOT).as_posix(),sha256=sha(log)))
  if not okay:raise RuntimeError(name+'\n'+out[-3000:])
 try:
  for record in records():
   project=record['project'];case=CASES[int(project[-2:])];public=work/'public'/project;private=work/'validation'/project
   for rel in public_paths():dest=public/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dest)
   (public/'ASSESSMENT.json').write_text(json.dumps(record,indent=2)+'\n');public_hashes={p.relative_to(public).as_posix():sha(p) for p in public.rglob('*') if p.is_file()};assert not any('assessor' in p.parts for p in public.rglob('*'));shutil.copytree(public,private)
   for rel,h in providers.items():
    assert sha(ROOT/rel)==h,rel;dest=private/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dest)
   for rel in (ROOT/'assessor/services_followthrough').rglob('*'):
    if rel.is_file() and '__pycache__' not in rel.parts:dest=private/rel.relative_to(ROOT);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(rel,dest)
   reports=[]
   for cc in ('gcc','clang'):
    for kind,n in [('reference',case),('public-scaffold',1)]:
     name=project+'-'+cc+'-'+kind;cmd=[sys.executable,'labs/services_followthrough/guest.py','--kernel-source','assessor/kernel_smp','--kernel-lab','labs/kernel_smp','--sdk-source','assessor/service_depth/kernel','--sdk-public','labs/service_depth/adapters','--source','assessor/services_followthrough' if kind=='reference' else 'labs/services_followthrough','--work','build/'+name,'--case',n,'--cc',cc,'--opt','O2','--cpus',2]
     if kind=='public-scaffold':cmd+=['--expected',35,'--marker','SF_CHECK:typed current process']
     run(name,cmd,private,0,'"passed": true');path=private/'build'/name/'result.json';reports.append(dict(path=path.relative_to(ROOT).as_posix(),sha256=sha(path),case=n,kind=kind,root=private.relative_to(ROOT).as_posix()))
   assert all(sha(public/p)==h for p,h in public_hashes.items());packages.append(dict(project=project,public_root=public.relative_to(ROOT).as_posix(),validation_root=private.relative_to(ROOT).as_posix(),public_sha256=public_hashes,private_reference_in_public=False,reports=reports,exact_reference_case=case));print(project,'PASS',flush=True)
  passed=True;error=None
 except Exception as e:passed=False;error=repr(e)
 result=dict(passed=passed,error=error,rows=rows,packages=packages,source_sha256={**{p:sha(ROOT/p) for p in public_paths()},Path(__file__).relative_to(ROOT).as_posix():sha(Path(__file__))},artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Eleven unmodified public archives contain no private reference. Separate relocated validation trees supply accepted prerequisites, execute distinct private criterion cases and reject the incomplete public SF scaffold. Not independent learner execution.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),packages=len(packages))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
