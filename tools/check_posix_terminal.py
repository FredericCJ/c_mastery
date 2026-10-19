"""Guard original terminal criteria, explicit imports and standalone learner exports."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import cli
from check_posix_followthrough import fresh
from research.v5.expansion.legacy_posix_terminal.author_support import attach
def main(report):
 base=ROOT/'reviews/v5/expansion/legacy_posix_terminal';dest=(ROOT/report).resolve()
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use terminal evidence directory')
 checks=[];paths=[Path(__file__).resolve(),ROOT/'tools/check_posix_followthrough.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_posix_terminal/author_support.py',ROOT/'research/v5/expansion/legacy_runtime/author_supplements.py']
 public=ROOT/'labs/posix_lifecycle_depth/terminal'
 for project in ('PR-T23-04','PR-T24-06'):
  canonical='curriculum/v5/expansion/legacy_assessments/'+project+'.json';paths.append(ROOT/canonical);current=json.loads((ROOT/canonical).read_text(encoding='utf-8'))
  old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+canonical],cwd=ROOT,text=True,encoding='utf-8'))
  cleaned=copy.deepcopy(current['criteria'])
  for criterion in cleaned:criterion.pop('successor_support',None)
  for name,passed in [('original-criteria',cleaned==old['criteria']),('original-binding',current['baseline_binding']==old['baseline_binding']),('original-execution-observation',all(current['execution_support'].get(k)==v for k,v in old['execution_support'].items())),('attach-idempotent',attach(copy.deepcopy(current))==current)]:checks.append({'name':project+'-'+name,'passed':passed})
  with tempfile.TemporaryDirectory(prefix='terminal-export-') as tmp:
   work=Path(tmp)/'learner';cli.instantiate(project,work,family_name='posix_lifecycle_depth');actual=work/'terminal'
   copied=all((actual/p.name).read_bytes()==p.read_bytes() for p in public.iterdir() if p.is_file());leaks=[str(p.relative_to(work)) for p in work.rglob('*') if p.is_file() and 'assessor' in p.relative_to(work).parts]
   checks.append({'name':project+'-public-export','passed':copied and not leaks,'assessor_files':leaks})
   command=['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(actual),str(actual/'tests.c'),str(actual/'terminal_stub.c'),'-Wl,--wrap=tcgetattr,--wrap=tcsetattr,--wrap=fork,--wrap=pipe2,--wrap=clock_gettime','-o',str(work/'terminal-tests')]
   build=subprocess.run(command,capture_output=True,text=True,timeout=60);execution=subprocess.run([str(work/'terminal-tests'),'guard'],capture_output=True,text=True,timeout=15) if build.returncode==0 else None
   checks.append({'name':project+'-unsolved-guard','passed':execution is not None and execution.returncode==1 and 'PL_CHECK:' in execution.stderr,'command':command,'build_exit':build.returncode,'build_output':build.stdout+build.stderr,'execution_exit':None if execution is None else execution.returncode,'output':None if execution is None else execution.stdout+execution.stderr})
   missing=subprocess.run([sys.executable,str(actual/'run_stream.py'),'--parser-source',str(work/'absent.c'),'--parser-include',str(work)],capture_output=True,text=True,timeout=15)
   checks.append({'name':project+'-missing-explicit-import','passed':missing.returncode==2 and 'Missing explicit learner input' in missing.stderr,'output':missing.stdout+missing.stderr})
   # Explicit assessor input is author verification only. The exported runner
   # still defaults to its unsolved learner adapter and must reject it.
   imported=subprocess.run([sys.executable,str(actual/'run_stream.py'),'--parser-source',str(ROOT/'assessor/checkpoint_depth/checkpoint_depth.c'),'--parser-include',str(ROOT/'labs/checkpoint_depth/include')],capture_output=True,text=True,timeout=60)
   checks.append({'name':project+'-unsolved-adapter-with-explicit-reference-parser','passed':imported.returncode!=0 and 'PL_CHECK:' in imported.stderr and 'CalledProcessError' in imported.stderr,'output':imported.stdout+imported.stderr})
 campaign_file=base/'terminal_validation.json';paths.append(campaign_file);campaign=json.loads(campaign_file.read_text(encoding='utf-8'));fresh(campaign,120,6);checks.append({'name':'120-source-bound-outcomes','passed':True})
 for name,alter in [('failed-campaign',lambda d:d.update(passed=False)),('missing-outcome',lambda d:d['profiles'][0]['outcomes'].pop()),('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64}))]:
  negative=copy.deepcopy(campaign);alter(negative)
  try:fresh(negative,120,6);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 paths += [p for p in public.iterdir() if p.is_file()]
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Author preservation/export/import guards; no external review, learner attempt or physical observation.'}
 dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':value['passed'],'checks':len(checks)}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(main(a.report))
