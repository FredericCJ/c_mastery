"""Require every declared reassembly competency and actual export boundary."""
import argparse,copy,json
from close_network_ip_exercise import ROOT,HERE,SUPPORT,REPORTS,SELECTORS,brief,sha
from research.v5.expansion.legacy_network_driver.integration import validate as validate_driver
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 need(bool(d.get('source_sha256')),'missing pins');m=brief();ids={m['id'],*[m['id']+'-'+v for v in m['delayed_transfer']['variant_ids']],*[r['id'] for r in m['rubric']]};rows=d.get('updates',[]);need(len(rows)==9 and {r.get('id') for r in rows}==ids,'nine distinct original exercise/variant/rubric IDs')
 errors.extend(validate_driver(rows,'NW-IP-DRIVER'))
 for r in rows:
  need(r.get('objective')=='T41.05.a' and r.get('minimum_days')==7,'original objective and delay');dims=r.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions');need(dims.get('implementation',{}).get('state')=='complete' and r.get('remaining_author_work')==[],'implementation state')
  for key,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dims.get(key,{})==dict(state=state,evidence=[]),'external evidence invented')
  key=r.get('id','').rsplit('-',1)[-1];expected=SELECTORS.get(key,list(dict.fromkeys(x for values in SELECTORS.values() for x in values)));need(r.get('author_validation')==dict(report=REPORTS[0],checks=expected),'criterion-specific evidence lost')
 need(d.get('current_reports')==REPORTS,'report set');reports=[json.loads((ROOT/p).read_text()) for p in REPORTS]
 for d0 in reports:
  need(d0.get('passed') is True,'failed report')
  for p,h in d0.get('input_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale report input '+p)
 names={'actual exercise init','compiling learner stub rejects','explicit author reference passes','source editing allowed','harness editing rejected'};checks=reports[2].get('checks',[]);need(len(checks)==5 and {r['label'] for r in checks}==names and all(r['passed'] for r in checks),'actual exercise boundaries')
 for r in checks:
  if 'command' in r:need(r['returncode']==r['expected']==(1 if r['label']=='compiling learner stub rejects' else 0),'actual CLI exit')
 need(bool(d.get('remaining_parents')),'parent gaps lost');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads(SUPPORT.read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  edits={'stale':lambda x:x['source_sha256'].update({next(iter(x['source_sha256'])):'0'*64}),'missing-row':lambda x:x['updates'].pop(),'duplicate':lambda x:x['updates'].__setitem__(0,x['updates'][1]),'lost-delay':lambda x:x['updates'][0].update(minimum_days=0),'wrong-objective':lambda x:x['updates'][0].update(objective='T41.05.b'),'lost-driver':lambda x:x['updates'][0].pop('driver_integration',None),'wrong-evidence':lambda x:x['updates'][-1]['author_validation'].update(checks=['gcc version']),'missing-export':lambda x:x['current_reports'].pop(),'lost-parents':lambda x:x.update(remaining_parents=[])}
  for key in ['external_review','independent_learner','physical_observation']:edits[key]=lambda x,k=key:x['updates'][0]['dimensions'][k].update(state='passed')
  for name,edit in edits.items():x=copy.deepcopy(d);edit(x);rejected=bool(validate(x));controls.append(dict(control=name,rejected=rejected));errors+=[] if rejected else ['accepted '+name]
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [SUPPORT,ROOT/'tools/check_network_ip_exercise_package.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
