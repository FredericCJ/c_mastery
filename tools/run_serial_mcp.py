"""Actual metadata/wire/transaction/TDC author campaign with explicit scope."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_mcp';PUBLIC=ROOT/'labs/serial_followthrough/mcp';PRIVATE=ROOT/'assessor/serial_followthrough/mcp';BUILD=ROOT/'build'/('serial-mcp-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['layout','init','failure','transfer','receive','bench','spi','atomic']
MUTANTS={'CRC':(1,'m.calls == calls + 1','MC_MUTANT=1'),'ZERO_FD':(3,'r == MC_OK','MC_MUTANT=2'),'PUBLICATION':(3,'mc_poll','MC_MUTANT=3'),'RX_ADVANCE':(4,'m.rx_consumed == 1','MC_MUTANT=4'),'RAM_BUDGET':(0,'l.total ==','MC_MUTANT=5'),'ECC':(1,'load(0xe0c) == 7','MC_MUTANT=6'),'EARLY_READY':(2,'d.state != MC_READY','MC_MUTANT=7'),'PEER_SEQUENCE':(5,'MC_FAILED','MC_MUTANT=8'),'RX_EARLY':(7,'out.id == 999','MC_MUTANT=10'),'CS_QUARANTINE':(6,'rx[0] == 77','MC_MUTANT=9')}
IMPORTS=['tools/run_thread_lifecycle.py','assessor/peripheral_depth/spi/target.c','labs/peripheral_depth/spi/spi.h','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/spi/CONTRACTS.md','assessor/peripheral_depth/spi/spi.c','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','assessor/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','dossier.controls'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','f439.route']}
 for m in ['PUBLICATION','RX_ADVANCE','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/serial_depth/include'),'-I'+str(ROOT/'labs/peripheral_depth/spi'),'-I'+str(ROOT/'labs/peripheral_depth/uart')]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'mcp.c',PUBLIC/'spi_adapter.c' if stub else PRIVATE/'spi_adapter.c',ROOT/('labs/peripheral_depth/spi/spi.c' if stub else 'assessor/peripheral_depth/spi/spi.c'),ROOT/('labs/peripheral_depth/uart/uart.c' if stub else 'assessor/peripheral_depth/uart/uart.c'),PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('spi','labs/peripheral_depth/spi/spi.c' if stub else 'assessor/peripheral_depth/spi/spi.c'),('uart','labs/peripheral_depth/uart/uart.c' if stub else 'assessor/peripheral_depth/uart/uart.c'),('spi-include','labs/peripheral_depth/spi'),('uart-include','labs/peripheral_depth/uart'),('adapter','labs/serial_followthrough/mcp/spi_adapter.c' if stub else 'assessor/serial_followthrough/mcp/spi_adapter.c'),('serial-include','labs/serial_depth/include'),('boot','assessor/interrupt_depth/frame/target.S'),('link','labs/interrupt_depth/frame/target.ld'),('runtime','assessor/execution/support.c')]:a+=['--'+k,ROOT/v]
  a+=['--compare',PRIVATE/'compare.c']
  if not stub:a+=['--source',PRIVATE/'mcp.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='MCP_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='MC_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'MCP_GUEST_PASS')
 try:
  run('dossier.controls',[sys.executable,PRIVATE/'dossier_tests.py'],marker='"synthetic_only": true')
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS mcp_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'MC_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    run(cc+'.O'+opt+'.f439.route',[sys.executable,PRIVATE/'target_audit.py',BUILD/(cc+'.O'+opt+'.route'),cc,opt],marker='"passed": true')
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['PUBLICATION','RX_ADVANCE']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual current selected-controller implementation, read-CRC and original register fixture; not physical device validation.','Actual retained struct ps_spi/struct pu_rates composition and Cortex-M4 execution; target register interactions are fixture-generated, not physical F439/MCP samples.','Cold/awake40MHz route only; sleep/wake and arbitrary topology not supported.','Both original physical milestones, actual board/peer/capture, external review and independent delayed learner work remain required and not-run.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
