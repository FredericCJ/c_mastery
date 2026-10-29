"""T38.01/.02 UART architecture, format and actual producer composition."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_uart';PUBLIC=ROOT/'labs/serial_followthrough/uart';PRIVATE=ROOT/'assessor/serial_followthrough/uart';BUILD=ROOT/'build'/('serial-uart-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['frames','baud','tx','rx','deadline'];PACKETS=[1,6,16,1,1]
MUTANTS={'WIDTH':(0,'((f.cr1>>12)&1u)==(width==8 && parity!=0)','UA_MUTANT=1'), 'BUDGET':(4,'ua_step(&s,io(&m),1,8)==PU_TIMEOUT','UA_MUTANT=2'),'ERROR':(3,'b.error==(i&15u)','UA_MUTANT=3'),'EARLY':(2,'ua_step(&s,io(&m),1,now++)==PU_BUSY','PU_MUTANT=7'),'STATUS':(3,'b.error==(i&15u)','PU_MUTANT=4'),'GENERATION':(1,'ua_begin(&s,io(&m),2,1,&b,1,0,100)==PU_STALE','PU_MUTANT=5')}
MAPPED_MUTANTS={'CHANNEL':('PU_TARGET_MUTANT=1','R(STREAM)==0x08000441u'),'AF':('PU_TARGET_MUTANT=2','R(0x40020c24u)==0xffffff77u'),'DMA_CANCEL':('PU_MUTANT=6','pu_cancel(&d,io,2)==PU_OK && d.memory_owned'),'DMA_TC':('PU_MUTANT=7','!d.memory_owned && d.line_owned && d.active')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/target.c','labs/peripheral_depth/uart/CONTRACTS.md','assessor/peripheral_depth/uart/uart.c','assessor/peripheral_depth/uart/target.c','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/execution/support.c','fixtures/communications/PR-T38-01.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'cache.investigation','cache.mutation.CLEAN','cache.mutation.INVALIDATE','kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.stack'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','mapped.compile','mapped','mapped.oracle','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in list(MUTANTS)+list(MAPPED_MUTANTS):out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for o in ['0','3']:out|={cc+'.O'+o+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['WIDTH','EARLY','ERROR','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/peripheral_depth/uart'),'-I'+str(ROOT/'labs/serial_depth/include')]
 def compile(tag,cc,opt,stub=False,mapped=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'uart_depth.c',ROOT/'assessor/peripheral_depth/uart/uart.c',ROOT/'assessor/serial_depth/serial_depth.c']
  if mapped:s += [ROOT/'assessor/peripheral_depth/uart/target.c',PUBLIC/'mapped.c',PUBLIC/'mapped_native.c'];opt+=[ '-DPU_TARGET_MODEL']
  else:s += [PUBLIC/'tests.c',PUBLIC/'native.c']
  run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);harness.OUTPUTS.add(b);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt,'--uart',ROOT/'assessor/peripheral_depth/uart/uart.c','--uart-target',ROOT/'assessor/peripheral_depth/uart/target.c','--serial',ROOT/'assessor/serial_depth/serial_depth.c','--uart-include',ROOT/'labs/peripheral_depth/uart','--serial-include',ROOT/'labs/serial_depth/include','--boot',ROOT/'assessor/interrupt_depth/frame/target.S','--link',ROOT/'labs/interrupt_depth/frame/target.ld','--runtime',ROOT/'assessor/execution/support.c']
  if not stub:a+=['--source',PRIVATE/'uart_depth.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='UA_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='UA_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'UA_GUEST_PASS')
 try:
  run('cache.investigation',[sys.executable,PRIVATE/'cache_investigation.py'],marker='CACHE_INVESTIGATION_PASS');
  for mutant in ['CLEAN','INVALIDATE']:run('cache.mutation.'+mutant,[sys.executable,PRIVATE/'cache_investigation.py',mutant],43,'CACHE_ASSERT:')
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt.copy())
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS '+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.mapped',cc,opt.copy(),mapped=True);run(tag+'.mapped',[b],marker='OBS mapped');run(tag+'.mapped.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.mapped.log'),'mapped'],marker='"status": "pass"');b=compile(tag+'.stub',cc,opt.copy(),stub=True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'UA_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
    for m,(definition,marker) in MAPPED_MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition],mapped=True);run(tag+'.mutation.'+m,[b],41,marker)
   obj=BUILD/(cc+'.core.o');run(cc+'.core',[cc,*flags,'-O2','-ffreestanding','-fno-builtin','-fstack-usage','-c',PRIVATE/'uart_depth.c','-o',obj]);harness.OUTPUTS.add(obj);harness.OUTPUTS.add(obj.with_suffix('.su'));run(cc+'.core.undefined',['nm','-u',obj]);run(cc+'.core.stack',['cat',obj.with_suffix('.su')])
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['WIDTH','EARLY','ERROR']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Exact PR-T38-01 ordinary and changed analysis/implementation mechanisms; no physical UART/RS232 accuracy, board-route, external-review or learner assessment.','Actual unchanged struct pu_driver/struct pu_rx and sd_uart/sd_uart_decode producers; actual mapped F439 register adapter in host and MPS2 RAM model, not a peripheral emulator.','MPS2 executes the imported reset/vector source unchanged with IRQs masked; no interrupt service or latency observation.','No full-domain closure; PR03–12 remain separately required.'];r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted({p for p in BUILD.rglob('*') if p.is_file()}|harness.OUTPUTS)},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
