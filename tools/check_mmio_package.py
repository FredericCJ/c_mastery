"""Reject lost original obligations, stale support and fabricated external claims."""
from pathlib import Path
import argparse,copy,hashlib,json
ROOT=Path(__file__).resolve().parents[1];SUPPORT='research/v5/expansion/legacy_advanced_mmio/completion_dispositions.json'
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def validate(d):
 e=[]
 def need(b,s):
  if not b:e.append(s)
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale '+p)
 original={}
 for c in read('curriculum/v5/expansion/legacy_assessments/PR-T28-01.json')['criteria']:
  original[c['id']]=(c['objective'],c['task']);original[c['variants'][0]['id']]=(c['objective'],c['variants'][0]['task'])
 rows=d.get('updates',[]);need(len(rows)==18 and {r['id'] for r in rows}==set(original),'eighteen exact rows')
 reports={p:read(p) for p in d.get('current_reports',[])}
 for p,r in reports.items():
  need(r.get('passed') is True,'failed '+p)
  for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(f)==h,'stale report '+f)
 for row in rows:
  need((row.get('objective'),row.get('original_task'))==original.get(row['id']),'original task changed')
  need(row.get('minimum_days')==7,'delay removed')
  dims=row.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for k,s in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dims.get(k,{}).get('state')==s and not dims.get(k,{}).get('evidence'),'external inference')
  av=row.get('author_validation',{});report=reports.get(av.get('report'),{})
  if row['objective']=='T28.02.b':need(av.get('kind')=='reasoning-only semantic counterexample' and not av.get('checks'),'reasoning substituted by narrow test')
  else:need(bool(av.get('checks')),'no mechanism evidence')
  for label in av.get('checks',[]):need(any(r.get('label')==label and r.get('passed') for r in report.get('checks',[])),'unexecuted selector')
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(SUPPORT);e=validate(d);controls=[]
 if a.self_test and not e:
  for k in ['stale','missing','changed-task','removed-delay','physical','learner','review','unexecuted','reasoning']:
   x=copy.deepcopy(d)
   if k=='stale':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif k=='missing':x['updates'].pop()
   elif k=='changed-task':x['updates'][0]['original_task']='Implement GPIO'
   elif k=='removed-delay':x['updates'][0]['minimum_days']=0
   elif k in ['physical','learner','review']:dim={'physical':'physical_observation','learner':'independent_learner','review':'external_review'}[k];x['updates'][0]['dimensions'][dim]['state']='passed'
   elif k=='reasoning':next(r for r in x['updates'] if r['objective']=='T28.02.b')['author_validation']['kind']='one narrow test proves all'
   else:x['updates'][0]['author_validation']['checks']=['unexecuted']
   rejected=bool(validate(x));controls.append(dict(control=k,rejected=rejected))
   if not rejected:e.append('accepted '+k)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={p:sha(p) for p in [SUPPORT,'tools/check_mmio_package.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=not e,controls=len(controls),errors=e)));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
