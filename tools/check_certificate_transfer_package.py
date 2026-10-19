"""Exact original-task and current-source guard for T36 completion bindings."""
import argparse,copy,json,pathlib
from common import ROOT,sha
from close_certificate_transfer import BASE,IDS,REPORTS,PROVIDERS,read,rows
SUPPORT=BASE+'/completion_dispositions.json'
def validate(d,files=True):
 errors=[]
 def need(ok,s):
  if not ok:errors.append(s)
 need(d.get('updates')==rows() and len(d.get('updates',[]))==16,'16 exact original task/dimension/evidence rows')
 need(d.get('source_gap_ids')==['G-CONTRACT-C17'] and len(d.get('external_evidence',[]))==2,'remaining external/source boundaries')
 for row in d.get('updates',[]):
  for n in row.get('artifacts',[]):need(n in d.get('source_sha256',{}),'bound artifact '+n)
 if files:
  for n,h in d.get('source_sha256',{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current source '+n)
  for name in IDS:
   before=json.loads((ROOT/BASE/'history'/(name+'.before.md')).read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0]);current=json.loads((ROOT/'curriculum/v5/exercises'/(name+'.md')).read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0])
   for k in ['id','project','objectives','rubric','delayed_transfer','objective_coverage']:need(current[k]==before[k],'historical field '+name+k)
  reports=list(REPORTS.values())+[BASE+'/exports.json']+[str(pathlib.PurePosixPath(p).parent/n) for p in PROVIDERS.values() for n in ['validation.json','exports.json']]
  for n in reports:
   r=read(n);need(r.get('status') in ['pass','passed'] and all(q.get('passed') is True for q in r['commands']),'actual executed report '+n)
   for k in ['source_sha256','outputs_sha256','inspected_paths']:
    for p,h in r.get(k,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'current report input/output '+p)
   for p,h in r.get('external_sha256',{}).items():need(pathlib.Path(p).is_file() and sha(pathlib.Path(p))==h,'actual tool '+p)
  gate=read(BASE+'/research_gate.json')
  for p,h in gate['source_sha256'].items():need(sha(ROOT/p)==h,'actual passage '+p)
  for p,h in gate['external_source_sha256'].items():need(pathlib.Path(p).is_file() and sha(pathlib.Path(p))==h,'inspected CLRS capture '+p)
  import sys
  sys.path.insert(0,str(ROOT))
  from cmastery import exercises,learning
  e=read(BASE+'/exports.json');need(e.get('selected_catalog_sha256')=={ident:learning.digest(exercises.exercise(ident)) for ident in IDS},'current selected exercise contracts')
  need(len(e['commands'])==12 and len(e['exports'])==3 and all(not r['private_sources_copied'] for r in e['exports']),'three separated actual exports')
  for q in e['commands']:
   p=ROOT/q['output'];s=p.read_text();need(sha(p)==q['output_sha256'] and q['exit_code']==q['expected_exit'] and(q['required_marker'] is None or q['required_marker'] in s),'actual export log '+q['id'])
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/certificate_transfer/package_guard.json');a=p.parse_args();d=read(SUPPORT);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for n in ['lost-root','lost-rubric','lost-variant','changed-task','changed-delay','lost-binding','wrong-provider','false-review','false-learner','false-physical','lost-source-gap','lost-artifact']:
   bad=copy.deepcopy(d)
   if n in ['lost-root','lost-rubric','lost-variant']:bad['updates'].pop({'lost-root':0,'lost-rubric':1,'lost-variant':4}[n])
   elif n=='changed-task':bad['updates'][4]['original_task']='generic changed constant'
   elif n=='changed-delay':bad['updates'][4]['minimum_days']=0
   elif n=='lost-binding':bad['updates'][0]['assessment_bindings']=[]
   elif n=='wrong-provider':bad['updates'][0]['assessment_bindings'][0]['report']='wrong.json'
   elif n.startswith('false-'):key={'false-review':'external_review','false-learner':'independent_learner','false-physical':'physical_observation'}[n];bad['updates'][0]['dimensions'][key]['state']='passed'
   elif n=='lost-source-gap':bad['source_gap_ids']=[]
   else:bad['source_sha256'].pop(bad['updates'][0]['artifacts'][0])
   rejected=bool(validate(bad,False));controls.append(dict(name=n,rejected=rejected))
   if not rejected:errors.append('accepted '+n)
 result=dict(passed=not errors,rows=len(d['updates']),input_sha256={SUPPORT:sha(ROOT/SUPPORT)},negative_controls=controls,errors=errors);out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
