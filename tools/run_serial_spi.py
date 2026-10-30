"""T38.06 exact original/changed tasks using the retained SPI/ring producers."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_spi';PUBLIC=ROOT/'labs/serial_followthrough/spi';PRIVATE=ROOT/'assessor/serial_followthrough/spi';BUILD=ROOT/'build'/('serial-spi-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
HEADERS=pathlib.Path('/home/frede/cmastery-freestanding-tools/root/usr/include/newlib')
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['edges','queue','quiescence','irq','device'];PACKETS=[1,17,2,1,3]
MUTANTS={'TOKEN':(3,'irq_status==PS_STALE','SS_MUTANT=1'),'LEASE':(1,'ss_take(&q,1,&result,out)==PS_BUSY','SS_MUTANT=2'),'DRAIN':(1,'m.reads==n','PS_MUTANT=2'),'BSY':(1,'==PS_BUSY','PS_MUTANT=3'),'CANCEL':(3,'irq_q.driver.tx_owned && irq_q.driver.rx_owned','PS_MUTANT=4'),'STATUS':(0,'==PS_PROTOCOL','PS_MUTANT=5'),'ENDIAN':(0,'value==0x1234','PS_MUTANT=6')}
MAPPED_MUTANTS={'AF':('PS_TARGET_MUTANT=2','==0x555fffffu'),'WIDTH':('PS_TARGET_MUTANT=3','==0x06002c01u'),'CHANNEL':('PS_TARGET_MUTANT=1','==0x06002c01u'),'MODE':('PS_MUTANT=1','==0x35fu')}
IMPORTS=['tools/run_thread_lifecycle.py','fixtures/communications/PR-T38-05.json','labs/peripheral_depth/spi/spi.h','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/spi/target.c','labs/peripheral_depth/spi/CONTRACTS.md','labs/peripheral_depth/spi/BOARD_CAPTURE.md','assessor/peripheral_depth/spi/spi.c','assessor/peripheral_depth/spi/target.c','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','assessor/serial_depth/serial_depth.c','labs/algorithms/include/algo.h','labs/algorithms/src/algo.c','assessor/algorithms/algo.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/execution/support.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():
 files={pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')}|{p for p in HEADERS.rglob('*') if p.is_file()}
 assert (HEADERS/'math.h') in files
 return {str(p):sha(p) for p in sorted(files)}
def expected_commands():
 out={'edge.derive','kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   t=cc+'.'+p;out|={t+'.'+n for n in ['compile','mapped.compile','mapped','mapped.oracle','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in [*MUTANTS,*MAPPED_MUTANTS,'EDGE']:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for o in ['0','3']:out|={cc+'.O'+o+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['TOKEN','LEASE','DRAIN','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;derived=BUILD/'edge-mutant.c'
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/peripheral_depth/spi'),'-I'+str(ROOT/'labs/peripheral_depth/uart'),'-I'+str(ROOT/'labs/serial_depth/include'),'-I'+str(ROOT/'labs/algorithms/include')]
 def compile(tag,cc,opt,stub=False,mapped=False,edge=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'scheduler.c',PUBLIC/'ports_learner.c' if stub else PRIVATE/'ports.c',ROOT/('labs/peripheral_depth/spi/spi.c' if stub else 'assessor/peripheral_depth/spi/spi.c'),ROOT/('labs/peripheral_depth/uart/uart.c' if stub else 'assessor/peripheral_depth/uart/uart.c'),PUBLIC/'ring_prerequisite.c' if stub else ROOT/'assessor/algorithms/algo.c',ROOT/'labs/serial_depth/src/serial_depth.c' if stub else derived if edge else ROOT/'assessor/serial_depth/serial_depth.c']
  if mapped:s += [ROOT/('labs/peripheral_depth/spi/target.c' if stub else 'assessor/peripheral_depth/spi/target.c'),PUBLIC/'mapped.c',PUBLIC/'mapped_native.c'];opt+=['-DPS_TARGET_MODEL']
  else:s += [PUBLIC/'tests.c',PUBLIC/'native.c']
  run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);harness.OUTPUTS.add(b);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt,'--ports',PRIVATE/'ports.c','--spi',ROOT/'assessor/peripheral_depth/spi/spi.c','--spi-target',ROOT/'assessor/peripheral_depth/spi/target.c','--uart',ROOT/'assessor/peripheral_depth/uart/uart.c','--serial',ROOT/'assessor/serial_depth/serial_depth.c','--algorithm',ROOT/'assessor/algorithms/algo.c','--spi-include',ROOT/'labs/peripheral_depth/spi','--uart-include',ROOT/'labs/peripheral_depth/uart','--serial-include',ROOT/'labs/serial_depth/include','--algorithm-include',ROOT/'labs/algorithms/include','--type-headers',HEADERS,'--boot',ROOT/'assessor/interrupt_depth/frame/target.S','--link',ROOT/'labs/interrupt_depth/frame/target.ld','--runtime',ROOT/'assessor/execution/support.c']
  if not stub:a+=['--source',PRIVATE/'scheduler.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='SS_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='SS_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'SS_GUEST_PASS')
 try:
  run('edge.derive',[sys.executable,PRIVATE/'provider_mutant.py',ROOT/'assessor/serial_depth/serial_depth.c',derived]);run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt.copy())
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS '+('irq_stale' if c=='irq' else c));run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.mapped',cc,opt.copy(),mapped=True);run(tag+'.mapped',[b],marker='OBS mapped');run(tag+'.mapped.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.mapped.log'),'mapped'],marker='"status": "pass"');b=compile(tag+'.stub',cc,opt.copy(),stub=True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'SS_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
    for m,(definition,marker) in MAPPED_MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition],mapped=True);run(tag+'.mutation.'+m,[b],41,marker)
    b=compile(tag+'.mutation.EDGE',cc,opt.copy(),edge=True);run(tag+'.mutation.EDGE',[b,0],41,'sample[edge]==')
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['TOKEN','LEASE','DRAIN']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,'==PS_STALE && m.calls==calls' if m=='TOKEN' else MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Three original T38.06 criterion/V1 pairs; actual author implementation and bounded observations, not learner mastery.','Actual frozen struct ps_spi/struct pu_rates/algo_ring/sd_spi_edge and F439 adapter; MPS2 executes Thumb and a real software-pended CPU IRQ with RAM register models, not physical SPI or DMA.','Selected newlib headers are compilation inputs only; retained firmware has no libc/libm/compiler-runtime link. Unused algorithms are discarded and not claimed validated.','Finite two-slot scheduler, serialization and caller-controlled time; no fairness, interrupt latency, all-interleaving or hard termination guarantee.','Physical dedicated-peer timing, electrical loading and board routing remain required, along with external review and independent delayed learner attempts.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
