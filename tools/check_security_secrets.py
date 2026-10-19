"""Exact token-copy/work/provider evidence and non-inflation controls."""
import argparse,copy,functools,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_secrets'
from research.v5.expansion.legacy_security_commands.author_support import attach
import run_security_secrets as runner
from close_security_secrets import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
@functools.lru_cache(None)
def contents(p,size,mtime):return p.read_bytes()
def data(p):
 s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 c=read(canon());b=read(HERE/'PR-T35-05.before-secrets.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent maintained attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {x.get('id') for x in rows}==set(expected),'exact three criteria and three delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'full source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure');need(r.get('external_sha256')==runner.external(),'actual selected provider/tool/header identities')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact passing command inventory');logs={}
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[x['id']]=text;need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id'])
 def command(name,exit,marker=None):
  x=commands.get(name,{});need(x.get('exit_code')==exit and (marker is None or x.get('required_marker')==marker),'intended operation '+name)
 for cc in ['gcc','clang']:
  command(cc+'.c23.execute',0);command(cc+'.c17.unavailable',1,'memset_explicit')
  need('-std=c23' in commands.get(cc+'.c23.compile',{}).get('command',[]),'actual edition mode')
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile
   for m,marker in [('EARLY','out.work==4 && out.equal==(k==2)'),('CLEAR','p.bytes[i]==0')]:
    command(tag+'.mutation.'+m,41,marker);need('-DSS_MUTANT_'+m in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutant definition')
   for case in runner.CASES:command(tag+'.stub.'+case,41,'SS_ASSERT:')
   command(tag+'.provider.stub',41,'SS_ASSERT:');command(tag+'.provider.mutant',41,'ledger.key_cleared==1');need('-DSS_MUTANT_KEY' in commands.get(tag+'.provider.mutant.compile',{}).get('command',[]),'intended key-clear mutation')
   args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual baseline/profile '+tag)
   pargs=commands.get(tag+'.provider.compile',{}).get('command',[]);need('-DSS_FIRST_PRINCIPLES' in pargs and any(x.endswith('/assessor/replicated/hashing/hashing.c') for x in pargs) and any(x.endswith('/libcrypto.so.3') for x in pargs),'actual distinct oracle/provider linkage '+tag)
   oracle=json.loads(logs.get(tag+'.provider.oracle','{}'));need(oracle.get('status')=='pass' and oracle.get('provider_vectors')==65,'actual tag oracle '+tag)
   tags=re.findall(r'^TAG (\d+) ([0-9a-f]{64})$',logs.get(tag+'.provider',''),re.M);need(len(tags)==65 and [int(n) for n,_ in tags]==list(range(65)),'actual all message lengths '+tag)
 command('musl.c23.unavailable',1,'memset_explicit')
 obs=r.get('observations',[]);need(len(obs)==6 and {o.get('profile') for o in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six exact profiles')
 for o in obs:
  tag=o.get('profile','missing');assembly=logs.get(tag+'.disassembly','')
  for key,function in [('ordinary_clear_disassembly','ss_dead_store'),('explicit_clear_disassembly','ss_explicit_store'),('indexed_disassembly','ss_indexed')]:
   match=re.search('<'+function+r'>:\n(.*?)(?:\n\n|$)',assembly,re.S);need(bool(match) and o.get(key)==match[1],'actual function instructions '+tag+key)
  symbols=[line.split()[-1] for line in logs.get(tag+'.undefined','').splitlines()];need(o.get('component_undefined')==symbols,'actual object dependencies '+tag)
  allowed={'ss_wipe'} if not tag.endswith('sanitized') else {'ss_wipe','__start_asan_globals','__stop_asan_globals'}|{x for x in symbols if x.startswith(('__asan_','__ubsan_'))};need(set(symbols)<=allowed,'profile-specific helper boundary '+tag)
  need(o.get('cases')==runner.CASES and o.get('changed_bytes')==1024 and o.get('early_work')==[1,4,4] and o.get('fixed_work')==[4,4,4] and o.get('address_count')==8 and o.get('address_trace_leaks') is True,'exact work/address observations '+tag)
  need(o.get('provider_vectors')==65 and o.get('provider_faults')==7 and o.get('provider_owner_key_preserved') is True,'exact provider lifetime observations '+tag)
  need(o.get('machine_constant_time') is False and o.get('production_approval') is False,'non-inflated claim boundary '+tag)
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==7 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source status')
 for name,h in gate['research_input_sha256'].items():need(sha(ROOT/name)==h,'source gate '+name)
 for resource in read(HERE/'resources.json')['resources']:
  for name,h in resource['inspection'].get('external_source_sha256',{}).items():
   p=pathlib.Path(name);need(p.is_file() and sha(p)==h,'inspected source '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'bounded artifact claims')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T35-05' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','constant-time','production-approval','lost-provider-fault','lost-independent-oracle','lost-address-leak','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.provider.mutant']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.provider.mutant')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='constant-time':y['observations'][0]['machine_constant_time']=True
   elif kind=='production-approval':y['observations'][0]['production_approval']=True
   elif kind=='lost-provider-fault':y['observations'][0]['provider_faults']=6
   elif kind=='lost-independent-oracle':next(q for q in y['commands'] if q['id']=='gcc.debug.provider.compile')['command'].remove('-DSS_FIRST_PRINCIPLES')
   elif kind=='lost-address-leak':y['observations'][0]['address_trace_leaks']=False
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
