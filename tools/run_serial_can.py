"""Source-bound Classic CAN codec/FCE/selected bxCAN campaign; no hardware claim."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
from c_source import replace_unique
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_can';PUBLIC=ROOT/'labs/serial_followthrough/can';PRIVATE=ROOT/'assessor/serial_followthrough/can';BUILD=ROOT/'build'/('serial-can-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['arbitration','codec','stream','fault','timing','fifo','mailbox','recovery','port']
MUTANTS={'NUMERIC':(0,'winner==-1','NUMERIC'),'STREAM':(2,'consumed==n-second','CN_MUTANT=1'),'CRC':(1,'==CN_ERROR','CN_MUTANT=2'),'PROPAGATION':(4,'==CN_UNAVAILABLE','CN_MUTANT=3'),'ARB_COUNTER':(3,'s.tec==0','CN_FAULT_MUTANT=1'),'EXCEPTION':(3,'s.tec==128&&!out.rules','CN_FAULT_MUTANT=2'),'REC_RESET':(3,'s.rec==127','CN_FAULT_MUTANT=3'),'EARLY_ARB':(6,'==CN_MORE','CN_CONTROLLER_MUTANT=1'),'ABORT_LEASE':(6,'==CN_BUSY','CN_CONTROLLER_MUTANT=2'),'W1C':(5,'out.overrun==1','CN_CONTROLLER_MUTANT=3')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','assessor/serial_depth/serial_depth.c','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']
SOURCES=['codec.c','fault.c','controller.c','port.c']
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
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','port.compile','port.audit']}
 for m in ['STREAM','ABORT_LEASE','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DCN_MAPPED','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/serial_depth/include')]
 def compile(tag,cc,opt,stub=False,numeric=False):
  b=BUILD/tag;serial=ROOT/('labs/serial_depth/src/serial_depth.c' if stub else 'assessor/serial_depth/serial_depth.c')
  if numeric:
   old=serial.read_text()
   serial=BUILD/'numeric-mutant.c';serial.write_text(replace_unique(old,'n = arb(a, x);','*winner=a->identifier < b->identifier ? -1 : a->identifier > b->identifier ? 1 : 0; return SD_OK;\n  n = arb(a, x);'))
  s=[PUBLIC/'learner.c'] if stub else [PRIVATE/n for n in SOURCES]
  run(tag+'.compile',[cc,*flags,*opt,*s,serial,PUBLIC/'tests.c',PUBLIC/'controller_tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('serial','labs/serial_depth/src/serial_depth.c' if stub else 'assessor/serial_depth/serial_depth.c'),('serial-include','labs/serial_depth/include'),('boot','assessor/interrupt_depth/frame/target.S'),('link','labs/interrupt_depth/frame/target.ld'),('runtime','assessor/execution/support.c')]:a+=['--'+k,ROOT/v]
  if not stub:
   for n in SOURCES:a+=['--source',PRIVATE/n]
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='CN_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='CN_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'CN_GUEST_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS can_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'CN_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,*([] if definition=='NUMERIC' else ['-D'+definition])],numeric=definition=='NUMERIC');run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    port=d/'physical-port.o';run(tag+'.port.compile',[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-std=c17','-O'+opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Werror','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/serial_depth/include'),'-c',PRIVATE/'port.c','-o',port]);run(tag+'.port.audit',[sys.executable,PRIVATE/'port_audit.py',port],marker='"passed": true')
  for m in ['STREAM','ABORT_LEASE']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Author implementations and six original ordinary/delayed tasks; no independent learner or external review.','Actually read current ISO fault-confinement clauses; classified-event model is not a physical FCE or full bit-level controller.','Local strict standard-data codec intentionally narrower than all current ISO receiver behavior.','MPS2 executes Thumb with original RAM register effects; no physical F439 CAN1, transceiver, ACK or bus timing measurements.','Board clock/pin/reset/transceiver prerequisites and physical two-node captures remain required; no production targets.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
