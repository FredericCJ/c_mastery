"""Exact original tasks, specific evidence and six independent status dimensions."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess
from close_interrupt_time_depth import SELECTORS,PROJECTS,project
ROOT=Path(__file__).resolve().parents[1];SUPPORT='research/v5/expansion/legacy_advanced_time/completion_dispositions.json'
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def validate(d):
 errors=[]
 def need(x,msg):
  if not x:errors.append(msg)
 need(bool(d.get('source_sha256')),'source pins absent')
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale '+p)
 expected={}
 for name in PROJECTS:
  old=json.loads(subprocess.check_output(['git','show','HEAD:'+project(name)],cwd=ROOT,text=True,encoding='utf8'));now=read(project(name));need(old['criteria']==now['criteria'] and old['baseline_binding']==now['baseline_binding'],'baseline changed')
  for c in old['criteria']:
   for delayed in [False,True]:v=c['variants'][0];expected[v['id'] if delayed else c['id']]=(c['objective'],v['task'] if delayed else c['task'],v['id'],v['minimum_days'])
 rows=d.get('updates',[]);need(len(rows)==12 and {r['id'] for r in rows}==set(expected),'12 exact rows')
 reports={p:read(p) for p in d.get('current_reports',[])};need(len(reports)==3,'three required report dimensions')
 for p,r in reports.items():
  need(r.get('passed') is True,'failed '+p)
  for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(f)==h,'stale report input '+f)
 for row in rows:
  need((row.get('objective'),row.get('original_task'),row.get('original_delayed_variant'),row.get('minimum_days'))==expected.get(row['id']),'changed task/delay')
  dims=row.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for key,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dims.get(key,{}).get('state')==state and not dims.get(key,{}).get('evidence'),'invented external evidence')
  av=row.get('author_validation',{});labels=av.get('checks',[]);key=row.get('objective','')[4:];need(bool(SELECTORS.get(key)) and all(v in labels for v in SELECTORS.get(key,[])),'missing mechanism/transfer evidence')
  for label in labels:need(any(r.get('label')==label and r.get('passed') is True for r in reports.get(av.get('report'),{}).get('checks',[])),'unexecuted evidence')
  need(av.get('analysis')=='assessor/interrupt_depth/time/ANALYSIS.md#'+key,'wrong technical analysis')
 need(bool(d.get('remaining_parents')),'scope erased');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(SUPPORT);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['stale','missing-row','changed-task','removed-delay','invented-physical','invented-review','invented-learner','unrelated-evidence','missing-transfer','wrong-analysis','erased-parent','missing-export']:
   x=copy.deepcopy(d)
   if kind=='stale':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='missing-row':x['updates'].pop()
   elif kind=='changed-task':x['updates'][0]['original_task']='Implement a queue'
   elif kind=='removed-delay':x['updates'][0]['minimum_days']=0
   elif kind.startswith('invented-'):x['updates'][0]['dimensions'][{'invented-physical':'physical_observation','invented-review':'external_review','invented-learner':'independent_learner'}[kind]]['state']='passed'
   elif kind=='unrelated-evidence':x['updates'][0]['author_validation']['checks']=['gcc version']
   elif kind=='missing-transfer':next(r for r in x['updates'] if r['objective']=='T29.09.b')['author_validation']['checks'].remove('gcc-debug 2 execute')
   elif kind=='wrong-analysis':x['updates'][0]['author_validation']['analysis']='wrong'
   elif kind=='missing-export':x['current_reports']=[p for p in x['current_reports'] if not p.endswith('exports.json')]
   else:x['remaining_parents']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(p) for p in [SUPPORT,'tools/check_interrupt_time_package.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
