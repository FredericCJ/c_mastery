"""Actual selected-transceiver core and UART producer composition campaign."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_rs485';PUBLIC=ROOT/'labs/serial_followthrough/rs485';PRIVATE=ROOT/'assessor/serial_followthrough/rs485';BUILD=ROOT/'build'/('serial-rs485-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['config','timing','events','cancellation','link','port']
MUTANTS={'WAKE':(0,'d.enable==11000','R5_MUTANT=1'),'TC':(1,'m.r[PU_DR]==34&&m.de','R5_MUTANT=2'),'LOSS':(4,'before==','R5_MUTANT=3'),'RESERVE':(3,'q.reserved==2','R5_MUTANT=4'),'TOKEN':(1,'==R5_STALE','R5_MUTANT=5'),'PREFLIGHT':(2,'m.reads==1','R5_MUTANT=6'),'POLARITY':(5,'gpio[6]==(1u<<10)','R5_PORT_MUTANT=1'),'DMA_STOP':(3,'s.tx.byte.memory_owned','PU_MUTANT=6'),'TXE':(1,'m.r[PU_DR]==34&&m.de','PU_MUTANT=7')}
IMPORTS=['assessor/peripheral_depth/uart/target.c','labs/communications/include/com_edu7.h','labs/communications/learner/edu7.c','assessor/communications/edu7.c','tools/run_thread_lifecycle.py','labs/serial_followthrough/uart/uart_depth.h','labs/serial_followthrough/uart/learner.c','labs/serial_followthrough/uart/CONTRACTS.md','assessor/serial_followthrough/uart/uart_depth.c','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','assessor/serial_depth/serial_depth.c','labs/interrupt_depth/frame/target.S','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']
INCLUDES=['labs/serial_followthrough/rs485','labs/serial_followthrough/uart','labs/peripheral_depth/uart','labs/serial_depth/include','labs/communications/include']
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
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','port.compile','port.audit','bad-placement']}
 for m in ['WAKE','LOSS','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DR5_TARGET_MODEL','-DPU_TARGET_MODEL',*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'channel.c',PUBLIC/'port_learner.c' if stub else PRIVATE/'port.c',ROOT/'assessor/peripheral_depth/uart/target.c',ROOT/'assessor/communications/edu7.c',*[ROOT/n for n in ['assessor/serial_followthrough/uart/uart_depth.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c']],PUBLIC/'tests.c',PUBLIC/'mapped.c',PUBLIC/'native.c'];run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('uart','assessor/serial_followthrough/uart/uart_depth.c'),('byte','assessor/peripheral_depth/uart/uart.c'),('serial','assessor/serial_depth/serial_depth.c'),('uart-include',INCLUDES[1]),('byte-include',INCLUDES[2]),('serial-include',INCLUDES[3]),('boot','assessor/interrupt_depth/frame/target.S'),('link','labs/serial_followthrough/rs485/target.ld'),('byte-port','assessor/peripheral_depth/uart/target.c'),('edu','assessor/communications/edu7.c'),('edu-include',INCLUDES[4]),('runtime','assessor/execution/support.c')]:a+=['--'+k,ROOT/v]
  if not stub:a+=['--source',PRIVATE/'channel.c','--port',PRIVATE/'port.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='R5_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='R5_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'R5_GUEST_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS rs485_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'R5_PORT_ASSERT:' if c=='port' else 'R5_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    port=d/'physical-port.o';run(tag+'.port.compile',[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-std=c17','-O'+opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Werror',*['-I'+str(ROOT/n) for n in INCLUDES],'-c',PRIVATE/'port.c','-o',port]);run(tag+'.port.audit',[sys.executable,PRIVATE/'port_audit.py',port],marker='"passed": true')
    args=next(q['command'] for q in harness.COMMANDS if q['id']==tag+'.build').copy();args[args.index('--build')+1]=str(BUILD/(tag+'.bad-placement'));run(tag+'.bad-placement',args+['--bad-placement'],1,'R5_DMA_REACHABILITY')
  for m in ['WAKE','LOSS']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual author implementations and original three criterion/V1 pairs; no independent learner or external review.','Actual UaTx/PuDriver and EDU7 decoder composed; no complete T40 stack claim.','MPS2 executes Thumb with original RAM register effects; no physical F439 UART/DMA or transceiver physics.','32-bit modular clocks require every caller interval and active deadline comparison within the documented half-range; clock source and tick conversion are caller obligations.','Physical two-node bus, termination/reference/loading and actual DE/last-stop-bit traces remain required.','Receiver-enabled target uses RE tied low; shutdown timing is a separately selected abstract policy, not an executed target RE pin adapter.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
