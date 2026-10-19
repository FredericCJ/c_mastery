"""Guard exact retained tasks, current evidence, exported separation and six dimensions."""
import argparse,copy,json
from close_integer_kernel import ROOT,SUPPORT,CARD,REPORTS,SELECTORS,original,read,sha
def validate(d):
 errors=[]
 def need(x,why):
  if not x:errors.append(why)
 need(bool(d.get('source_sha256')),'source pins missing')
 for p,h in d.get('source_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 old=original();now=read(ROOT/CARD);need(old['criteria']==now['criteria'] and old['baseline_binding']==now['baseline_binding'],'historical identity changed')
 expected={}
 for c in old['criteria']:
  for delayed in [False,True]:
   v=c['variants'][0];expected[v['id'] if delayed else c['id']]=(c['objective'],v['task'] if delayed else c['task'],v['id'],v['minimum_days'])
 rows=d.get('updates',[]);need(len(rows)==24 and {r['id'] for r in rows}==set(expected),'twenty-four exact task identities')
 need(d.get('current_reports')==REPORTS,'mandatory reports')
 reports={p:read(ROOT/p) for p in d.get('current_reports',[])}
 for p,r in reports.items():
  need(r.get('passed') is True,'failed '+p)
  for f,h in r.get('input_sha256',{}).items():need((ROOT/f).is_file() and sha(ROOT/f)==h,'stale report input '+f)
 for r in rows:
  need((r.get('objective'),r.get('original_task'),r.get('original_delayed_variant'),r.get('minimum_days'))==expected.get(r['id']),'exact task/delay')
  dims=r.get('dimensions',{});need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions')
  for key,state in [('teaching','complete'),('implementation','complete'),('author_validation','passed'),('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-run')]:
   need(dims.get(key,{}).get('state')==state,'dimension state '+key)
   need(bool(dims.get(key,{}).get('evidence'))==(key in ['teaching','implementation','author_validation']),'dimension evidence '+key)
  av=r.get('author_validation',{});labels=av.get('command_groups',[]);required=SELECTORS.get(r.get('objective','')[4:],[])
  need(bool(required) and all(label in labels for label in required),'criterion-specific checks')
  for label in labels:need(any(x.get('group')==label and x.get('passed') is True for x in reports.get(av.get('report'),{}).get('commands',[])),'unexecuted check '+label)
  need(av.get('analysis')=='assessor/integer_kernel/ANALYSIS.md#'+r.get('objective','').lower().replace('.',''),'criterion-specific analysis')
  need(r.get('remaining_author_work')==[] and bool(r.get('evidence_limits')),'scope or limits missing')
 export=reports.get(REPORTS[-1],{});cmds=export.get('commands',[])
 wanted={'actual init':(0,[]),'header dependency identity':(0,[])}
 from run_integer_kernel import MARKERS,STUB
 wanted.update({'target build':(0,['FI_TARGET_BUILD_PASS explicit_objects=4 no_archives']),'target execute':(0,['FI_TARGET_PASS manual_arithmetic no_wider_guards size32_input64 logical_offsets wire_octets']),'target closure':(0,['"passed": true'])})
 wanted.update({cc+' '+str(ref):(int(not ref),MARKERS if ref else [STUB]) for cc in ['gcc','clang'] for ref in [False,True]})
 need(len(cmds)==9 and {r.get('label') for r in cmds}==set(wanted) and not export.get('errors'),'actual export scope')
 for r in cmds:
  code,markers=wanted.get(r.get('label'),(-9,[]));need(r.get('passed') is True and r.get('returncode')==r.get('expected')==code and r.get('markers')==markers and all(m in r.get('stdout','')+r.get('stderr','') for m in markers),'actual export outcome')
 need(bool(d.get('remaining_parents')),'erased parent work');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_integer_kernel/package_validation.json');a=p.parse_args();d=read(ROOT/SUPPORT);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['stale','missing-row','changed-task','removed-delay','invented-review','invented-learner','invented-physical','unrelated-evidence','missing-transfer','wrong-analysis','missing-export','erased-parent']:
   x=copy.deepcopy(d)
   if kind=='stale':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
   elif kind=='missing-row':x['updates'].pop()
   elif kind=='changed-task':x['updates'][0]['original_task']='Implement unrelated queue'
   elif kind=='removed-delay':x['updates'][0]['minimum_days']=0
   elif kind.startswith('invented-'):x['updates'][0]['dimensions'][{'invented-review':'external_review','invented-learner':'independent_learner','invented-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='unrelated-evidence':x['updates'][0]['author_validation']['command_groups']=['unrelated']
   elif kind=='missing-transfer':next(r for r in x['updates'] if r['objective']=='T04.03.c')['author_validation']['command_groups'].remove('gcc:c17:True')
   elif kind=='wrong-analysis':x['updates'][0]['author_validation']['analysis']='wrong'
   elif kind=='missing-export':x['current_reports'].pop()
   else:x['remaining_parents']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [SUPPORT,'tools/check_integer_kernel_package.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
