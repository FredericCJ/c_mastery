"""Exact source-bound hardening evidence, retained tasks and negative controls."""
import argparse,copy,functools,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_hardening'
from research.v5.expansion.legacy_security_commands.author_support import attach
import run_security_hardening as runner
from close_security_hardening import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
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
 c=read(canon());b=read(HERE/'PR-T35-04.before-hardening.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent maintained attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {x.get('id') for x in rows}==set(expected),'exact three criteria and three delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'full source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure');need(r.get('external_sha256')==runner.external(),'actual selected tool/libc/header identities')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact passing command inventory')
 logs={}
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[x['id']]=text;need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id'])
 def command(name,exit,marker=None):
  x=commands.get(name,{});need(x.get('exit_code')==exit and (marker is None or x.get('required_marker')==marker),'intended operation '+name)
 for cc in ['gcc','clang']:
  for m,marker in [('BOUND','sh_copy(payload, 8, &out) == 0 && out == 36'),('POLICY','sh_authorize(1,0,&n)==-1 && n==7')]:
   command(cc+'.mutation.'+m,41,marker);need('-DSH_MUTANT_'+m in commands.get(cc+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutant definition')
  for case in ['bounds','policy']:command(cc+'.stub.'+case,41,'SH_ASSERT:')
  command(cc+'.handler.stub',41,'SH_ASSERT:bounded_failure_record');command(cc+'.handler.mutant',41,'SH_ASSERT:bounded_failure_record');command(cc+'.guard',42,'OBS actual_guard_mismatch_handler');command(cc+'.sanitizer.defect',-6,'AddressSanitizer: stack-buffer-overflow')
  need('-fsanitize=address,undefined' in commands.get(cc+'.sanitizer.compile',{}).get('command',[]),'actual sanitizer '+cc)
 command('fixture.stub',41,'SH_ASSERT:declared_fixture');command('clang.unsupported',1,"unknown argument: '-fhardened'");command('arm.missing',1,'__stack_chk_fail')
 obs=r.get('observations',[]);need(len(obs)==6 and {x.get('variant') for x in obs}=={x[0] for x in runner.VARIANTS},'six exact variants')
 for o in obs:
  tag=o.get('variant','missing');hard=tag in ['gcc.hardened','gcc.aggregate','clang.hardened'];plain=tag.endswith('plain');syms=logs.get(tag+'.symbols','');programs=logs.get(tag+'.programs','');dynamic=logs.get(tag+'.dynamic','');header=logs.get(tag+'.headers','')
  need(o.get('guard')==('__stack_chk_fail' in syms) and o.get('checked_call')==('__memcpy_chk' in syms) and o.get('relro')==('GNU_RELRO' in programs),'actual emitted mechanisms '+tag)
  need(o.get('pie')==bool(re.search(r'Type:.*DYN',header)) and o.get('eager_binding')==bool('BIND_NOW' in dynamic or re.search(r'FLAGS_1.*NOW',dynamic)),'actual ELF markers '+tag)
  if hard:
   need(all(o.get(k) is True for k in ['pie','relro','eager_binding','guard','checked_call']),'hardened actual markers '+tag);command(tag+'.defect',-6,'buffer overflow detected')
  if plain:need(all(o.get(k) is False for k in ['pie','relro','eager_binding','guard','checked_call']),'plain negative marker control '+tag)
  if tag=='musl.static':need(o.get('guard') is True and o.get('checked_call') is False and o.get('interpreter')==[] and o.get('dynamic_dependencies')==[],'actual musl static topology')
  samples=[int(x) for x in re.findall(r'^SAMPLE \d+ (\d+)$',logs.get(tag+'.bench',''),re.M)];need(len(samples)==31 and o.get('valid_copy_samples_ns')==samples and o.get('iterations_per_sample')==200000 and o.get('checksum')==223200000,'raw equivalent cost experiment '+tag)
  bases=[];permissions=[]
  binary=commands.get(tag+'.compile',{}).get('command',['missing'])[-1]
  for i in range(4):
   lines=[x.split() for x in logs.get(tag+'.maps.'+str(i),'').splitlines() if binary in x]
   if lines:bases.append(int(lines[0][0].split('-')[0],16));permissions.append([x[1] for x in lines])
  need(len(bases)==4 and o.get('bases')==bases and o.get('permissions')==permissions and o.get('observed_base_change')==(len(set(bases))>1),'actual four maps '+tag)
  need(o.get('aslr_policy')==int(logs.get('aslr.policy','-1').strip()) and logs.get('page.size','').strip()=='4096','actual read-only policy/page assumption')
  need(o.get('optimized_away_unguarded') is True and o.get('data_only_violation') is True and o.get('repair_passed') is True,'eligible-site/policy limits '+tag)
  need(o.get('size_output')==logs.get(tag+'.size') and o.get('flawed_copy_stack_adjustments'),'actual resource data '+tag)
  need('-std=c17' in commands.get(tag+'.compile',{}).get('command',[]),'active baseline '+tag)
 target=dict(isa='Cortex-M4 Thumb',missing_guard_link_rejected=True,undefined_symbols=[],loader=False,libc=False,fixed_teaching_guard=True,native_record_test=True,target_execution=False,physical=False);need(r.get('target')==target,'bounded target link/native evidence');need(not logs.get('arm.final.undefined','missing').strip() and 'wfi' in logs.get('arm.disassembly',''),'actual helper closure')
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
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T35-04' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','invented-maps','invented-fortify','lost-raw-cost','target-execution','target-entropy','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.mutation.POLICY']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.mutation.POLICY')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='invented-maps':y['observations'][0]['bases']=[1,2,3,4]
   elif kind=='invented-fortify':next(q for q in y['observations'] if q['variant']=='musl.static')['checked_call']=True
   elif kind=='lost-raw-cost':y['observations'][0]['valid_copy_samples_ns']=[]
   elif kind=='target-execution':y['target']['target_execution']=True
   elif kind=='target-entropy':y['target']['fixed_teaching_guard']=False
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
