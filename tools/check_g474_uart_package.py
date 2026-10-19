"""Reject stale or overclaimed G474 successor support."""
from pathlib import Path
import argparse,copy,json
from run_g474_uart_followthrough import ROOT,sha
SUPPORT='research/v5/expansion/legacy_advanced_g474_uart/completion_dispositions.json'
def validate(d,files=True):
 e=[]
 def need(x,s):
  if not x:e.append(s)
 rows=d.get('updates',[]);need(len(rows)==3 and {r['id'] for r in rows}=={'G-PL-G474-FULL-PORT','G-PL-G474-FULL-PORT-IMPLEMENTATION','G-PL-G474-FULL-PORT-IMPLEMENTATION-DELAYED'},'exact parent/child IDs')
 need(bool(d.get('source_sha256')),'pins');need(set(d.get('remaining_parents',[]))=={'G-PA-RM0440-FULL','G-PL-G474-FULL-PORT-MANUAL','G-PL-G474-FULL-PORT-PHYSICAL'},'source/physical work retained')
 for r in rows:
  dimensions=r.get('dimensions',{});need(set(dimensions)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for k,s in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dimensions.get(k,{}).get('state')==s and not dimensions.get(k,{}).get('evidence'),'no invented external evidence')
  if r['id']=='G-PL-G474-FULL-PORT':need(dimensions.get('implementation',{}).get('state')=='partial' and r.get('external_blockers') and r.get('remaining_author_work'),'mixed parent')
  else:
   need(dimensions.get('implementation',{}).get('state')=='complete' and dimensions.get('author_validation',{}).get('state')=='passed' and not r.get('remaining_author_work'),'implemented children');need(r.get('objective')=='T28.11.c' and r.get('criterion')=='AX-PR-T28-06-T28.11.c','original objective linkage')
   if r['id'].endswith('DELAYED'):need(r.get('minimum_days')==7,'delay retained')
 if files:
  for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  for p in d.get('current_reports',[]):
   r=json.loads((ROOT/p).read_text());need(r.get('passed') is True,'failed current report')
   for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(ROOT/f)==h,'stale report input '+f)
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/legacy_g474_uart/package_validation.json');a=p.parse_args();d=json.loads((ROOT/SUPPORT).read_text());e=validate(d);controls=[]
 if a.self_test and not e:
  for kind in ['row','pins','parent','source','physical','review','learner','delay','link','implementation']:
   x=copy.deepcopy(d)
   if kind=='row':x['updates'].pop()
   elif kind=='pins':x['source_sha256']={}
   elif kind=='parent':x['updates'][2]['dimensions']['implementation']['state']='complete'
   elif kind=='source':x['remaining_parents']=[]
   elif kind in ['physical','review','learner']:x['updates'][0]['dimensions'][{'physical':'physical_observation','review':'external_review','learner':'independent_learner'}[kind]]['state']='passed'
   elif kind=='delay':x['updates'][1]['minimum_days']=0
   elif kind=='link':x['updates'][0]['objective']='T01.01.a'
   else:x['updates'][0]['remaining_author_work']=['missing code']
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:e.append('accepted '+kind)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={SUPPORT:sha(ROOT/SUPPORT),'tools/check_g474_uart_package.py':sha(Path(__file__))});(ROOT/a.report).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not e,controls=len(controls),errors=e));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
