"""Verify equivalent-contract samples, original criterion and explicit import export."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import cli
from check_posix_followthrough import fresh
from research.v5.expansion.legacy_posix_workload.author_support import attach
def validate(report):
 fresh(report,42,6)
 for profile in report['profiles']:
  expected={('record',v) for v in ('reference','mutant1','mutant2','mutant3','learner')}|{('bench',v) for v in ('reference','weak_mapping')}
  if {(o['case'],o['variant']) for o in profile['outcomes']}!=expected:raise ValueError('Changed controls')
  for o in profile['outcomes']:
   if o['build_exit']!=0 or o['execution_exit']!=(0 if o['variant']=='reference' else 1):raise ValueError('Compile failure/crash is not semantic detection')
  candidates=[ROOT/p for p in report['log_sha256'] if Path(p).name==profile['name']+'-bench-reference.log']
  if len(candidates)!=1:raise ValueError('Missing actual sample log')
  actual=[{k:(v if k in ('method','workload') else int(v)) for k,v in (item.split('=',1) for item in line.split()[1:])} for line in candidates[0].read_text(encoding='utf-8').splitlines() if line.startswith('PW_SAMPLE ')]
  if len(actual)!=32 or profile['samples']!=actual:raise ValueError('Missing or altered measured sample')
  for workload in ('sequential','sparse_random'):
   rows=[r for r in actual if r['workload']==workload]
   if {(r['trial'],r['method']) for r in rows}!={(i,m) for i in range(8) for m in ('pread','mapped')} or len({r['digest'] for r in rows})!=1:raise ValueError('Unequal compared record results')
   if any(r['records']!=8192 or r['bytes']!=262144 or r['pread_calls']!=(8192 if r['method']=='pread' else 0) for r in rows):raise ValueError('Changed work/call accounting')
 binding=report['reused_fault_evidence'];path=ROOT/binding['path']
 if hashlib.sha256(path.read_bytes()).hexdigest()!=binding['sha256']:raise ValueError('Changed fault receipt')
 fresh(json.loads(path.read_text(encoding='utf-8')),114,6)
 if binding['reference_fault_outcomes']!=6:raise ValueError('Missing isolated fault evidence')
def main(report_path):
 base=ROOT/'reviews/v5/expansion/legacy_posix_workload';dest=(ROOT/report_path).resolve()
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use workload evidence directory')
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-03.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));cleaned=copy.deepcopy(current['criteria'])
 for c in cleaned:c.pop('successor_support',None)
 checks=[{'name':'original-criteria','passed':cleaned==old['criteria']},{'name':'baseline-binding','passed':current['baseline_binding']==old['baseline_binding']},{'name':'original-execution','passed':all(current['execution_support'].get(k)==v for k,v in old['execution_support'].items())},{'name':'attach-idempotent','passed':attach(copy.deepcopy(current))==current}]
 campaign_path=base/'workload_validation.json';campaign=json.loads(campaign_path.read_text(encoding='utf-8'));validate(campaign);checks.append({'name':'42-outcomes-192-current-samples-and-reused-faults','passed':True})
 for name,alter in [('missing-outcome',lambda d:d['profiles'][0]['outcomes'].pop()),('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),('compile-failure',lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),('crash',lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),('missing-sample',lambda d:d['profiles'][0]['samples'].pop()),('invented-fast-time',lambda d:d['profiles'][0]['samples'][0].update(wall_ns=1)),('changed-fault-receipt',lambda d:d['reused_fault_evidence'].update(sha256='0'*64))]:
  altered=copy.deepcopy(campaign);alter(altered)
  try:validate(altered);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 with tempfile.TemporaryDirectory(prefix='workload-export-') as directory:
  work=Path(directory)/'learner';cli.instantiate('PR-T23-03',work,family_name='posix_lifecycle_depth');public=ROOT/'labs/posix_lifecycle_depth/workload';actual=work/'workload';copied=all((actual/p.name).read_bytes()==p.read_bytes() for p in public.iterdir() if p.is_file());leaks=[str(p.relative_to(work)) for p in work.rglob('*') if p.is_file() and 'assessor' in p.relative_to(work).parts]
  checks.append({'name':'standalone-public-export','passed':copied and not leaks,'assessor_files':leaks})
  command=['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(actual),str(actual/'tests.c'),str(actual/'record_stub.c'),'-Wl,--wrap=pread,--wrap=__pread_chk','-o',str(work/'record')];build=subprocess.run(command,capture_output=True,text=True,timeout=60);execution=subprocess.run([str(work/'record')],capture_output=True,text=True,timeout=15) if build.returncode==0 else None
  checks.append({'name':'standalone-unsolved-record','passed':execution is not None and execution.returncode==1 and 'PL_CHECK:' in execution.stderr,'command':command,'build_exit':build.returncode,'build_output':build.stdout+build.stderr,'execution_exit':None if execution is None else execution.returncode,'output':None if execution is None else execution.stdout+execution.stderr})
 paths=[Path(__file__).resolve(),ROOT/'tools/check_posix_followthrough.py',path,campaign_path,ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_posix_workload/author_support.py',ROOT/'research/v5/expansion/legacy_runtime/author_supplements.py']
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Author source/sample/preservation/export guards; no independent learner or physical performance claim.'};dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':value['passed'],'checks':len(checks)}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(main(a.report))
