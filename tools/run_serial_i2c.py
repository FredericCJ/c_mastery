"""T38.04/.05 actual I2C producer composition and electrical derivations."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_i2c';PUBLIC=ROOT/'labs/serial_followthrough/i2c';PRIVATE=ROOT/'assessor/serial_followthrough/i2c';BUILD=ROOT/'build'/('serial-i2c-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['electrical','arbitration','transaction','failure','recovery','late_publication'];PACKETS=[303,1,4,5,12,1]
MUTANTS={'ROUND':(0,'x.minimum==967','IA_MUTANT=1'),'LEAK':(0,'actual_max*','IA_MUTANT=2'),'PHASE':(1,'==IA_UNSUPPORTED','IA_MUTANT=3'),'LOSER':(1,'r.survivors==','IA_MUTANT=4'),'IDLE':(4,'!r.active && !l.writes','IA_MUTANT=5'),'ACK':(2,'==PI_BUSY && !m.violation','PI_MUTANT=1'),'BTF':(2,'==PI_BUSY && !m.reads && !m.stops','PI_MUTANT=2'),'ARLO':(3,'m.stops==0','PI_MUTANT=3')}
MAPPED_MUTANTS={'AF':('PI_TARGET_MUTANT=1','==0xffffff44u'),'DRAIN':('PI_TARGET_MUTANT=2','==0xffffff44u')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/peripheral_depth/i2c/i2c.h','labs/peripheral_depth/i2c/i2c.c','labs/peripheral_depth/i2c/target.c','labs/peripheral_depth/i2c/CONTRACTS.md','assessor/peripheral_depth/i2c/i2c.c','assessor/peripheral_depth/i2c/target.c','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/execution/support.c','fixtures/communications/PR-T38-03.json','fixtures/communications/PR-T38-04.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'capture','capture.mutation','late.derive','kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.stack'}
  for p,_ in PROFILES:
   t=cc+'.'+p;out|={t+'.'+n for n in ['compile','mapped.compile','mapped','mapped.oracle','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in [*MUTANTS,*MAPPED_MUTANTS,'LATE']:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for o in ['0','3']:out|={cc+'.O'+o+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['ROUND','ACK','ARLO','LATE','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;derived=BUILD/'late-mutant.c'
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/peripheral_depth/i2c'),'-I'+str(ROOT/'labs/peripheral_depth/uart'),'-I'+str(ROOT/'labs/serial_depth/include')]
 def compile(tag,cc,opt,stub=False,mapped=False,late=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'i2c_depth.c',PUBLIC/'clear_learner.c' if stub else PRIVATE/'clear.c',ROOT/'labs/peripheral_depth/i2c/i2c.c' if stub else derived if late else ROOT/'assessor/peripheral_depth/i2c/i2c.c',ROOT/('labs/peripheral_depth/uart/uart.c' if stub else 'assessor/peripheral_depth/uart/uart.c'),ROOT/('labs/serial_depth/src/serial_depth.c' if stub else 'assessor/serial_depth/serial_depth.c')]
  if mapped:s += [ROOT/'assessor/peripheral_depth/i2c/target.c',PUBLIC/'mapped.c',PUBLIC/'mapped_native.c'];opt+=['-DPI_TARGET_MODEL']
  else:s += [PUBLIC/'electrical.c',PUBLIC/'transaction.c',PUBLIC/'cases.c',PUBLIC/'native.c']
  run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);harness.OUTPUTS.add(b);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False,late=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt,'--i2c',derived if late else ROOT/'assessor/peripheral_depth/i2c/i2c.c','--i2c-target',ROOT/'assessor/peripheral_depth/i2c/target.c','--uart',ROOT/'assessor/peripheral_depth/uart/uart.c','--serial',ROOT/'assessor/serial_depth/serial_depth.c','--i2c-include',ROOT/'labs/peripheral_depth/i2c','--uart-include',ROOT/'labs/peripheral_depth/uart','--serial-include',ROOT/'labs/serial_depth/include','--boot',ROOT/'assessor/interrupt_depth/frame/target.S','--link',ROOT/'labs/interrupt_depth/frame/target.ld','--runtime',ROOT/'assessor/execution/support.c']
  if not stub:a+=['--source',PRIVATE/'i2c_depth.c','--clear',PRIVATE/'clear.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='IA_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='IA_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'IA_GUEST_PASS')
 try:
  run('capture',[sys.executable,PRIVATE/'capture_investigation.py'],marker='CAPTURE_INVESTIGATION_PASS');run('capture.mutation',[sys.executable,PRIVATE/'capture_investigation.py','decoded-only'],43,'CAPTURE_ASSERT:')
  run('late.derive',[sys.executable,PRIVATE/'provider_mutant.py',ROOT/'assessor/peripheral_depth/i2c/i2c.c',derived]);run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt.copy())
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS '+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.mapped',cc,opt.copy(),mapped=True);run(tag+'.mapped',[b],marker='OBS mapped');run(tag+'.mapped.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.mapped.log'),'mapped'],marker='"status": "pass"');b=compile(tag+'.stub',cc,opt.copy(),stub=True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'IA_ASSERT:' if i<2 else 'IA_TRANSACTION_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
    for m,(definition,marker) in MAPPED_MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition],mapped=True);run(tag+'.mutation.'+m,[b],41,marker)
    b=compile(tag+'.mutation.LATE',cc,opt.copy(),late=True);run(tag+'.mutation.LATE',[b,5],41,'OBS late_publication 0 195 0 3')
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*flags,'-O2','-ffreestanding','-fno-builtin','-fstack-usage','-c',PRIVATE/'i2c_depth.c','-o',obj]);run(cc+'.core.undefined',['nm','-u',obj]);run(cc+'.core.stack',['cat',obj.with_suffix('.su')])
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['ROUND','ACK','ARLO']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  # The ordinary transaction test rejects stale progress before the final publication probe.
  execute('thumb.LATE',target('thumb.LATE',late=True),True,'==PI_STALE && m.calls==calls');execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Six original T38.04/.05 criterion/V1 pairs; author implementation and finite executed evidence, not learner mastery.','Actual unchanged PiI2c/PiRecovery/PuRates producers and F439 mapped adapter; MPS2 executes C/Thumb with RAM register models, not an I2C peripheral emulator.','Synthetic analog evidence packets and bounded rational model; physical board routes, instrument accuracy, rise/fall/hold waveforms remain required.','No arbitrary-length transaction, interrupt latency, multi-controller recovery lease implementation or full T38 closure.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
