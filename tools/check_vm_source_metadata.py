"""Reject inspection expansion, eligibility promotion and private assignment regression."""
import argparse,copy,json,sys
from pathlib import Path
from repair_vm_source_metadata import ROOT,BASE,PACKAGES,resources,read,sha,write
sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_vm_representation.author_support import attach

def require(ok,message):
 if not ok:raise ValueError(message)

def compare_resource(old,now):
 expected=copy.deepcopy(old)
 for row in expected['resources']:row['inspection']['scope']=copy.deepcopy(row['locators'])
 require(now==expected,'scope repair must not alter eligibility, locators, date, captures or limits')

def card_check(card):
 expected=attach(copy.deepcopy(card));require(expected==card,'attachment not idempotent/current')
 for source in card['source_records']:
  registry=read(source['path']);row=next(r for r in registry['resources'] if r['id']==source['id'])
  require(row.get('assignable') is not False,'author-only inspection assigned to learner')
  ins=row['inspection'];require(all(ins.get(k) for k in ['date','method','scope','limits']),'incomplete inspection schema')
  require(set(source['passages'])<=set(row['locators']),'uninspected passage assigned')

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();require(not (ROOT/a.output).exists(),'preserve report history');history=read(BASE+'/history_01/manifest.json');inputs={};count=0;private=0
 for path,row in history.items():require(sha(row['archive'])==row['sha256'],'historical bytes changed')
 for path in resources():
  old=read(history[path]['archive']);now=read(path);compare_resource(old,now);inputs[path]=sha(path)
  for r in now['resources']:
   count+=1;private+=r.get('assignable') is False
   for source,h in r['inspection']['source_sha256'].items():require(sha(source)==h,'captured/inspected provider source changed');inputs[source]=h
  gate_path=str(Path(path).with_name('research_gate.json')).replace('\\','/');gate=read(gate_path)
  for source,h in gate['research_input_sha256'].items():require(sha(source)==h,'stale research gate');inputs[source]=h
  inputs[gate_path]=sha(gate_path)
 projects=[]
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T46-*.json')):
  relative=path.relative_to(ROOT).as_posix();card=read(relative);old=read(history[relative]['archive']);require(card['criteria']==old['criteria'] and card['baseline_binding']==old['baseline_binding'],'criterion/baseline changed');card_check(card);projects.append(relative);inputs[relative]=sha(relative)
 for name in PACKAGES:
  report='reviews/v5/expansion/legacy_vm_'+name+'/current_validation.json';current=read(report);require(current['passed'],'original campaign failed');require(sha(report)==history[report]['sha256'],'original C execution envelope rewritten')
  for source,h in current['input_sha256'].items():require(sha(source)==h,'original C campaign input changed');inputs[source]=h
  inputs[report]=sha(report)
 validation='research/v5/expansion/legacy_diagnostics/validation.json';v=read(validation);require(v['status']=='passed' and v['counts']['projects']==92 and v['counts']['criteria']==288 and all(r['status']=='passed' for r in v['checks']),'diagnostics consumer failed');inputs[validation]=sha(validation)
 controls=[]
 if a.self_test:
  old=read(history[resources()[1]]['archive']);now=read(resources()[1])
  for kind in ['scope-expansion','eligibility-promotion','date-rewrite','capture-rewrite','private-assignment','missing-source']:
   altered=copy.deepcopy(now);card=read('curriculum/v5/expansion/legacy_assessments/PR-T46-02.json')
   if kind=='scope-expansion':altered['resources'][0]['inspection']['scope'].append('entire codebase')
   elif kind=='eligibility-promotion':altered['resources'][1]['assignable']=True
   elif kind=='date-rewrite':altered['resources'][0]['inspection']['date']='future reading'
   elif kind=='capture-rewrite':altered['resources'][0]['inspection']['source_sha256']={}
   elif kind=='private-assignment':card['source_records'].append(dict(id='R-VV-CVM-ORIGINAL',path=resources()[1],passages=now['resources'][1]['locators']))
   else:card['source_records']=[r for r in card['source_records'] if r['id']!='R-VV-CLANG-DATAFLOW']
   try:compare_resource(old,altered);card_check(card)
   except (ValueError,KeyError,StopIteration):controls.append(dict(control=kind,rejected=True))
   else:raise ValueError('guard accepted '+kind)
 for source in [BASE+'/history_01/manifest.json',BASE+'/normalization.json','tools/repair_vm_source_metadata.py','tools/check_vm_source_metadata.py','research/v5/expansion/legacy_vm_representation/author_support.py']:inputs[source]=sha(source)
 result=dict(passed=True,scope='Metadata schema and learner-source eligibility repair only; no original C campaign repeated or source-reading scope expanded.',resource_entries=count,author_only_retained=private,projects=len(projects),original_C_campaigns_unchanged=len(PACKAGES),negative_controls=controls,input_sha256=inputs,remaining=['External review, learner delay, physical observations and exact ISO source access remain governed by each original package.']);write(a.output,result);print(json.dumps({k:v for k,v in result.items() if k!='input_sha256'}))
if __name__=='__main__':main()
