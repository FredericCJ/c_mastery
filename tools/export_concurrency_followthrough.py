"""Ten relocated learner packages with distinct semantic failures and private validation."""
import argparse,hashlib,json,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_concurrency_followthrough.author_support import public_paths,records
GROUPS={1:{'core':['ring','batch']},2:{'core':['cas'],'reservation':['tests']},3:{'protocol':['baseline'],'models':['analysis']},4:{'core':['pool'],'measure':['pool']},5:{'protection':['validation','collector','retention','departure','generation'],'epoch':['retention']},6:{'protocol':['pause','stress']},7:{'epoch':['registration','threads','retention']},8:{'versions':['epoch','versions','pressure'],'epoch':['arrivals'],'measure':['rw']},9:{'history':['tests'],'protocol':['baseline']},10:{'measure':['queue'],'core':['diagnostic']}}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];packages=[]
 def run(name,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);out=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(out);okay=r.returncode==expected and (marker is None or marker in out);rows.append(dict(name=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=okay,log=log.relative_to(ROOT).as_posix(),sha256=sha(log)));print(name,okay,flush=True)
  if not okay:raise RuntimeError(name+'\n'+out[-4000:])
 try:
  for record in records():
   project=record['project'];number=int(project[-2:]);learner=work/'public'/project;validation=work/'validation'/project
   for rel in public_paths():dst=learner/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dst)
   (learner/'ASSESSMENT.json').write_text(json.dumps(record,indent=2)+'\n');before={p.relative_to(learner).as_posix():sha(p) for p in learner.rglob('*') if p.is_file()};assert not any('assessor' in p.parts for p in learner.rglob('*'))
   shutil.copytree(learner,validation);shutil.copytree(ROOT/'assessor/concurrency_followthrough',validation/'assessor/concurrency_followthrough',ignore=shutil.ignore_patterns('__pycache__'))
   for rel in json.loads((ROOT/'assessor/concurrency_followthrough/providers.json').read_text()).values():dst=validation/rel;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(ROOT/rel,dst)
   for cc in ('gcc','clang'):
    for kind,tree in [('public',learner),('reference',validation)]:
     source=tree/('labs' if kind=='public' else 'assessor')/'concurrency_followthrough';expected=41 if kind=='public' else 0
     for part,cases in GROUPS[number].items():
      name=project+'-'+cc+'-'+kind+'-'+part;build=tree/'build'/(cc+'-'+part)
      if part=='models':run(name,[sys.executable,source/'models.py','--output',tree/'build'/(cc+'-models.json')],1 if kind=='public' else 0,'CF_UNIMPLEMENTED:' if kind=='public' else 'CF_PASS:models');continue
      run(name+'-compile',[sys.executable,tree/'labs/concurrency_followthrough/run.py','--part',part,'--source',source,'--providers',source/'providers.json','--build',build,'--cc',cc,'--compile-only'])
      for case in cases:
       if part=='measure':
        args={'queue':['queue',3,4,256,1,0,4000],'rw':['rw',0,1,500],'pool':['pool',2,1000]}[case]+[build/(case+'.csv')];marker='CF_MEASURE_UNIMPLEMENTED:' if expected else 'CF_MEASURE '
       else:args=[case];marker='_CHECK:' if expected and part in ('core','epoch','protocol','history') else '_ASSERT:' if expected else {'core':'CF_PASS:','epoch':'CE_PASS:','protocol':'CP_PASS:','history':'CH_PASS:','versions':'OBS ','protection':'OBS ','reservation':'OBS '}[part]
       run(name+'-'+case,[build/'program',*args],expected,marker)
   assert all(sha(learner/p)==h for p,h in before.items());packages.append(dict(project=project,public_root=learner.relative_to(ROOT).as_posix(),public_sha256=before,private_reference_in_public=False,groups=GROUPS[number],original_ids=[c['id'] for c in record['criteria']]))
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 pins={p:sha(ROOT/p) for p in public_paths()};pins[Path(__file__).relative_to(ROOT).as_posix()]=sha(Path(__file__))
 report=dict(passed=passed,error=error,rows=rows,packages=packages,source_sha256=pins,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Ten relocated public packages compile with intended mechanism-specific semantic rejection; complete implementations remain solely in separate reference validation trees.')
 (work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),packages=len(packages))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
