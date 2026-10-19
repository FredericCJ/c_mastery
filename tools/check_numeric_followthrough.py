"""Read-only exact P6 source/receipt/criterion/canonical export guard."""
import argparse,copy,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_numeric_followthrough.author_support import OPEN,HERE,SUPPORT,CAMPAIGN,NOTES,records,attach,public_paths
from run_numeric_followthrough import sha
from audit_numeric_followthrough import inspect,pins,fixtures,load
COVERAGE=dict(original_criteria=8,delayed_variants=8,native_correctness_profiles=6,no_avx_emulator_runs=4,semantic_controls=13,correctness_commands=57,measurement_commands=24,accepted_measurement_profiles=3,rejected_lto_categories=1,raw_timing_samples=1404,isolated_packages=2,export_commands=16)
def need(ok,message):
 if not ok:raise ValueError(message)
def validate_campaign(v):
 need(v['passed'] and v['coverage']==COVERAGE and v['open_gates']==OPEN,'incorrect coverage/gates');need(set(v['components'])=={'correctness','measurements','exports','audit'},'missing producer');pins(v['source_sha256']);data={}
 for n,r in v['components'].items():
  need(sha(ROOT/r['path'])==r['sha256'],'changed receipt');d=load(r['path']);need(d['passed'] and d['error'] is None,'failed producer');pins(d['source_sha256']);pins(d['artifacts']);data[n]=d
 a=data['audit'];need(all(a['components'][n]==v['components'][n] for n in ('correctness','measurements','exports')),'different audited producer');checks,ratios=inspect(*(data[n] for n in ('correctness','measurements','exports')));need(checks==a['checks'] and ratios==a['paired_ratios'] and all(x['passed'] for x in checks),'raw audit mismatch');need(fixtures()==v['fixtures']==a['fixtures'],'fixture mismatch')
 old=load(a['retained_receipt']['path']);need(sha(ROOT/a['retained_receipt']['path'])==a['retained_receipt']['sha256'] and old['passed'],'changed old receipt');pins(old['inspected_paths']);pins(old['log_sha256']);resources=load(HERE+'/resources.json');pins(resources['frozen_provider_sha256']);r=resources['retained_registration'];need(sha(ROOT/r['path'])==r['sha256'],'changed primary registration')
 for p in data['exports']['packages']:need(all(s in p['public_sha256'] and sha(ROOT/s)==p['public_sha256'][s] for s in public_paths()),'incomplete public inventory')
 return data
def main(report):
 v=load(CAMPAIGN);data=validate_campaign(v);checks=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 check('distinct actual mechanisms, exact binary measurements and exports',True)
 for name,mutation in [('missing-delayed',lambda x:x['coverage'].update(delayed_variants=7)),('independent-promotion',lambda x:x.update(open_gates=[])),('changed-measurement',lambda x:x['components']['measurements'].update(sha256='0'*64)),('missing-export',lambda x:x['components'].pop('exports')),('changed-source',lambda x:x.update(source_sha256={'labs/numeric_followthrough/numeric.h':'0'*64}))]:
  bad=copy.deepcopy(v);mutation(bad)
  try:validate_campaign(bad);rejected=False
  except (ValueError,KeyError,AssertionError):rejected=True
  check(name,rejected)
 c=copy.deepcopy(data['correctness']);c['rows']=[r for r in c['rows'] if not r['name'].endswith('-no-avx')];raw,_=inspect(c,data['measurements'],data['exports']);check('missing actual no-AVX executions rejected',not all(x['passed'] for x in raw))
 m=copy.deepcopy(data['measurements']);m['profiles'][-1]['accepted_for_timing']=True
 try:raw,_=inspect(data['correctness'],m,data['exports']);rejected=not all(x['passed'] for x in raw)
 except (ValueError,KeyError,StopIteration):rejected=True
 check('promoted invalid LTO scalar category rejected',rejected)
 prior={r['project']:r for r in load(HERE+'/preserved_inputs.json')['records']};support=load(SUPPORT);mapping=load('labs/numeric_followthrough/criteria.json')['criteria'];expected=set()
 for r in records():
  path='curriculum/v5/expansion/legacy_assessments/'+r['project']+'.json';old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path],cwd=ROOT,text=True));stripped=copy.deepcopy(r['criteria'])
  for c in stripped:c.pop('numeric_followthrough_support',None)
  check(r['project']+' preserved prior criterion providers',stripped==prior[r['project']]['criteria']);baseline=copy.deepcopy(stripped)
  for c in baseline:c.pop('successor_support',None)
  check(r['project']+' exact original objects',baseline==old['criteria']);check(r['project']+' retained bindings/status',all(r[k]==prior[r['project']][k] for k in ('baseline_binding','execution_support','status')) and r['baseline_binding']==old['baseline_binding'] and r['status']==old['status']);check(r['project']+' idempotent attach',attach(copy.deepcopy(r))==r)
  for c in r['criteria']:
   expected.add(c['id']);expected.update(x['id'] for x in c['variants']);row=next(x for x in mapping if x['id']==c['id']);check(c['id']+' exact task/expected/variants',row['task']==c['task'] and row['expected']==c['verification']['expected'] and row['variants']==c['variants'])
   for ident,reason in [(c['id'],NOTES[c['objective']][1]),(c['variants'][0]['id'],NOTES[c['objective']][2])]:check(ident+' exact derivation',next(x for x in support['updates'] if x['id']==ident)['closure_rationale']==reason)
 check('16 exact identities',{x['id'] for x in support['updates']}==expected and len(support['updates'])==16);pins(support['source_sha256']);check('source identities',True);check('independent gates retained',all(x['dimensions']['external_review']['state']=='not-run' and x['dimensions']['independent_learner']['state']=='not-assessed' and x['dimensions']['physical_observation']['state']=='not-run' and set(x['dimensions'])=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'} for x in support['updates']));check('author work accounted',support['remaining_author_work']==[]);check('concrete evidence',all((ROOT/p).is_file() for r in support['updates'] for d in r['dimensions'].values() for p in d['evidence']))
 from cmastery import assessments
 with tempfile.TemporaryDirectory(prefix='numeric-canonical-') as temporary:
  for r in records():
   dest=Path(temporary)/r['project'];assessments.export(ROOT,r['project'],dest);check(r['project']+' canonical export',all((dest/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*')))
 result=dict(passed=all(x['passed'] for x in checks),checks=checks,source_sha256={p:sha(ROOT/p) for p in (SUPPORT,CAMPAIGN,'tools/check_numeric_followthrough.py')},scope='All16 original P6 identities, exact retained providers and actual selected host/emulator observations; external independent gates retained.');dest=(ROOT/report).resolve();need(dest.is_relative_to(ROOT/'reviews/v5/expansion/legacy_numeric_followthrough') and not dest.exists(),'preserve historical owned receipt');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),failed=[x['name'] for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
