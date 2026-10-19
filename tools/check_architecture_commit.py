"""Guard exact legacy/sidecar scope and actual composed producer execution."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_commit'
from research.v5.expansion.legacy_architecture_commit.author_support import attach
from close_architecture_commit import REMAINING,all_rows,canon
import run_architecture_commit as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-03.before-commit.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment')
 expected=all_rows(r);need(d.get('updates')==expected and len(expected)==21,'exact original legacy/sidecar task, criterion, delay, binding and six dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+name)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current sources');need(r.get('external_sha256')==runner.external(),'selected tools/libc')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passing command inventory');logs={}
 for name,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[name]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+name);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'actual intended result '+name)
 def command(name,code,marker=None):
  q=commands.get(name,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+name)
 def imports(name,sources):
  for source in sources:need(any(str(q).endswith('/'+source) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+source)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_commit/commit.c','assessor/systems_certificates/systems_certificates.c','assessor/architecture_alu/x86.S','labs/architecture_commit/native.c'])
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AT_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   text=logs.get(tag+'.disassembly','');need('<aa_isa_compare>:' in text and '<sc_clock>:' in text,'actual retained producers in binary '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need('-ffreestanding' in args and '-fno-builtin' in args,'component runtime mode');symbols={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need('sc_clock' in symbols and symbols<={'sc_clock','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'actual one-way producer dependency')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AT_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_commit/commit.c','assessor/systems_certificates/systems_certificates.c','assessor/architecture_alu/thumb.S','labs/architecture_commit/link.ld']);need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'AT_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need('-D'+definition in args and '-DAT_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'AT_THUMB_FAIL');packet=json.loads(logs.get('evidence.oracle','{}'));need(packet==dict(status='pass',packets=6,physical=False),'bounded evidence classification')
 obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,actual_isa_zero_comparisons=65536,ordinary_commits=11,multicycle_ticks=12,physical=False),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==10 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected primary/source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','missing-sidecar-root','missing-sidecar-rubric','missing-sidecar-delay','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-clock-producer','lost-native-assembly','lost-target-assembly','lost-sanitizer','lost-load','lost-squash','lost-status','lost-evidence','lost-target-case','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed','missing-sidecar-root','missing-sidecar-rubric','missing-sidecar-delay']:x['updates'].pop({'missing-criterion':0,'missing-delayed':1,'missing-sidecar-root':12,'missing-sidecar-rubric':13,'missing-sidecar-delay':19}[kind])
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-load','lost-squash','lost-status','lost-evidence']:
    ident={'missing-mutant':'gcc.debug.mutation.CANCEL','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.clocked.oracle','lost-load':'gcc.debug.load','lost-squash':'gcc.debug.speculation','lost-status':'gcc.debug.status','lost-evidence':'evidence.oracle'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.SQUASH')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind in ['lost-clock-producer','lost-native-assembly','lost-target-assembly']:
    source={'lost-clock-producer':'assessor/systems_certificates/systems_certificates.c','lost-native-assembly':'assessor/architecture_alu/x86.S','lost-target-assembly':'assessor/architecture_alu/thumb.S'}[kind];row=next(q for q in y['commands'] if q['id']==('thumb.optimized.compile' if kind=='lost-target-assembly' else 'gcc.debug.compile'));row['command']=[q for q in row['command'] if not q.endswith('/'+source)]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.SQUASH.compile')['command'].remove('-DAT_TEST_CASE=5')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
