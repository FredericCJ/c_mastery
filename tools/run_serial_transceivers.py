"""Actual selected-transceiver core and UART producer composition campaign."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_transceivers';PUBLIC=ROOT/'labs/serial_followthrough/transceivers';PRIVATE=ROOT/'assessor/serial_followthrough/transceivers';BUILD=ROOT/'build'/('serial-transceivers-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['thresholds','connections','capture232','differential','exclusion','captures422']
MUTANTS={'TYPICAL':(0,'d == expected','TR_MUTANT=1'),'OPEN':(3,'d == (delta>=200?1:delta<=-200?0:TR_UNKNOWN)','TR_MUTANT=2'),'DIRECTION':(1,'tr_connections(&dte,&dte,straight,3) == TR_INVALID','TR_MUTANT=3'),'EVIDENCE':(2,'TR_INCOMPLETE && result.matched == 99','TR_MUTANT=4'),'EXCLUSION':(4,'TR_BUSY && b.owner == 0 && b.serial == 1','TR_MUTANT=5'),'CROSSINGS':(5,'edge.transitions == 3 && edge.opposite == 2','TR_MUTANT=6')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/serial_followthrough/uart/uart_depth.h','labs/serial_followthrough/uart/learner.c','labs/serial_followthrough/uart/CONTRACTS.md','assessor/serial_followthrough/uart/uart_depth.c','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','assessor/serial_depth/serial_depth.c','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']
INCLUDES=['labs/serial_followthrough/transceivers','labs/serial_followthrough/uart','labs/peripheral_depth/uart','labs/serial_depth/include']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['TYPICAL','EXCLUSION','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'transceivers.c',*[ROOT/n for n in ['assessor/serial_followthrough/uart/uart_depth.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c']],PUBLIC/'tests.c',PUBLIC/'native.c'];run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('uart','assessor/serial_followthrough/uart/uart_depth.c'),('byte','assessor/peripheral_depth/uart/uart.c'),('serial','assessor/serial_depth/serial_depth.c'),('uart-include',INCLUDES[1]),('byte-include',INCLUDES[2]),('serial-include',INCLUDES[3]),('boot','assessor/interrupt_depth/frame/target.S'),('link','labs/interrupt_depth/frame/target.ld'),('runtime','assessor/execution/support.c')]:a+=['--'+k,ROOT/v]
  if not stub:a+=['--source',PRIVATE/'transceivers.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='TR_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='TR_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'TR_GUEST_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS transceivers_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'TR_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['TYPICAL','EXCLUSION']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual author implementations, six original criterion/V1 mechanisms; no independent learner or external review.','MPS2 executes actual Thumb with synthetic line points and RAM UART effects, not transceiver analog physics or physical F439 UART.','Selected-device source intervals and original circuit policy do not certify arbitrary third-party modules or cables.','Physical MAX3232 and two-receiver differential captures, uncertainty bounds and changed cable/load/baud measurements remain required.','struct tr_bus is only a trusted serialized coordinator policy; real RS485 parts/loading/failsafe and malicious-peer exclusion remain distinct obligations.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
