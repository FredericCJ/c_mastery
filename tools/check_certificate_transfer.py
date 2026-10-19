"""Source-bound positive/negative and strict-image evidence for two transfer gaps."""
import argparse,copy,functools,json,pathlib
import run_certificate_transfer as run
ROOT=run.ROOT;REPORT='research/v5/expansion/certificate_transfer/validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def text(p):return data(p).decode('utf8')
def validate(d,check_files=True):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('status')=='pass' and d.get('failure') is None,'successful final run')
 need(d.get('source_sha256')==run.pins(),'current source closure');need(d.get('external_sha256')==run.external(),'actual tool identities')
 if check_files:
  for n,h in d.get('outputs_sha256',{}).items():need((ROOT/n).is_file() and run.sha(ROOT/n)==h,'output '+n)
 rows=d.get('commands',[]);commands={r['id']:r for r in rows};need(len(rows)==len(commands)==250,'250 distinct required commands')
 def cmd(n):
  need(n in commands,'missing '+n);return commands.get(n,{})
 def args(n):return cmd(n).get('command',[])
 def result(n,code,marker):
  q=cmd(n);need(q.get('exit_code')==q.get('expected_exit')==code and q.get('required_marker')==marker,'result '+n)
 for q in rows:
  need(q.get('passed') is True and q.get('exit_code')==q.get('expected_exit'),'actual exit '+q['id']);p=ROOT/q.get('output','missing')
  if check_files:
   t=text(p) if p.is_file() else '';need(p.is_file() and run.sha(p)==q.get('output_sha256'),'raw log '+q['id']);need(q.get('required_marker') is None or q['required_marker'] in t,'raw marker '+q['id'])
 for cc in ['gcc','clang']:
  for mode,flags in run.PROFILES:
   tag=cc+'.'+mode;need(all(f in args(tag+'.build') for f in ['-std=c17','-Werror',*flags]),'native flags '+tag)
   for case in ['0','1']:result(tag+'.case'+case,0,'CT_PASS');result(tag+'.stub.reject'+case,41,'CT_ASSERT:')
   result(tag+'.independent',0,'"passed": true');result_path=pathlib.Path(args(tag+'.independent')[-1]);o=json.loads(text(result_path));need(o.get('passed') and o.get('seed')==360021 and o.get('cases')==2421 and o.get('accepted',0)>0 and o.get('rejected',0)>0,'nonvacuous independent tree oracle '+tag)
   for name,case,marker in run.MUTANTS:
    result(tag+'.reject.'+name,41,marker);need('-DCT_MUTANT_'+name in args(tag+'.mutant.'+name+'.build'),'compiled semantic mutant '+tag+name);need(cmd(tag+'.mutant.'+name+'.build').get('exit_code')==0,'mutant compiled '+tag+name)
   need(any(s.endswith('/learner.c') for s in args(tag+'.stub.build')),'unsolved learner '+tag)
  for mode in ['O0','O3']:
   for kind in ['iterative','recursive']:
    tag=cc+'.'+mode+'.cost.'+kind;need('-fstack-usage' in args(tag+'.compile'),'actual stack report '+tag)
    for suffix in ['size','disassembly','undefined']:cmd(tag+'.'+suffix)
 for cc in ['arm-none-eabi-gcc','clang']:
  for mode in ['O0','O3']:
   tag=cc+'.target.'+mode
   for i in range(8):
    need(all(f in args(tag+'.object'+str(i)) for f in ['-ffreestanding','-fno-builtin','-mcpu=cortex-m4']), 'strict object '+tag+str(i))
    for suffix in ['undefined','assembly']:cmd(tag+'.'+suffix+str(i))
   link=args(tag+'.link');need(link and link[0]=='arm-none-eabi-ld' and '--no-undefined' in link and len([s for s in link if s.endswith('.o')])==8 and not any(s.endswith('.a') or s.startswith('-l') for s in link),'direct original-object closure '+tag)
   if check_files:need(not text(ROOT/cmd(tag+'.undefined')['output']).strip(),'empty selected undefined '+tag)
   result(tag+'.execute',0,'CT_TARGET_PASS');need('mps2-an386' in args(tag+'.execute'),'actual MPS2 target '+tag)
 result('costs',0,'CT_COST_PASS profiles=4');cost_args=args('costs')
 if cost_args:
  cost=json.loads(text(pathlib.Path(cost_args[-1])));need(cost.get('passed') and len(cost.get('profiles',[]))==4,'measured resource table')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/certificate_transfer/guard.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for name in ['lost-command','failed-run','stale-source','stale-tool','missing-native','missing-mutant','compile-only-mutant','missing-stub','lost-oracle','weak-target','archive-runtime','lost-target-run','lost-costs']:
   bad=copy.deepcopy(d)
   def get(n):return next(r for r in bad['commands'] if r['id']==n)
   if name=='lost-command':bad['commands'].pop()
   elif name=='failed-run':bad['status']='fail'
   elif name.startswith('stale-'):k='source_sha256' if name=='stale-source' else 'external_sha256';bad[k][next(iter(bad[k]))]='0'*64
   elif name=='missing-native':get('gcc.O0.case0')['exit_code']=1
   elif name=='missing-mutant':get('gcc.O0.reject.EARLY')['exit_code']=0
   elif name=='compile-only-mutant':get('gcc.O0.mutant.EARLY.build')['exit_code']=1
   elif name=='missing-stub':get('gcc.O0.stub.reject0')['exit_code']=0
   elif name=='lost-oracle':get('gcc.O0.independent')['required_marker']=None
   elif name=='weak-target':get('clang.target.O0.object0')['command'].remove('-ffreestanding')
   elif name=='archive-runtime':get('clang.target.O0.link')['command'].append('libgcc.a')
   elif name=='lost-target-run':get('clang.target.O0.execute')['exit_code']=1
   elif name=='lost-costs':get('costs')['required_marker']=None
   rejected=bool(validate(bad,False));controls.append(dict(name=name,rejected=rejected))
   if not rejected:errors.append('accepted '+name)
 result=dict(passed=not errors,commands=len(d['commands']),input_sha256={REPORT:run.sha(ROOT/REPORT)},negative_controls=controls,errors=errors);out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
