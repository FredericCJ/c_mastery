"""Execute actual retained EDU7 and explicit boundary/route/representation mechanisms."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_edu7_physical';PUBLIC=ROOT/'labs/edu7_followthrough/physical';PRIVATE=ROOT/'assessor/edu7_followthrough/physical';BUILD=ROOT/'build'/('edu7-physical-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['physical','tx','grant','port']
MARKERS=['EP_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'FIT':(0,'b.divisor','EP_MUTANT=1'),'TC':(1,'ep_step','EP_MUTANT=2'),'CHUNK':(1,'m.de==','EP_MUTANT=3'),'START':(2,'ep_tick','EP_MUTANT=4'),'ECHO':(2,'echo.echoes','EP_MUTANT=5'),'PORT':(3,'(gpioa[8]','EP_PORT_MUTANT=1'),'UNGRANTED':(2,'ep_reply','EP_MUTANT=6'),'PREENABLE':(1,'ep_step(&s,1,199)','EP_MUTANT=7'),'DOUBLEGUARD':(2,'r==EP_BUSY && s->grant.phase','EP_MUTANT=8')}
INCLUDES=['labs/edu7_followthrough/physical','labs/edu7_followthrough/foundation','labs/serial_followthrough/rs485','labs/serial_followthrough/uart','labs/peripheral_depth/uart','labs/serial_depth/include','labs/communications/include','labs/edu7_depth/include']
PROVIDERS=['assessor/edu7_followthrough/foundation/foundation.c','assessor/serial_followthrough/rs485/channel.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c','assessor/communications/edu7.c','assessor/edu7_depth/edu7_depth.c']
INPUTS={'foundation':PROVIDERS[0],'channel':PROVIDERS[1],'byte':PROVIDERS[2],'serial':PROVIDERS[3],'codec':PROVIDERS[4],'depth':PROVIDERS[5],'direction':'assessor/serial_followthrough/rs485/port.c','byte-port':'assessor/peripheral_depth/uart/target.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c'}
INPUTS.update(dict(zip([n+'-include' for n in ['foundation','channel','uart','byte','serial','codec','depth']],INCLUDES[1:])))
IMPORTS=['tools/run_thread_lifecycle.py',*PROVIDERS,*INPUTS.values(),*[p.as_posix() for d in INCLUDES[1:] for p in pathlib.Path(d).glob('*.h')],'labs/communications/include/com_edu7.h','labs/serial_depth/include/serial_depth.h','labs/edu7_depth/include/edu7_depth.h','labs/edu7_depth/CONTRACTS.md','labs/serial_followthrough/rs485/CONTRACTS.md','labs/peripheral_depth/uart/CONTRACTS.md','labs/edu7_followthrough/foundation/CONTRACTS.md']
IMPORTS=[n for n in sorted(set(IMPORTS)) if (ROOT/n).is_file()]

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
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','real.build','real.closure','real.disassembly']}
 for m in ['TC','ECHO','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-DEP_PORT_MODEL','-DPU_TARGET_MODEL','-DR5_TARGET_MODEL','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'physical.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'port_learner.c' if stub else PRIVATE/'port.c',ROOT/'assessor/serial_followthrough/rs485/port.c',ROOT/'assessor/peripheral_depth/uart/target.c',PUBLIC/'mapped.c',PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False,real=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  args+=['--port',PRIVATE/'port.c']
  if not stub:args+=['--source',PRIVATE/'physical.c']
  if real:args+=['--real-port']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='EP_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='EP_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'EP_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'EP_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,'EP_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    real=target(tag+'.real',cc,opt,real=True);assert not run(tag+'.real.closure',['arm-none-eabi-nm','-u',real/'image.elf']).strip();run(tag+'.real.disassembly',['arm-none-eabi-objdump','-d',real/'image.elf'])
  for m in ['TC','ECHO']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,'EP_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual retained struct pu_driver/R5 timing and EDU7 framing/grant composition, six host and four Cortex-M4 struct lr_cpu model profiles.','F439 USART3/PD10 port is retained, linked and disassembled but never executed on physical boards; no waveform, termination, polarity or contention evidence.','No independent learner or external review; finite author tests are bounded evidence.','T40 transport epoch, distributed session and full two-segment exchange remain required separately.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
