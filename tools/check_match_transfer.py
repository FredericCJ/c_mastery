"""Reject omitted, stale or vacuous streaming callback composition evidence."""
import argparse,copy,json,pathlib
import run_match_transfer as run
ROOT=run.ROOT;REPORT='research/v5/expansion/match_transfer/validation.json'
def validate(d,files=True):
 e=[]
 def need(ok,s):
  if not ok:e.append(s)
 need(d.get('status')=='pass' and d.get('failure') is None,'successful final run');need(d.get('source_sha256')==run.pins(),'current sources');need(d.get('external_sha256')==run.common.external(),'current tools')
 rows=d.get('commands',[]);commands={r['id']:r for r in rows};need(len(rows)==len(commands)==181,'181 commands')
 if files:
  for n,h in d.get('outputs_sha256',{}).items():need((ROOT/n).is_file() and run.common.sha(ROOT/n)==h,'output '+n)
 def cmd(n):need(n in commands,'missing '+n);return commands.get(n,{})
 def args(n):return cmd(n).get('command',[])
 def result(n,code,marker):q=cmd(n);need(q.get('exit_code')==q.get('expected_exit')==code and q.get('required_marker')==marker,'result '+n)
 for q in rows:
  need(q.get('passed') and q.get('exit_code')==q.get('expected_exit'),'actual exit '+q['id'])
  if files:
   p=ROOT/q.get('output','missing');s=p.read_text() if p.is_file() else '';need(p.is_file() and run.common.sha(p)==q.get('output_sha256'),'log '+q['id']);need(q.get('required_marker') is None or q['required_marker'] in s,'marker '+q['id'])
 for cc in ['gcc','clang']:
  for mode,flags in run.PROFILES:
   t=cc+'.'+mode;need(all(f in args(t+'.build') for f in ['-std=c17','-Werror',*flags]),'flags '+t)
   for case,count in [('0',40),('1',6),('2',4815)]:
    result(t+'.case'+case,0,'MT_PASS');result(t+'.stub.reject'+case,41,'MT_ASSERT:')
    if files:need('cases='+str(count) in (ROOT/cmd(t+'.case'+case)['output']).read_text(),'nonvacuous corpus '+t+case)
   for name,case,marker in run.MUTANTS:
    result(t+'.reject.'+name,41,marker);need('-DMT_MUTANT_'+name in args(t+'.mutant.'+name+'.build'),'actual mutant '+t+name);need(cmd(t+'.mutant.'+name+'.build').get('exit_code')==0,'mutant compiled '+t+name)
   need(any(a.endswith('/learner.c') for a in args(t+'.stub.build')),'actual stub '+t)
   need(any(a.endswith('/algorithm_lookup/lookup.c') for a in args(t+'.build')) and any(a.endswith('/algorithm_depth/matching.c') for a in args(t+'.build')),'actual prior providers '+t)
 for cc in ['arm-none-eabi-gcc','clang']:
  for mode in ['O0','O3']:
   t=cc+'.target.'+mode
   for i in range(6):
    need(all(f in args(t+'.object'+str(i)) for f in ['-ffreestanding','-fno-builtin','-fstack-usage','-mcpu=cortex-m4']),'strict object '+t+str(i));cmd(t+'.undefined'+str(i));cmd(t+'.assembly'+str(i))
   a=args(t+'.link');need(a and a[0]=='arm-none-eabi-ld' and '--no-undefined' in a and len([v for v in a if v.endswith('.o')])==6 and not any(v.endswith('.a') or v.startswith('-l') for v in a),'direct original closure '+t)
   if files:need(not (ROOT/cmd(t+'.undefined')['output']).read_text().strip(),'no unresolved selected image '+t)
   result(t+'.execute',0,'MT_TARGET_PASS');need('mps2-an386' in args(t+'.execute'),'actual target '+t)
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/match_transfer/guard.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for n in ['lost-command','stale-source','stale-tool','lost-retry','lost-cancel','lost-enumeration','lost-stub','compile-failure-mutant','omitted-provider','weak-target','archive-target','unexecuted-target']:
   bad=copy.deepcopy(d)
   def get(i):return next(q for q in bad['commands'] if q['id']==i)
   if n=='lost-command':bad['commands'].pop()
   elif n.startswith('stale-'):key='source_sha256' if n=='stale-source' else 'external_sha256';bad[key][next(iter(bad[key]))]='0'*64
   elif n in ['lost-retry','lost-cancel','lost-enumeration']:get('gcc.O0.case'+str(['lost-retry','lost-cancel','lost-enumeration'].index(n)))['exit_code']=1
   elif n=='lost-stub':get('gcc.O0.stub.reject0')['exit_code']=0
   elif n=='compile-failure-mutant':get('gcc.O0.mutant.SKIP.build')['exit_code']=1
   elif n=='omitted-provider':get('gcc.O0.build')['command']=[v for v in get('gcc.O0.build')['command'] if not v.endswith('/algorithm_depth/matching.c')]
   elif n=='weak-target':get('clang.target.O0.object0')['command'].remove('-ffreestanding')
   elif n=='archive-target':get('clang.target.O0.link')['command'].append('libgcc.a')
   else:get('clang.target.O0.execute')['exit_code']=1
   rejected=bool(validate(bad,False));controls.append(dict(name=n,rejected=rejected))
   if not rejected:errors.append('accepted '+n)
 result=dict(passed=not errors,commands=len(d['commands']),input_sha256={REPORT:run.common.sha(ROOT/REPORT)},negative_controls=controls,errors=errors);out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
