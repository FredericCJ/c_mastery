"""Exact artifact-policy/service evidence and non-inflation controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_artifacts'
from research.v5.expansion.legacy_security_commands.author_support import attach
import run_security_artifacts as runner
from close_security_artifacts import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
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
 c=read(canon());b=read(HERE/'PR-T35-06.before-artifacts.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent maintained attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {x.get('id') for x in rows}==set(expected),'exact six criteria and six delayed identities')
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
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile
   for case in runner.CASES:command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'SA_ASSERT:')
   for mutant,(_,marker) in runner.MUTANTS.items():command(tag+'.mutation.'+mutant,41,marker);need('-DSA_MUTANT_'+mutant in commands.get(tag+'.mutation.'+mutant+'.compile',{}).get('command',[]),'intended mutant definition')
   for f in range(1,5):command(tag+'.failure.'+str(f),0,'OBS provider_failure');need('-DSA_PROVIDER_FAILURE='+str(f) in commands.get(tag+'.failure.'+str(f)+'.compile',{}).get('command',[]),'actual acquisition failure')
   args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual baseline/profile '+tag)
   for source in ['assessor/security_depth/artifacts/artifacts.c','assessor/security_depth/artifacts/service.c','assessor/security_depth/command/command.c','assessor/security_depth/command/service.c','assessor/security_depth/worker/worker.c','labs/security_depth/secrets/platform.c']:
    need(any(p.endswith('/'+source) for p in args),'actual composed producer '+source)
   need(any(x.endswith('/libcrypto.so.3') for x in args) and 'libcrypto.so.3' in logs.get(tag+'.dynamic',''),'actual provider linkage '+tag)
   need('fresh-trust-check cached-report-counterexample artifact-not-command-signature' in logs.get(tag+'.integrated',''),'actual adapter/counterexample '+tag)
  for variant,effect in [('first',1),('second',1),('repair',0)]:command(cc+'.release.'+variant+'.execute',0,'synthetic_effect='+str(effect))
 obs=r.get('observations',[]);need(len(obs)==6 and {o.get('profile') for o in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six exact profiles')
 for o in obs:
  need(o.get('cases')==runner.CASES and all(o.get(k) is True for k in ['actual_ed25519','adjacent_digest_replace','untrusted_actual_signature','rotation_revokes_old','commit_separate','fresh_trust_adapter','cached_report_counterexample','same_process_authority_mutation']),'actual policy/variant observations')
  need(o.get('provider_faults')==4 and o.get('new_opcode')==6 and o.get('resource_byte_budget')==4 and o.get('ordinary_work_budget')==6 and o.get('changed_widths')==[1,2],'exact failure/quota/width observations')
  need(all(o.get(k) is False for k in ['host_compromise','physical','command_signature']),'bounded isolation/installation/authentication claims')
 repro=r.get('reproducibility',[]);need(len(repro)==2 and {x.get('compiler') for x in repro}=={'gcc','clang'},'two actual compiler investigations')
 for row in repro:
  paths=[ROOT/row.get(k,'missing') for k in ['first','second','repair']];need(all(p.is_file() for p in paths),'actual release binaries')
  if all(p.is_file() for p in paths):need(sha(paths[0])==sha(paths[1]) and sha(paths[0])!=sha(paths[2]),'actual reproducibility and repair distinction')
  need(row.get('same_bytes') is True and row.get('reproduced_unauthorized_effect')==1 and row.get('repair_effect')==0 and row.get('trustworthiness_inferred') is False,'reproducibility is not trust')
 if len(repro)==2:need(sha(ROOT/repro[0]['first'])!=sha(ROOT/repro[1]['first']),'actual cross-compiler mismatch')
 need('__printf_chk@plt' in logs.get('gcc.release.first.disassembly','') and '<printf@plt>' in logs.get('clang.release.first.disassembly',''),'inspected differing call boundary')
 need('.plt.sec' in logs.get('gcc.release.first.sections','') and '.got.plt' in logs.get('clang.release.first.sections',''),'inspected section differences')
 analysis=(ROOT/'assessor/security_depth/artifacts/ANALYSIS.md').read_text(encoding='utf8')
 for boundary in ['Extent','Arithmetic','Lifetime','Authority','Quota','Descriptor','Secret','Artifact']:need('| '+boundary+' |' in analysis,'actual review row '+boundary)
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
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T35-06' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','forged-command-auth','repro-trust','lost-provider-fault','lost-fresh-adapter','lost-cached-counterexample','lost-quota','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.TRUST']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.TRUST')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='forged-command-auth':y['observations'][0]['command_signature']=True
   elif kind=='repro-trust':y['reproducibility'][0]['trustworthiness_inferred']=True
   elif kind=='lost-provider-fault':y['observations'][0]['provider_faults']=3
   elif kind=='lost-fresh-adapter':y['observations'][0]['fresh_trust_adapter']=False
   elif kind=='lost-cached-counterexample':y['observations'][0]['cached_report_counterexample']=False
   elif kind=='lost-quota':y['observations'][0]['resource_byte_budget']=6
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
