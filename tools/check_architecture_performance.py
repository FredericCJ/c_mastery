"""Guard original performance tasks, real providers and all raw measurement cells."""
import argparse,collections,copy,functools,hashlib,json,pathlib,re,statistics,struct,sys
from fractions import Fraction
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_performance'
from research.v5.expansion.legacy_architecture_performance.author_support import attach
from close_architecture_performance import REMAINING,all_rows,canon
from assessor.architecture_performance.generate import source
import run_architecture_performance as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def relocated_kernel_targets(p):
 """ELF64 little-endian relative relocation evidence for static pointer tables.

 A pointer initializer need not appear as an annotated instruction operand in
 objdump. Read the real relocation addend and symbol value instead of assuming
 Clang emits the same source-order instruction form as GCC.
 """
 raw=data(p);assert raw[:6]==b'\x7fELF\x02\x01';offset=struct.unpack_from('<Q',raw,40)[0];size,count=struct.unpack_from('<HH',raw,58);sections=[struct.unpack_from('<IIQQQQIIQQ',raw,offset+i*size) for i in range(count)];names={};addends=set()
 for section in sections:
  _,kind,_,_,start,length,link,_,_,stride=section
  if kind==2:
   strings=sections[link];table=raw[strings[4]:strings[4]+strings[5]]
   for at in range(start,start+length,stride):
    name,_,_,_,value,_=struct.unpack_from('<IBBHQQ',raw,at);text=table[name:].split(b'\0',1)[0].decode()
    if text in ['pe_c','pe_asm']:names[text]=value
  if kind==4:
   for at in range(start,start+length,stride):
    _,info,value=struct.unpack_from('<QQq',raw,at)
    if info&0xffffffff==8:addends.add(value)
 return {name for name,value in names.items() if value in addends}
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-08.before-performance.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected=all_rows(r);need(d.get('updates')==expected and len(expected)==6,'exact original task/delay/binding/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current sources');need(r.get('external_sha256')==runner.external(),'actual tool/libc identity')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passing command inventory');logs={}
 for name,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[name]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+name);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'actual intended result '+name)
 def command(name,code,marker=None):
  q=commands.get(name,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+name)
 def imports(name,sources):
  for item in sources:need(any(str(q).endswith('/'+item) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+item)
 generated=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/prior.c')];provenance=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/provenance.json')];code,record=source();need(len(generated)==1 and data(generated[0]).decode()==code,'exact extracted producer bodies/macro');need(len(provenance)==1 and read(provenance[0])==record,'exact source provenance')
 drift=json.loads(logs.get('drift','{}'));need(drift.get('status')=='pass' and drift.get('paired_differences')==[5]*31 and drift.get('unpaired_mean_difference')==[-3095,1] and drift.get('differential_drift_median')==-5,'ordinary/differential drift counterexample')
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile
   for suffix in ['.compile','.measure.compile']:
    args=commands.get(tag+suffix,{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag+suffix);imports(tag+suffix,['assessor/architecture_performance/performance.c','assessor/algorithms/x86_64.S']);need(bool(generated) and str(generated[0]) in args,'actual producer compilation '+tag+suffix);need('-flto' not in args,'declared non-LTO boundary '+tag)
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'PE_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutant '+m)
   command(tag+'.no_work',41,'PE_MEASURE_ASSERT: calibration result/checksum');need('-DPE_MUTANT_NO_WORK' in commands.get(tag+'.no_work.compile',{}).get('command',[]),'actual omitted-work control '+tag);command(tag+'.measure',0,'PE_MEASURE_PASS raw=248 no_speed_threshold')
   asm=logs.get(tag+'.measure.disassembly','');need(all('<'+f+'>:' in asm for f in ['pe_c','pe_asm','algo_lower','ab_checked','algo_asm_lower']),'actual retained provider symbols '+tag);binary_args=commands.get(tag+'.measure',{}).get('command',[]);binary=pathlib.Path(binary_args[0]) if binary_args else None;need(binary is not None and binary.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'actual pinned measurement binary '+tag);relocated=relocated_kernel_targets(binary) if binary is not None and binary.is_file() else set()
   for callee in ['pe_c','pe_asm','algo_lower','ab_checked','algo_asm_lower']:
    # pe_c/pe_asm may be selected through actual function pointers; require
    # an address reference and direct producer edges underneath those calls.
    if callee in ['pe_c','pe_asm']:need(asm.count('<'+callee+'>')>=2 or (callee in relocated and bool(re.search(r'\bcallq?\s+\*%',asm))),'actual kernel pointer instruction/ELF relocation '+tag+callee)
    else:need(bool(re.search(r'\b(?:callq?|jmpq?)\s+[^\n]*<'+callee+'>',asm)),'retained real producer edge '+tag+callee)
   text=logs.get(tag+'.measure','');samples=[list(map(int,s.split()[1:])) for s in text.splitlines() if s.startswith('SAMPLE ')];need(len(samples)==248,'all raw samples '+tag);group=collections.defaultdict(list);cells=set()
   for row in samples:
    if len(row)!=11:need(False,'raw row shape '+tag);continue
    n,policy,block,order,k,reps,elapsed,cpu,before,after,checksum=row;need(n in [16,1024] and policy in [0,1] and 0<=block<31 and order in [0,1] and k==(order^(block&1)) and reps in [1<<i for i in range(10)] and elapsed>0 and cpu>=0,'actual selected observation '+tag);cells.add((n,policy,block,k));group[n,policy,k].append(Fraction(elapsed,reps*64))
   need(len(cells)==248 and len(group)==8 and all(len(x)==31 for x in group.values()),'complete original/changed design '+tag)
   analysis=json.loads(logs.get(tag+'.analysis','{}'));need(analysis.get('status')=='pass' and analysis.get('raw_samples')==248 and analysis.get('samples_per_condition')==31 and analysis.get('conditions')==8 and analysis.get('work')==[[16,257,960,1008],[1024,641,65472,32830]],'actual independent analyzer '+tag);need(len(analysis.get('raw_overhead',[]))==101 and analysis.get('clock_resolution_ns',0)>0,'actual timer evidence '+tag)
   for summary in analysis.get('summaries',[]):
    n=summary['size'];policy=int(summary['preparation']=='displacement-before-batch')
    for condition in summary['conditions']:
     k=int(condition['kernel']=='checked-assembly');xs=group[n,policy,k]
     if xs:
      for key,value in [('minimum_ns',min(xs)),('median_ns',statistics.median(xs)),('maximum_ns',max(xs))]:need(condition[key]==[value.numerator,value.denominator],'unfiltered independent '+key+' '+tag)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(q in args for q in ['-ffreestanding','-fno-builtin','-fstack-usage']),'component runtime/cost mode');s={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need({'algo_lower','ab_checked'}<=s and s<={'algo_lower','ab_checked','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'actual producer unresolved edges');need(all(n in logs.get(cc+'.core.stack','') for n in ['pe_equation','pe_stats','pe_c','pe_asm']),'actual stack listing')
 obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,raw_native_samples=248,samples_per_condition=31,conditions=8,physical_mcu=False,equal_admission=True,producer='algo_lower + ab_checked + original algo_asm_lower'),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==7 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','tests.compile','measure.compile','measure.rejection',*{'tests.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():
  need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
  if name!='init':need(q.get('cwd','').endswith('/PR-T37-08'),'export isolation '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-measurement','lost-no-work','lost-oracle','lost-producer','lost-provenance','lost-sanitizer','lost-outlier','lost-admission','lost-drift','lost-cost','lost-disassembly','wrong-sample-count','lost-delay','wrong-design','wrong-producer','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-measurement','lost-no-work','lost-oracle','lost-outlier','lost-admission','lost-drift','lost-cost','lost-disassembly']:
    ident={'missing-mutant':'gcc.debug.mutation.UNITS','lost-measurement':'gcc.debug.measure','lost-no-work':'gcc.debug.no_work','lost-oracle':'gcc.debug.equation.oracle','lost-outlier':'gcc.debug.mutation.OUTLIER','lost-admission':'gcc.debug.mutation.TRUST','lost-drift':'drift','lost-cost':'gcc.core.stack','lost-disassembly':'gcc.debug.measure.disassembly'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.COMM')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-producer':
    row=next(q for q in y['commands'] if q['id']=='gcc.debug.measure.compile');row['command']=[q for q in row['command'] if not q.endswith('/assessor/algorithms/x86_64.S')]
   elif kind=='lost-provenance':y['outputs_sha256']={k:v for k,v in y['outputs_sha256'].items() if not k.endswith('/generated/provenance.json')}
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.measure.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='wrong-sample-count':y['observations'][0]['samples_per_condition']=9
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   elif kind=='wrong-design':y['observations'][0]['conditions']=2
   elif kind=='wrong-producer':y['observations'][0]['producer']='unchecked assembly only'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
