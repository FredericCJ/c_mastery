"""Relocated P6 public archives and separate source-explicit assessor executions."""
import argparse,hashlib,json,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_numeric_followthrough.author_support import records,public_paths
from run_numeric_followthrough import SOURCES,FLAGS,PROVIDERS,sha
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];packages=[]
 def run(name,cmd,cwd,expected=0,marker=None):
  r=subprocess.run(list(map(str,cmd)),cwd=cwd,capture_output=True,text=True,timeout=120);text=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(text);rows.append(dict(name=name,command=list(map(str,cmd)),returncode=r.returncode,expected=expected,marker=marker,passed=r.returncode==expected and (marker is None or marker in text),log=log.relative_to(ROOT).as_posix(),sha256=sha(log)));assert rows[-1]['passed'],name
 try:
  for r in records():
   public=work/'public'/r['project'];private=work/'validation'/r['project']
   for rel in public_paths():dest=public/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dest)
   (public/'ASSESSMENT.json').write_text(json.dumps(r,indent=2)+'\n');hashes={p.relative_to(public).as_posix():sha(p) for p in public.rglob('*') if p.is_file()};shutil.copytree(public,private)
   for rel in PROVIDERS+['assessor/numeric_followthrough/numeric.c']:dest=private/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dest)
   for cc in ('gcc','clang'):
    for kind in ('reference','learner'):
     name=r['project']+'-'+cc+'-'+kind;binary=private/name;sources=[s.replace('assessor/numeric_followthrough/numeric.c','labs/numeric_followthrough/numeric.c') if kind=='learner' else s for s in SOURCES];run(name+'-build',[cc,*FLAGS,'-O2',*sources,'-lm','-o',binary],private);run(name+'-run',[binary],private,1 if kind=='learner' else 0,'NF_CHECK:' if kind=='learner' else 'NF_PASS')
   assert all(sha(public/p)==h for p,h in hashes.items());packages.append(dict(project=r['project'],public_root=public.relative_to(ROOT).as_posix(),public_sha256=hashes,private_reference_in_public=False));print(r['project'],'PASS',flush=True)
  passed=True;error=None
 except Exception as exc:passed=False;error=repr(exc)
 result=dict(passed=passed,error=error,rows=rows,packages=packages,source_sha256={p:sha(ROOT/p) for p in public_paths()+PROVIDERS+['assessor/numeric_followthrough/numeric.c','tools/export_numeric_followthrough.py']},artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Two public packages without private sources; separate accepted prerequisites used for actual relocated reference and incomplete-companion semantic gates.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),packages=len(packages))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
