"""Verify exact checkpoint successor IDs, evidence freshness and retained task contracts."""
import argparse,copy,hashlib,importlib.util,json,subprocess,sys
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_completion import support

def run():
 data=support.read(support.REGISTER);errors=[];controls=[]
 for path,value in data['source_sha256'].items():
  if not (ROOT/path).is_file() or support.sha(path)!=value:errors.append('Stale support input '+path)
 expected={f'AX-{project}-{leaf}' for project,rows in support.ROWS.items() for leaf,_,_ in rows}
 actual=[row['id'] for row in data['updates']]
 if set(actual)!=expected or len(actual)!=len(expected):errors.append('Wrong or duplicate criterion IDs')
 for path in data['report_status']:
  current=support.report_status(path)
  if current!=data['report_status'][path] or not current['passed']:errors.append('Missing/stale/failing report '+path)
 for project in support.ROWS:
  relative='curriculum/v5/expansion/legacy_assessments/'+project+'.json';record=support.read(relative)
  baseline=subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+relative],cwd=ROOT,text=True,encoding='utf-8')
  original=json.loads(baseline)
  original_criteria=original['criteria'];current=copy.deepcopy(record['criteria'])
  for criterion in current:criterion.pop('successor_support',None)
  if current!=original_criteria:errors.append('Original criterion/task/rubric changed '+project)
  if record['baseline_binding']!=original['baseline_binding']:errors.append('Historical baseline binding changed '+project)
  if any(record['execution_support'].get(k)!=v for k,v in original['execution_support'].items()):errors.append('Original execution observation overwritten '+project)
  if record.get('successor_support',{}).get('path')!=support.REGISTER:errors.append('Missing active successor '+project)
  if support.attach(copy.deepcopy(record))!=record:errors.append('Non-idempotent authoring attachment '+project)
 for row in data['updates']:
  dimensions=row['dimensions']
  if set(dimensions)!={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}:errors.append('Missing evidence dimension '+row['id'])
  if dimensions['external_review']['state']!='not-run' or dimensions['independent_learner']['state']!='not-assessed':errors.append('Fabricated assessment '+row['id'])
  if not row['mechanism_scope'] or not row['tests'] or not row['variant_ids']:errors.append('Missing criterion-specific binding '+row['id'])
 report=next(iter(data['report_status']));real=support.read;value=real(report)
 # These controls deliberately alter in-memory evidence, never stored receipts.
 for label,mutate in [('failed-report',lambda x:x.update(passed=False)),('stale-source',lambda x:next(iter(x['inspected_paths'])) and x['inspected_paths'].update({next(iter(x['inspected_paths'])):'0'*64}))]:
  changed=copy.deepcopy(value);mutate(changed)
  with patch.object(support,'read',side_effect=lambda path:changed if path==report else real(path)):
   rejected=not support.report_status(report)['passed']
  controls.append({'case':label,'rejected':rejected})
 missing='reviews/v5/expansion/legacy_completion/nonexistent-evidence.json'
 controls.append({'case':'missing-report','rejected':not support.report_status(missing)['passed']})
 wrong=copy.deepcopy(data['updates']);wrong[0]['id']='AX-PR-P1-01-T99.99.z'
 controls.append({'case':'foreign-criterion','rejected':{x['id'] for x in wrong}!=expected})
 exports=support.read('reviews/v5/expansion/legacy_completion/checkpoint_depth_exports.json')
 if not exports['passed'] or not support.report_status('reviews/v5/expansion/legacy_completion/checkpoint_depth_exports.json')['passed']:errors.append('Missing/current standalone export proof')
 if errors or not all(c['rejected'] for c in controls):return {'passed':False,'errors':errors,'negative_controls':controls}
 return {'passed':True,'criteria':len(actual),'projects':len(support.ROWS),'reports':len(data['report_status']),'negative_controls':controls,
         'checks':['Exact original criterion IDs and per-objective mechanisms','Every source/evidence hash current','Original task/rubric/prerequisite and historical observation retained','Pure authoring attachment idempotent','Six evidence dimensions; no fabricated review/learner completion','All12 standalone exports preserve learner stubs and reject them semantically'],
         'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ('tools/check_checkpoint_support.py','research/v5/expansion/legacy_completion/support.py',support.REGISTER)},
         'limits':'Structural/source-bound reference readiness verification only. It does not independently review every algorithm or perform a learner/physical assessment.'}

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report');args=parser.parse_args();result=run()
 if args.report:
  destination=(ROOT/args.report).resolve()
  if not destination.is_relative_to((ROOT/'reviews/v5/expansion/legacy_completion').resolve()):raise ValueError('completion evidence only')
  destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps(result,indent=2));raise SystemExit(0 if result['passed'] else 1)
