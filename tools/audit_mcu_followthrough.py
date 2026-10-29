"""Independent fixture arithmetic, native transfer checks and actual image ledgers."""
from __future__ import annotations
import argparse,hashlib,json,re,subprocess,sys
from fractions import Fraction
from pathlib import Path
from stack_mcu_followthrough import bounds
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def fixture(f):
 x=f['input'];kind=f['oracle']
 if kind=='image':return {'unresolved':sorted(set(x['symbols'])-set(x['provided'])),'forbidden_libraries':[v for v in x['libraries'] if any(q in v for q in ('libc','libgcc','compiler-rt','libatomic'))]}
 if kind=='pool':
  used=0;trace=[];rejected=[]
  for n in x['requests']:
   if 0<=used+n<=x['capacity']:used+=n
   else:rejected.append(n)
   trace.append(used)
  return {'used':trace,'rejected':rejected}
 if kind=='frame':
  n=(x['software_words']+x['hardware_words'])*x['word_bytes'];a=x['alignment']
  return {'payload_bytes':n,'aligned_bytes':n+(-n)%a}
 if kind=='states':
  table={('ready','dispatch'):'running',('running','block'):'blocked',('blocked','wake'):'ready',('running','exit'):'exited',('exited','reap'):'free'};state=x['initial'];trace=[]
  for a in x['actions']:state=table[state,a];trace.append(state)
  return {'states':trace}
 if kind=='periodic':
  absolute=[];relative=[];clock=x['phase']
  for i,work in enumerate(x['work']):absolute.append(x['phase']+(i+1)*x['period']);clock+=x['period'];relative.append(clock);clock+=work
  return {'absolute_releases':absolute,'relative_releases':relative}
 if kind=='masks':
  old=x['initial'];stack=[];during=[]
  for requested in x['enter_masks']:stack.append(old);old=requested if not old else min(old,requested);during.append(old)
  return {'during':during,'after_exit':list(reversed(stack))}
 if kind=='references':
  n=x['initial'];counts=[]
  for d in x['deltas']:n+=d;counts.append(n)
  return {'counts':counts,'first_zero_step':counts.index(0)+1}
 if kind=='winner':return {'terminal':x['events'][0],'ignored':x['events'][1:]}
 if kind=='timer_counts':
  n=-(-x['microseconds']*x['frequency']//1000000)
  return {'counts':n,'actual_us_numerator':n*1000000,'actual_us_denominator':x['frequency']}
 if kind=='budget':return {'ram_used':sum(x['ram_components']),'ram_free':x['ram_limit']-sum(x['ram_components']),'cycles_total':sum(x['cycles'])}
 raise ValueError(kind)
def serial_block(text,label,count):
 tail=text.split(label+'\n',1)[1];values=re.findall(r'^[0-9a-f]{8}$',tail,re.M)
 return [int(v,16) for v in values[:count]]
def main():
 p=argparse.ArgumentParser();p.add_argument('--campaign',type=Path,required=True);p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False)
 checks=[];commands=[];fixtures=[];images=[]
 def check(name,passed,details=None):
  checks.append({'name':name,'passed':bool(passed),'details':details})
  if not passed:raise RuntimeError(name)
 def command(name,argv,marker=None):
  r=subprocess.run(list(map(str,argv)),cwd=ROOT,text=True,capture_output=True,timeout=60);out=r.stdout+r.stderr;(work/(name+'.txt')).write_text(out)
  commands.append({'name':name,'command':list(map(str,argv)),'returncode':r.returncode,'output':out,'passed':r.returncode==0 and (marker is None or marker in out)})
  check(name,commands[-1]['passed'])
 try:
  for n in range(1,13):
   path=ROOT/f'fixtures/os_v3/PR-T50-{n:02}.json';f=json.loads(path.read_text());actual=fixture(f);check(f['id']+' independent fixture',actual==f['expected']);fixtures.append({'path':path.relative_to(ROOT).as_posix(),'sha256':sha(path),'actual':actual})
  for cc in ('gcc','clang'):
   for profile,flags in (('O0',['-O0']),('O2',['-O2']),('san',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])):
    name=cc+'-'+profile;exe=work/name
    command('compile-'+name,[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(ROOT/'labs/mcu_followthrough/core'),'-I'+str(ROOT/'labs/execution/include'),ROOT/'labs/mcu_followthrough/core/supplement.c',ROOT/'assessor/mcu_followthrough/core/mcu.c',ROOT/'assessor/execution/cmx.c','-o',exe])
    command('run-'+name,[exe],'MF_PASS:supplement')
  campaign=json.loads(a.campaign.read_text());check('campaign119',campaign['passed'] and len(campaign['rows'])==119)
  base=a.campaign.resolve().parent
  for name in ('direct-gcc-O2','direct-clang-O2','direct-fp-gcc-O2','direct-fp-clang-O2','spill-gcc','spill-clang','preempt-fp-gcc-O0','preempt-fp-gcc-O2','preempt-fp-clang-O0','preempt-fp-clang-O2','long-isr-gcc','long-isr-clang','log-mask-gcc','log-mask-clang','integrated-gcc-O2','integrated-clang-O2'):
   directory=base/name;report=json.loads((directory/'report.json').read_text());check(name+' passed image',report['status']=='passed')
   symbols={n:int(v,16) for v,n in re.findall(r'^([0-9a-fA-F]+)\s+\w\s+(\S+)$',(directory/'symbols.txt').read_text(),re.M)}
   frames=[]
   for su in sorted(directory.glob('*.su')):
    for line in su.read_text().splitlines():
     fields=line.split('\t');function=fields[0].split(':')[-1];frames.append({'function':function,'bytes':int(fields[1]),'kind':fields[2],'path':su.relative_to(ROOT).as_posix()})
   static_ram=symbols['__retained_end']-symbols['__data_start'];ram2=symbols.get('__data2_end',0)-symbols.get('__data2_start',0);free=symbols['__stack_top']-symbols['__retained_end'];check(name+' static+MSP reserve',free>8192)
   log=(directory/'serial.log').read_text();row={'name':name,'report':(directory/'report.json').relative_to(ROOT).as_posix(),'report_sha256':sha(directory/'report.json'),'static_primary_ram':static_ram,'secondary_data':ram2,'MSP_reserved':8192,'unreserved_gap_below_MSP':free-8192,'C_frames':frames,'assembly_direct_integer':40,'assembly_direct_FP':104,'exception_basic_payload':68,'exception_extended_payload':204,'alignment_allowance':4}
   if name.startswith(('direct','spill')):
    # The deepest live recursion has depth3,2,1,0: four nested() frames.
    # Additional probe/yield/switch assembly is inspected separately.
    nested=next(x['bytes'] for x in frames if x['function'].split('.')[0]=='nested')
    row['deepest_recursive_C_frame_bytes']=4*nested
    sizes={x['function'].split('.')[0]:x['bytes'] for x in frames}
    fp=name.startswith(('direct-fp','spill'));probe=48+(64 if fp else 0);switch=40+(64 if fp else 0)
    diagnostic=sizes['mf_require']+max(sizes['mf_text'],sizes['mf_number']+sizes['mf_text'],sizes['mf_finish'])
    row['selected_direct_stack_bound']=sizes['entry']+4*nested+max(probe+sizes['mf_direct_yield']+switch,diagnostic)
    row['selected_direct_bound_derivation']={'entry':sizes['entry'],'four_nested_frames':4*nested,'assembly_probe':probe,'C_yield':sizes['mf_direct_yield'],'assembly_switch':switch,'alternative_assertion_chain':diagnostic,'premises':'Depth<=3 (four calls), no enabled interrupts in direct image, exact compiled frames; controlled-return branch is smaller. No general bound for new code or external callbacks.'}
    row['observed_stack_words']=[int(v,16) for v in re.findall(r'MF direct task/high-water-words [0-9a-f]{8}\n([0-9a-f]{8})',log)]
    row['scope']='Selected complete direct-task call path bound; asynchronous/nested exception profiles require their separate PSP/MSP ledger.'
    check(name+' three watermarks',len(row['observed_stack_words'])==3 and all(0<n<768 for n in row['observed_stack_words']))
    check(name+' selected stack bound',max(row['observed_stack_words'])*4<=row['selected_direct_stack_bound']<768*4)
   if name.startswith(('preempt','long-isr')):
    row['frame_observation']=serial_block(log,'MF basic/extended/lazy-before/lazy-after/MSP-min',5)
    row['paint_before_after']=serial_block(log,'MF painted extension words before/after first FP instruction',2)
    row['counts_25MHz']=serial_block(log,'MF 25MHz emulated counts max-pend-latency/max-ISR/read-overhead/FP-instruction',4)
    check(name+' observed first-FP work',row['paint_before_after']==([1,1] if name.endswith('O0') else [18,1]))
    row['simultaneous_architectural_budget']='Extended task PSP payload204+possible4; SysTick basic/extended MSP32/104+possible4 plus its C call chain; nested NMI additional32/104+possible4 plus its C call chain. Sum simultaneous frames, not all .su functions;8KiB reserve is selected capacity, not a universal proof.'
    row['linked_call_path_bounds']=bounds(directory,frames)
    check(name+' nested selected capacity',row['linked_call_path_bounds']['MSP_nested_conservative_bytes']<8192 and row['linked_call_path_bounds']['MSP_fault_or_switch_bytes']<8192 and row['linked_call_path_bounds']['PSP_conservative_bytes']<4096)
   if name.startswith('integrated'):
    row['component_bytes']=serial_block(log,'MF RAM input/output/pool+leases/stacks/kernel+wrapper/diagnostics',6)
    row['CPU_work_bound']='Processor8 rounds per admitted sample; producer bounded12 burst and UART4 reads/IRQ; scheduler16 slots; queue capacities4/2; diagnostics2. Wait/interrupt interference and bus timing excluded from struct lr_cpu instruction bounds.'
    check(name+' stack reservation',row['component_bytes'][3]==5*768*4)
   images.append(row)
  for cc in ('gcc','clang'):
   normal=next(x for x in images if x['name']=='direct-fp-'+cc+'-O2');spill=next(x for x in images if x['name']=='spill-'+cc)
   check(cc+' spill invalidates old watermark',spill['deepest_recursive_C_frame_bytes']>normal['deepest_recursive_C_frame_bytes'] and max(spill['observed_stack_words'])>max(normal['observed_stack_words']))
   regular=next(x for x in images if x['name']=='preempt-fp-'+cc+'-O2');long=next(x for x in images if x['name']=='long-isr-'+cc)
   check(cc+' long ISR observed extra work',long['counts_25MHz'][1]>regular['counts_25MHz'][1])
  nominal=32768;ppm=10000;fast=Fraction(nominal*(1000000+ppm),1000000);nominal_counts=32768;safe_counts=-(-fast.numerator//fast.denominator)
  calibration={'requested_us':1000000,'hypothetical_fast_error_ppm':ppm,'nominal_counts':nominal_counts,'actual_us_at_fast_clock':str(Fraction(nominal_counts*1000000,1)/fast),'safe_counts_using_maximum_frequency':safe_counts,'scope':'Worked calibration premise counterexample, not measured oscillator error.'}
  check('nominal-only conversion can expire early',Fraction(nominal_counts,1)/fast<1 and Fraction(safe_counts,1)/fast>=1)
 except Exception as e:passed=False;error=str(e);calibration=locals().get('calibration')
 else:passed=True;error=None
 sources=[Path(__file__).resolve(),ROOT/'tools/stack_mcu_followthrough.py',ROOT/'labs/mcu_followthrough/core/supplement.c',ROOT/'assessor/mcu_followthrough/core/mcu.c',ROOT/'assessor/execution/cmx.c',ROOT/'labs/mcu_followthrough/core/mcu.h',ROOT/'labs/execution/include/cmx.h']
 result={'passed':passed,'error':error,'checks':checks,'commands':commands,'fixtures':fixtures,'images':images,'calibration':calibration,'campaign':{'path':a.campaign.as_posix(),'sha256':sha(a.campaign)},'source_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in sources},'artifacts':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Author arithmetic/native/image audit; no independent learner or physical bound.'}
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'checks':len(checks),'commands':len(commands)}));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
