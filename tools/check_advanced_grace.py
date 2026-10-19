"""Validate exact epoch/RCU evidence, isolation, identities and negative controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_grace'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_grace as runner
import run_advanced_grace_guest as guest_runner
from close_advanced_grace import PROJECTS,ARTIFACTS,REMAINING,selected,evidence,dimensions
@functools.lru_cache(None)
def cached(path,size,mtime):return hashlib.sha256(path.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r,g):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 expected={}
 for project in PROJECTS:
  c=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'));b=read(HERE/(project+'.before-grace.json'));need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'unchanged original '+project);need(attach(copy.deepcopy(c))==c,'maintained additive hook '+project)
  for x in selected(c):
   v=x['variants'][0];expected[x['id']]=(project,x,v,False);expected[v['id']]=(project,x,v,True)
 rows=d.get('updates',[]);need(len(rows)==24 and {x.get('id') for x in rows}==set(expected),'exact twelve criteria and twelve delayed IDs')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256']),(g,['source_sha256','outputs_sha256','external_input_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for p,h in owner.get(key,{}).items():
    path=pathlib.Path(p) if key=='external_input_sha256' else ROOT/p;need(path.is_file() and sha(path)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact native source closure');need(g.get('source_sha256')==guest_runner.pins(),'exact guest source closure');need(set(runner.pins())|set(guest_runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 need(set(g.get('external_input_sha256',{}))=={str(p) for p in guest_runner.EXTERNAL},'exact kernel/source/config closure')
 commands={x['id']:x for x in r.get('commands',[])};gc={x['id']:x for x in g.get('commands',[])}
 for report,items,ids in [(r,commands,runner.expected_commands()),(g,gc,{'compiler','emulator','reference.build','reference.imports','learner.build','learner.imports','guest'})]:
  need(report.get('status')=='pass' and report.get('failure') is None and len(items)==len(report.get('commands',[])) and set(items)==ids and all(x.get('passed') is True for x in items.values()),'exact passing command inventory')
  for x in items.values():
   p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id']);need(all(m in text for m in x.get('markers',[])),'guest markers '+x['id'])
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:
   tag=cc+'.'+profile
   for name,(define,case,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DAG_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutant '+tag+name)
   for case in runner.CASES:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled scaffold rejection '+tag+case)
   if profile=='tsan':need(commands.get(tag+'.race',{}).get('exit_code')==66 and commands.get(tag+'.race',{}).get('required_marker')=='WARNING: ThreadSanitizer: data race','detector sensitivity '+tag)
 observations=r.get('observations',[]);need(len(observations)==8 and {x.get('profile') for x in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'eight native profiles')
 for x in observations:need(x.get('cases')==runner.CASES and x.get('serialized_core') is True and x.get('actual_mutex_writers')==2 and x.get('publications')==64 and x.get('retired_versions')==16 and x.get('payload_bytes')==512 and x.get('joined_departure') is True and x.get('registry')=='actual ApiRegistry with counted version captures' and x.get('independent_or_physical') is False,'actual serialized scope')
 p=ROOT/commands.get('models',{}).get('output','missing');m=r.get('models',{});need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual independent model output');need(m.get('status')=='pass' and m.get('epoch')==dict(states=339,edges=979,epoch_limit=5,participants=2,nesting_limit=2,active4_at5_reachable=True,active1_at5_reachable=False),'exact reachable/injected epoch distinction')
 need(m.get('retention')==[dict(retirements=n,one_hazard_retained=1,old_epoch_retained=n,hazard_payload_bytes=32,epoch_payload_bytes=n*32) for n in range(1,17)],'normalized retention ledger');need(m.get('comparison_unit')==dict(payload_bytes=32,metadata_excluded=True,physical_measurement=False),'comparison limits')
 need(m.get('nested')==dict(depth_before=2,after_inner_exit=1,quiescent_after_inner=False,after_outer_exit=0,quiescent_after_outer=True),'outermost quiescence');need(m.get('borrow')==dict(raw_after_exit_retained=False,reference_before_exit_retained=True,drop_after_final_invocation=True),'capture ownership');need(m.get('departure')==dict(paused_may_resume=True,forced_clear_safe=False,joined_owner_may_resume=False),'departure authority')
 targets=r.get('target_audits',[]);need(len(targets)==4 and {(x.get('compiler'),x.get('cpu')) for x in targets}=={(cc,cpu) for cc in ['arm-none-eabi-gcc','clang'] for cpu in ['cortex-m4','cortex-m0']},'four exact target audits')
 for x in targets:
  t=x.get('compiler')+'.'+x.get('cpu');need(x.get('bare_closure') is False and x.get('declared_byte_provider_closure') is True and x.get('provider')=='assessor/execution/support.c' and x.get('execution')=='not-run' and x.get('client_callbacks_audited') is False,'named helper dependency and target limits')
  for part in ['grace','epoch','registry','support']:
   p=ROOT/commands.get(t+'.'+part+'.imports',{}).get('output','missing');need(p.is_file() and p.read_text(encoding='utf8').splitlines()==x.get('imports',{}).get(part),'actual imports '+t+part)
  p=ROOT/commands.get(t+'.composed.closed',{}).get('output','missing');need(p.is_file() and not p.read_text(encoding='utf8').strip(),'actual closed image '+t)
 config={k:k+'=y' for k in ['CONFIG_PREEMPT_RCU','CONFIG_KASAN','CONFIG_PROVE_LOCKING','CONFIG_DEBUG_ATOMIC_SLEEP']};config['CONFIG_PREEMPT_RT']='absent';need(g.get('kernel_config')==config,'actual selected kernel configuration')
 obs=g.get('observations',[]);need(len(obs)==10 and len({x.get('kernel_time') for x in obs})==10 and all(x.get('hardirq_mask',0)>0 and x.get('old')==1 and x.get('new')==2 and x.get('held_grace') is True and x.get('freed')==1 and x.get('errors')==0 for x in obs),'ten actual hard-interrupt reader/grace observations')
 q=gc.get('guest',{}).get('command',[]);need(q and q[0]=='qemu-system-x86_64' and all(v in q for v in ['-snapshot','-nodefaults','-nic','none','-smp','2']),'guest isolation and two CPUs')
 p=ROOT/gc.get('guest',{}).get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(all(s in text for s in ['AG_GUEST_PASS cycles=10 learner_rejected=1 negative_rejected=1','AG_RCU_STUB mechanism_unimplemented','AG_RCU_NEGATIVE_DETECTED errors=1 held=0','AG_RCU_UNLOADED callbacks_drained=1']) and 'AG_GUEST_FAIL' not in text,'actual guest controls and callback drain')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==16 and all(x.get('passed') is True for x in exports['commands']),'two real exports16 commands')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 ec={x['id']:x for x in exports.get('commands',[])}
 for project in PROJECTS:
  for case in runner.CASES:
   x=ec.get(project+'.rejection.'+case,{});need(x.get('exit_code')==41 and 'AG_ASSERT:' in x.get('output',''),'exported semantic rejection '+project+case)
 for p,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits')+g.get('limits'),'artifacts/bounded limits');need(row.get('remaining_author_work')==[],'specified author mechanism complete')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']),'criterion evidence binding')
 need(d.get('remaining_parents')==REMAINING,'remaining required work retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');g=read(HERE/'guest_validation.json');errors=validate(d,r,g);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','stale-kernel','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','invented-reachability','clear-paused-owner','capture-without-reference','remove-serialization','guest-not-hardirq','invented-kernel-config','missing-helper-provider','target-executed','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r);z=copy.deepcopy(g)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-kernel':'external_input_sha256'}[kind];owner=z if kind=='stale-kernel' else y;owner[key][next(iter(owner[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.refs']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.refs')['required_marker']='unrelated'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='invented-reachability':y['models']['epoch']['active1_at5_reachable']=True
   elif kind=='clear-paused-owner':y['models']['departure']['forced_clear_safe']=True
   elif kind=='capture-without-reference':y['models']['borrow']['reference_before_exit_retained']=False
   elif kind=='remove-serialization':y['observations'][0]['serialized_core']=False
   elif kind=='guest-not-hardirq':z['observations'][0]['hardirq_mask']=0
   elif kind=='invented-kernel-config':z['kernel_config']['CONFIG_PREEMPT_RT']='CONFIG_PREEMPT_RT=y'
   elif kind=='missing-helper-provider':y['target_audits'][0]['provider']='libc'
   elif kind=='target-executed':y['target_audits'][0]['execution']='passed'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,z));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),guest_commands=len(g['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
