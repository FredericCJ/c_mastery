"""Require current logs and all selected semantic discriminators."""
import argparse,copy,json,pathlib
from common import ROOT,sha
from run_posix_edition import PROFILES,MUTANTS
REPORT='research/v5/expansion/source_reconciliation/posix_validation.json'
def validate(d,files=True):
 errors=[]
 def need(ok,s):
  if not ok:errors.append(s)
 need(d.get('status')=='pass' and d.get('failure') is None,'campaign passed')
 commands=d.get('commands',[]);by={r['id']:r for r in commands}
 need(len(commands)==len(by)==99,'exact99 commands')
 for cc in ['gcc','clang']:
  for profile,_ in PROFILES:
   tag=cc+'.'+profile
   for case in ['close','read']:
    r=by.get(tag+'.'+case,{});need(r.get('exit_code')==r.get('expected_exit')==0 and r.get('required_marker')=='PE_'+case.upper(),'positive '+tag+case)
    r=by.get(tag+'.stub.'+case,{});need(r.get('exit_code')==r.get('expected_exit')==41,'stub '+tag+case)
   for mutant,case in MUTANTS:
    build=by.get(tag+'.mutant.'+mutant+'.build',{});r=by.get(tag+'.reject.'+mutant,{})
    need(build.get('exit_code')==build.get('expected_exit')==0,'mutant compiled '+tag+mutant)
    need(r.get('exit_code')==r.get('expected_exit')==41 and r.get('required_marker')=='PE_ASSERT:' and r.get('command',[])[-1:]==[case],'semantic mutant '+tag+mutant)
 for r in commands:
  need(r.get('passed') is True and r.get('exit_code')==r.get('expected_exit'),'consistent command '+r['id'])
  if files:
   p=ROOT/r['output'];need(p.is_file() and sha(p)==r['output_sha256'],'raw log '+r['id'])
   if p.is_file() and r.get('required_marker'):need(r['required_marker'] in p.read_text(),'marker '+r['id'])
 if files:
  for field in ['source_sha256','outputs_sha256']:
   need(bool(d.get(field)),'pins '+field)
   for p,h in d.get(field,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'current '+p)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/source_reconciliation/posix_guard.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);negative=[]
 if a.self_test and not errors:
  for name in ['missing-profile','missing-mutant','compile-rejection','wrong-oracle','missing-marker','failed-report']:
   b=copy.deepcopy(d)
   if name=='missing-profile':b['commands']=[r for r in b['commands'] if not r['id'].startswith('clang.sanitized')]
   elif name=='missing-mutant':b['commands']=[r for r in b['commands'] if r['id']!='gcc.debug.reject.RETRY']
   elif name=='compile-rejection':next(r for r in b['commands'] if r['id']=='gcc.debug.mutant.RETRY.build')['exit_code']=1
   elif name=='wrong-oracle':next(r for r in b['commands'] if r['id']=='gcc.debug.reject.RETRY')['expected_exit']=0
   elif name=='missing-marker':next(r for r in b['commands'] if r['id']=='gcc.debug.close')['required_marker']=None
   else:b['status']='fail'
   rejected=bool(validate(b,False));negative.append(dict(name=name,rejected=rejected));need=rejected
   if not need:errors.append('accepted '+name)
 out=dict(passed=not errors,errors=errors,negative_controls=negative,input_sha256={REPORT:sha(ROOT/REPORT)});dest=ROOT/a.report;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
