"""Check complete original T27 bindings without conflating external evidence."""
from pathlib import Path
import argparse,hashlib,json,copy
ROOT=Path(__file__).resolve().parents[1]
SUPPORT='research/v5/expansion/legacy_advanced_bringup/completion_dispositions.json'
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def validate(d):
 e=[]
 def need(ok,s):
  if not ok:e.append(s)
 need(bool(d.get('source_sha256')),'source bindings missing')
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale '+p)
 original={}
 for path in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T27-*.json'):
  for c in json.loads(path.read_text(encoding='utf8'))['criteria']:
   original[c['id']]=(c['objective'],c['task']);original[c['variants'][0]['id']]=(c['objective'],c['variants'][0]['task'])
 rows=d.get('updates',[]);need(len(rows)==60 and {r['id'] for r in rows}==set(original),'exact sixty obligations')
 cases={c['criterion']:c for c in read('assessor/bringup_depth/assessment_cases.json')['cases']}
 reports={p:read(p) for p in d.get('current_reports',[])}
 for p,r in reports.items():
  need(r.get('passed') is True,'failed '+p)
  for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(f)==h,'stale report input '+f)
 for r in rows:
  need((r.get('objective'),r.get('original_task'))==original.get(r['id']),'original obligation changed')
  need(r.get('minimum_days')==7,'delay removed')
  dims=r.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for k,s in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dims.get(k,{}).get('state')==s and not dims.get(k,{}).get('evidence'),'external claim invented')
  case=cases.get(r['id'].removesuffix('-DELAYED'),{});need(bool(case.get('inputs')) and bool(case.get('ordinary_expected')) and bool(case.get('changed_expected')),'missing assessor case')
  av=r.get('author_validation',{});need(bool(av.get('checks')),'missing specific checks')
  for c in av.get('checks',[]):need(any(x.get('label')==c['check'] and x.get('passed') is True for x in reports.get(c['report'],{}).get('checks',[])),'invalid evidence selector')
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(SUPPORT);e=validate(d);controls=[]
 if a.self_test and not e:
  for kind in ['stale','missing-row','missing-analysis','changed-task','removed-delay','invented-physical','invented-learner','invented-review','missing-evidence','wrong-selector']:
   x=copy.deepcopy(d)
   if kind=='stale':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='missing-row':x['updates'].pop()
   elif kind=='missing-analysis':x['updates'][0]['id']='unregistered'
   elif kind=='changed-task':x['updates'][0]['original_task']='Implement startup'
   elif kind=='removed-delay':x['updates'][0]['minimum_days']=0
   elif kind.startswith('invented'):k={'invented-physical':'physical_observation','invented-learner':'independent_learner','invented-review':'external_review'}[kind];x['updates'][0]['dimensions'][k]['state']='passed'
   elif kind=='missing-evidence':x['updates'][0]['author_validation']['checks']=[]
   else:x['updates'][0]['author_validation']['checks'][0]['check']='unexecuted test'
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:e.append('accepted '+kind)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={s:sha(s) for s in [SUPPORT,'tools/check_bringup_package.py']},limits='Author package integrity, not an external review or learner assessment')
 (ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=out['passed'],controls=len(controls),errors=e)));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
