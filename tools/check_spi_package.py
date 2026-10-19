"""Keep original spi tasks, criterion-specific checks and external dimensions."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1];SUPPORT='research/v5/expansion/legacy_advanced_spi/completion_dispositions.json';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-04.json'
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def validate(d):
 e=[]
 def need(ok,s):
  if not ok:e.append(s)
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale '+p)
 need(bool(d.get('source_sha256')),'missing source pins');old=json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'));need(old['criteria']==read(PROJECT)['criteria'],'original criteria changed')
 expected={}
 for c in old['criteria']:
  if not c['objective'].startswith('T28.07'):continue
  for delayed in [False,True]:v=c['variants'][0];expected[v['id'] if delayed else c['id']]=(c['objective'],v['task'] if delayed else c['task'],v['id'],v['minimum_days'])
 rows=d.get('updates',[]);need(len(rows)==6 and {r['id'] for r in rows}==set(expected),'six exact rows')
 reports={p:read(p) for p in d.get('current_reports',[])}
 for p,r in reports.items():
  need(r.get('passed') is True,'failed '+p)
  for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(f)==h,'stale report input '+f)
 for r in rows:
  need((r.get('objective'),r.get('original_task'),r.get('original_delayed_variant'),r.get('minimum_days'))==expected.get(r['id']),'changed task/delay')
  dims=r.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for k,s in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:need(dims.get(k,{}).get('state')==s and not dims.get(k,{}).get('evidence'),'invented external evidence')
  av=r.get('author_validation',{});labels=av.get('checks',[]);key=r.get('objective','')[4:];mutants={'07.a':[1,3,7],'07.b':[2,4],'07.c':[5,6]}.get(key,[])
  need(len(mutants)>=2 and all('gcc-debug execute '+str(v) in labels for v in mutants) and 'gcc-debug independent oracle' in labels,'unrelated or missing specific evidence')
  if key=='07.a':need('gcc-debug composition execute' in labels,'provider comparison missing')
  if key=='07.a':need('measurement analysis' in labels and 'gcc-debug execute 7' in labels,'snapshot/timing evidence missing')
  if key=='07.b':need('gcc-debug target execute 3' in labels,'halfword DMA evidence missing')
  for label in labels:need(any(x.get('label')==label and x.get('passed') is True for x in reports.get(av.get('report'),{}).get('checks',[])),'unexecuted evidence selector')
  need(av.get('analysis')=='assessor/peripheral_depth/spi/ANALYSIS.md#'+key,'analysis mapping')
 need(bool(d.get('remaining_parents')),'scope silently erased');return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(SUPPORT);e=validate(d);controls=[]
 if a.self_test and not e:
  for kind in ['stale','missing-row','changed-task','removed-delay','invented-physical','invented-review','invented-learner','unrelated-evidence','missing-snapshot-evidence','wrong-analysis','erased-parent']:
   x=copy.deepcopy(d)
   if kind=='stale':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='missing-row':x['updates'].pop()
   elif kind=='changed-task':x['updates'][0]['original_task']='Implement spi'
   elif kind=='removed-delay':x['updates'][0]['minimum_days']=0
   elif kind.startswith('invented'):x['updates'][0]['dimensions'][{'invented-physical':'physical_observation','invented-review':'external_review','invented-learner':'independent_learner'}[kind]]['state']='passed'
   elif kind=='unrelated-evidence':x['updates'][0]['author_validation']['checks']=['gcc-debug execute 3']
   elif kind=='missing-snapshot-evidence':next(r for r in x['updates'] if r['objective']=='T28.07.a')['author_validation']['checks'].remove('measurement analysis')
   elif kind=='wrong-analysis':x['updates'][0]['author_validation']['analysis']='assessor/peripheral_depth/spi/ANALYSIS.md#07.c'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:e.append('accepted '+kind)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={p:sha(p) for p in [SUPPORT,'tools/check_spi_package.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not e,controls=len(controls),errors=e));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
