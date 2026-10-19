"""Exact six kernel obligations, changed contexts and independent negative guards."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_kernel'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_kernel as runner
import run_advanced_kernel_guest as guest
from close_advanced_kernel import ARTIFACTS,REMAINING,selected,evidence,dimensions,canon
@functools.lru_cache(None)
def cached(p,size,mtime):return hashlib.sha256(p.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r,g):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 c=read(canon());b=read(HERE/'PR-T26-03.before-kernel.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged');need(attach(copy.deepcopy(c))==c,'maintained idempotent attachment');expected={}
 for x in selected(c):
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {x.get('id') for x in rows}==set(expected),'exact six criteria and six delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256']),(g,['source_sha256','outputs_sha256','external_input_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=pathlib.Path(name) if key=='external_input_sha256' else ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'native full source closure');need(g.get('source_sha256')==guest.pins(),'guest exact source closure');need(g.get('external_input_sha256')==guest.external(),'actual two kernel identities');need(set(runner.pins())|set(guest.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 commands={x['id']:x for x in r.get('commands',[])};gc={x['id']:x for x in g.get('commands',[])};gids={'compiler','emulator','probe.build'}
 for tag in ['packaged','debug']:
  gids|={tag+'.guest',*[tag+'.'+x+'.build' for x in ['reference','learner','retained']],*[tag+'.'+n+'.'+part for n in ['ak_work','ak_work_stub','b06_owner','b06_irq'] for part in ['abi','imports']]}
 for report,items,ids in [(r,commands,runner.expected_commands()),(g,gc,gids)]:
  need(report.get('status')=='pass' and report.get('failure') is None and len(items)==len(report.get('commands',[])) and set(items)==ids and all(x.get('passed') is True for x in items.values()),'exact passing command inventory')
  for x in items.values():
   p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id']);need(all(s in text for s in x.get('markers',[])),'actual guest markers '+x['id'])
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:
   tag=cc+'.'+profile
   for name,(define,case,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DAK_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutant '+tag+name)
   for case in runner.CASES:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled semantic stub '+tag+case)
 obs=r.get('observations',[]);need(len(obs)==6 and {x.get('profile') for x in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual native profiles')
 for x in obs:need(x.get('copy_residual_cases')==560 and x.get('actual_spsc') is True and x.get('retained_mapping_until_quiescence') is True and x.get('physical') is False and x.get('serialization')=='one external owner','copy and backend observation limits')
 p=ROOT/commands.get('models',{}).get('output','missing');m=r.get('models',{});need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual independent model output');need(m.get('status')=='pass','models passed');graphs=m.get('lkmm_fragment',[]);need(len(graphs)==4 and [(x.get('release'),x.get('acquire'),x.get('bad_outcome_excluded')) for x in graphs]==[(False,False,False),(False,True,False),(True,False,False),(True,True,True)],'exact four-event MP comparison')
 cp=m.get('c_publication',{});need(cp.get('atomic_data_stale_read_excluded') is True and cp.get('relaxed_flag_stale_read_not_excluded') is True and cp.get('plain_data_without_synchronization_is_data_race') is True and cp.get('kernel_barrier_creates_iso_synchronizes_with') is False,'distinct ISO and kernel semantics')
 need([(x.get('context'),x.get('user_copy'),x.get('gfp_kernel'),x.get('may_sleep')) for x in m.get('contexts',[])]==[('process_unlocked',True,True,True),('ordinary_worker_unlocked',True,True,True),('hardirq',False,False,False),('raw_spin_held',False,False,False)],'exact context restrictions')
 life=m.get('lifetime',{});need(life.get('original_exact') is True and life.get('unload_attempts')==[False,True] and life.get('rejected_events')==[2] and len(life.get('counterexamples',[]))==2 and all(x.get('unload_safe') is False for x in life.get('counterexamples',[])),'fixture and excluded timer/requeue edges');need(life.get('repaired_order')==['close_admission_under_gate','wake_and_join_producer','cancel_work_sync','cancel_timer','release_object','remove_code'],'actual closure order')
 device=m.get('device',{});need(device.get('cpu_publication_implies_device_visibility') is False and device.get('coherent_requires_ordering') is True and device.get('dma_barrier_orders_mmio') is False and device.get('physical_observation') is False,'third-domain visibility limits')
 go=g.get('observations',[]);need(len(go)==2 and [x.get('profile') for x in go]==['packaged','debug'] and [x.get('release') for x in go]==['7.0.0-38-generic','7.0.14-cmastery-b06'],'two distinct actual guest releases')
 for x in go:
  tag=x.get('profile');need(x.get('cycles')==10 and x.get('runs_per_cycle')==16 and x.get('external_requeues_per_cycle')==15 and x.get('retained_usercopy') is True and x.get('actual_hardirq_threaded') is True and x.get('admission_closed_before_join') is True and x.get('physical') is False,'actual guest semantics '+str(tag))
  config=x.get('config',{});expected_config={k:(k+'=y' if tag=='debug' else '# '+k+' is not set') for k in ['CONFIG_KASAN','CONFIG_PROVE_LOCKING','CONFIG_DEBUG_ATOMIC_SLEEP']};expected_config['CONFIG_PREEMPT_RT']='absent' if tag=='debug' else '# CONFIG_PREEMPT_RT is not set';need(config==expected_config,'actual diagnostic availability '+str(tag))
  cmd=gc.get(tag+'.guest',{});args=cmd.get('command',[]);need(args and args[0]=='qemu-system-x86_64' and all(s in args for s in ['-snapshot','-nodefaults','-nic','none','-smp','2']),'isolated two-CPU guest');p=ROOT/cmd.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(all(s in text for s in ['AK_GUEST_PASS cycles=10','AK_WORK_NEGATIVE no_racing_enqueue_required=1','AK_WORK_STUB unimplemented_admission_and_drain','AK_WORK_UNLOADED retained_callbacks=0','B06_KERNEL_PASS copy_fault=atomic','B06_IRQ_PASS task=1 hardirq=1 threaded=1']) and 'AK_GUEST_FAIL' not in text,'actual complete guest trace '+tag)
  if tag=='debug':need('AK_MISMATCHED_BUILD_REJECTED' in text and '.gnu.linkonce.this_module section size must match' in text,'actual mismatched ABI rejection')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==4 and all(x.get('passed') is True for x in exports['commands']),'actual four-command learner export')
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 ec={x['id']:x for x in exports.get('commands',[])}
 for case in runner.CASES:need(ec.get('rejection.'+case,{}).get('exit_code')==41 and 'AK_ASSERT:' in ec.get('rejection.'+case,{}).get('output',''),'actual exported rejection '+case)
 for name,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/name)==h,'source passage gate '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits')+g.get('limits'),'bounded artifact claims')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T26-03' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task/delay');need(row.get('author_validation')==evidence(list(commands),c['objective']),'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining external/source/aggregate obligations')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');g=read(HERE/'guest_validation.json');errors=validate(d,r,g);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','stale-kernel','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','kernel-equals-iso','hardirq-sleeps','barrier-drains-timer','cancel-before-close','dma-orders-mmio','physical-inference','same-kernel','invented-rt','lost-copy-boundary','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r);z=copy.deepcopy(g)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-kernel':'external_input_sha256'}[kind];owner=z if kind=='stale-kernel' else y;owner[key][next(iter(owner[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.quiesce']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.quiesce')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='kernel-equals-iso':y['models']['c_publication']['kernel_barrier_creates_iso_synchronizes_with']=True
   elif kind=='hardirq-sleeps':y['models']['contexts'][2]['may_sleep']=True
   elif kind=='barrier-drains-timer':y['models']['lifetime']['counterexamples'][1]['unload_safe']=True
   elif kind=='cancel-before-close':y['models']['lifetime']['repaired_order'].reverse()
   elif kind=='dma-orders-mmio':y['models']['device']['dma_barrier_orders_mmio']=True
   elif kind=='physical-inference':z['observations'][0]['physical']=True
   elif kind=='same-kernel':z['observations'][1]['release']=z['observations'][0]['release']
   elif kind=='invented-rt':z['observations'][1]['config']['CONFIG_PREEMPT_RT']='CONFIG_PREEMPT_RT=y'
   elif kind=='lost-copy-boundary':y['observations'][0]['copy_residual_cases']=1
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,z));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),guest_commands=len(g['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
