"""Regression: source gaps retain exact missing work and evidence limits."""
import copy,json
from reconcile_implementation_v5 import ROOT,add_source_requirements
def main():
 checks=[]
 def need(ok,name):checks.append(dict(check=name,passed=bool(ok)));assert ok,name
 empty=dict(source_states=[],requirements=[],completion_criteria=[],prerequisites=[],affected_consumers=[])
 cases=[('research/v5/expansion/legacy_advanced_diagnostic/source_access.json','gaps','G-DG-BOARD-ROUTE'),('research/v5/expansion/legacy_failure/source_access.json','gaps','G-FAILURE-EFFECTIVEC-LEAKS'),('research/v5/expansion/legacy_advanced_adc/source_access.json','reference_gaps','G-PA-F439-PA4-HEADER')]
 for path,collection,id in cases:
  row=next(r for r in json.loads((ROOT/path).read_text(encoding='utf8'))[collection] if r['id']==id);entry=copy.deepcopy(empty);add_source_requirements(entry,row,path)
  need(entry['source_states'][0]['status']==row['state'],id+' original state')
  if 'missing' in row or 'required' in row:need(row.get('missing',row.get('required')) in entry['requirements'],id+' exact missing requirement')
  if 'completion' in row:need(row['completion'] in entry['completion_criteria'],id+' exact completion rule')
  need(bool(entry['affected_consumers']),id+' affected consumers');need(bool(entry['source_access_attempts']),id+' access attempts')
  before=copy.deepcopy(entry);add_source_requirements(entry,row,path);need(before==entry,id+' deduplicated origins')
 entry=copy.deepcopy(empty);add_source_requirements(entry,dict(required=True,state='open',implemented='Compiles only'),'fixture')
 need(not entry['requirements'],'scope boolean not requirement');need(entry['existing_artifact_claims']==['Compiles only'] and 'dimensions' not in entry,'source claims never infer completed dimensions')
 path='research/v5/expansion/legacy_advanced_handoff/source_access.json';row=next(r for r in json.loads((ROOT/path).read_text())['gaps'] if r['id']=='XI-PHYSICAL');entry=copy.deepcopy(empty);add_source_requirements(entry,row,path)
 need(entry['prerequisites']==[row['dependency']],'exact physical prerequisite');need(entry['existing_artifact_claims']==[row['completed']],'emulator support not lost or promoted')
 entry=copy.deepcopy(empty);add_source_requirements(entry,dict(closure_limit='Selected source bytes only',limitations=['No physical execution']), 'fixture')
 need(set(entry['evidence_limits'])=={'Selected source bytes only','No physical execution'},'source evidence limits retained');need('dimensions' not in entry,'limit propagation does not close dimensions')
 out=ROOT/'reviews/v5/expansion/followthrough/inventory_source_fields.json';out.write_text(json.dumps(dict(passed=True,checks=checks),indent=2)+'\n');print(dict(passed=True,checks=len(checks)))
if __name__=='__main__':main()
