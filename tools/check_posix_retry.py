"""Reject stale/forged retry evidence and verify explicit standalone imports."""
import argparse,copy,hashlib,json,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import cli
from check_posix_followthrough import fresh
from check_service_adapters_v5 import validate as validate_service
from research.v5.expansion.legacy_posix_retry.author_support import attach
MARKERS={'SDK_MUTANT_REAPPLY':'out==7','SDK_MUTANT_SPLIT_RECEIPT':'out==7','SDK_MUTANT_PAYLOAD_BINDING':'SDR_CONFLICT','learner':'sdr_initialize'}
def validate(report):
 fresh(report,30,6)
 if {p['name'] for p in report['profiles']}!={c+'-'+m for c in ('gcc','clang') for m in ('debug','release','asan')}:raise ValueError('Profile identities changed')
 for profile in report['profiles']:
  if {r['variant'] for r in profile['outcomes']}!={'reference',*MARKERS}:raise ValueError('Variant identities changed')
  for row in profile['outcomes']:
   variant=row['variant'];expected=0 if variant=='reference' else 1
   if row['build_exit']!=0 or row['execution_exit']!=expected:raise ValueError('Compile/crash cannot substitute for semantic result')
   marker='PR_PASS' if variant=='reference' else MARKERS[variant]
   if variant!='reference' and row.get('semantic_marker')!=marker:raise ValueError('Wrong semantic marker')
   candidates=[ROOT/p for p in report['log_sha256'] if Path(p).name==profile['name']+'-'+variant+'.log']
   if len(candidates)!=1:raise ValueError('Missing exact execution log')
   text=candidates[0].read_text(encoding='utf-8')
   if marker not in text or (variant!='reference' and ('PL_CHECK:' not in text or 'Sanitizer' in text)):raise ValueError('Wrong observed semantic result')
 binding=report['reused_evidence']['adapter'];path=ROOT/binding['path']
 if hashlib.sha256(path.read_bytes()).hexdigest()!=binding['sha256']:raise ValueError('Stale reused adapter')
 adapter=json.loads(path.read_text(encoding='utf-8'));errors=validate_service(adapter)
 if errors:raise ValueError('Invalid reused source-bound evidence: '+str(errors))
 expected=[r for r in adapter['runs'] if r['id'] in ('receipt-host','receipt-guests')]
 if report['reused_evidence']['receipts']!=expected:raise ValueError('Missing or changed reused receipt')
def main(report_path):
 base=ROOT/'reviews/v5/expansion/legacy_posix_retry';dest=(ROOT/report_path).resolve()
 if not dest.is_relative_to(base.resolve()):raise ValueError('Use retry evidence directory')
 canonical=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';current=json.loads(canonical.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+canonical.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));cleaned=copy.deepcopy(current['criteria'])
 for c in cleaned:c.pop('successor_support',None)
 checks=[{'name':'original-criteria','passed':cleaned==old['criteria']},{'name':'baseline-binding','passed':current['baseline_binding']==old['baseline_binding']},{'name':'historical-execution','passed':all(current['execution_support'].get(k)==v for k,v in old['execution_support'].items())},{'name':'idempotent-renderer','passed':attach(copy.deepcopy(current))==current}]
 campaign_path=base/'retry_validation.json';campaign=json.loads(campaign_path.read_text(encoding='utf-8'));validate(campaign);checks.append({'name':'30-current-executions-and-nested-reused-service-evidence','passed':True})
 for name,alter in [('failed-campaign',lambda d:d.update(passed=False)),('missing-outcome',lambda d:d['profiles'][0]['outcomes'].pop()),('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),('compile-failure',lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),('crash',lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),('wrong-marker',lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='generic failure')),('missing-reused-receipt',lambda d:d['reused_evidence']['receipts'].pop())]:
  bad=copy.deepcopy(campaign);alter(bad)
  try:validate(bad);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 with tempfile.TemporaryDirectory(prefix='retry-export-') as directory:
  work=Path(directory)/'learner';cli.instantiate('PR-T23-02',work,family_name='posix_lifecycle_depth');public=ROOT/'labs/posix_lifecycle_depth/retry'
  copied=all((work/'retry'/p.name).read_bytes()==p.read_bytes() for p in public.iterdir() if p.is_file());leaks=[str(p.relative_to(work)) for p in work.rglob('*') if p.is_file() and 'assessor' in p.relative_to(work).parts]
  checks.append({'name':'standalone-public-export','passed':copied and not leaks,'assessor_files':leaks})
  stub=next(p for p in work.rglob('receipt.c') if p.read_bytes()==(ROOT/'labs/service_depth/adapters/receipt.c').read_bytes());header=next(p for p in work.rglob('receipt.h'));store_header=next(p for p in work.rglob('store.h'))
  command=[sys.executable,str(work/'retry/run_retry.py'),'--receipt-source',str(stub),'--receipt-include',str(header.parent),'--store-source',str(ROOT/'assessor/storage/store.c'),'--store-include',str(store_header.parent)]
  result=subprocess.run(command,capture_output=True,text=True,timeout=90)
  checks.append({'name':'exported-unsolved-receipt-with-explicit-author-store-import','passed':result.returncode!=0 and 'PL_CHECK:' in result.stderr and 'sdr_initialize' in result.stderr,'command':command,'exit':result.returncode,'output':result.stdout+result.stderr})
 paths=[Path(__file__).resolve(),ROOT/'tools/check_posix_followthrough.py',ROOT/'tools/check_service_adapters_v5.py',canonical,campaign_path,ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_posix_retry/author_support.py',ROOT/'research/v5/expansion/legacy_runtime/author_supplements.py']
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Author exact-criterion source/preservation/import guards; not whole storage acceptance or learner independence.'};dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':value['passed'],'checks':len(checks)}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(main(a.report))
