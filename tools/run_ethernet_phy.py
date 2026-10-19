"""Source-bound T39.05 PHY author experiments; never a physical-board receipt."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_phy';PUBLIC=ROOT/'labs/ethernet_followthrough/phy';PRIVATE=ROOT/'assessor/ethernet_followthrough/phy';BUILD=ROOT/'build'/('ethernet-phy-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['access','lifecycle','bounded','mapped','leases','faults']
MUTANTS={'EPOCH':(1,'p.incarnation==1','EP_MUTANT=1'),'RESET':(1,'m.reset_value==0x8000','EP_MUTANT=2'),'ACK':(1,'p.phase==EP_RESTART_WAIT','EP_MUTANT=3'),'BUSY':(0,'==ED_BUSY && m.writes==0','EP_MUTANT=4'),'BOUND':(0,'m.reads==4','EP_MUTANT=5'),'LATCH':(1,'p.incarnation==2','EP_MUTANT=6'),'READY':(1,'!p.ready && p.incarnation==3','EP_MUTANT=7'),'ADDRESS':(3,'ep_begin(&p,16,1000)==ED_BUSY','EP_TARGET_MUTANT=1')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','assessor/ethernet_depth/ethernet_depth.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.S','assessor/execution/support.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():
 files={pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')}
 return {str(p):sha(p) for p in sorted(files)}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','port.compile','port.audit']}
 for m in ['EPOCH','RESET','READY','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DEP_TARGET_MODEL','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/ethernet_depth/include')]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;s=[PUBLIC/'learner.c' if stub else PRIVATE/'phy.c',PUBLIC/'target_learner.c' if stub else PRIVATE/'target.c',ROOT/('labs/ethernet_depth/src/ethernet_depth.c' if stub else 'assessor/ethernet_depth/ethernet_depth.c'),*[PUBLIC/n for n in ['model.c','tests.c','mapped.c','leases.c','native.c']]]
  run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt,'--target',PRIVATE/'target.c','--ethernet',ROOT/'assessor/ethernet_depth/ethernet_depth.c','--ethernet-include',ROOT/'labs/ethernet_depth/include','--boot',ROOT/'assessor/interrupt_depth/frame/target.S','--runtime',ROOT/'assessor/execution/support.c']
  if not stub:a+=['--source',PRIVATE/'phy.c']
  if definition:a+=['--define',definition]
  run(tag+'.build',a,marker='EP_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='EP_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'EP_GUEST_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS phy_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'EP_MAP_ASSERT:' if c=='mapped' else 'EP_LEASE_ASSERT:' if c=='leases' else 'EP_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    port=d/'physical-port.o';run(tag+'.port.compile',[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-std=c17','-O'+opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Werror','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/ethernet_depth/include'),'-c',PRIVATE/'target.c','-o',port]);run(tag+'.port.audit',[sys.executable,PRIVATE/'port_audit.py',port],marker='"passed": true')
  for m in ['EPOCH','RESET','READY']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual author implementations and finite T39.05 criterion/changed experiments; no independent learner or external review claim.','MPS2 executes actual Thumb C and original RAM MDIO effects. It is not F439 Ethernet silicon, PHY electrical signaling, board routing or a physical link.','Nonwrapping caller clock plus independent finite budgets; callbacks themselves must terminate. No fairness or wall-clock deadline guarantee follows from these tests.','Retained EDTx composition establishes bounded lease refusal only; actual normal RX/TX DMA descriptor driver remains a separate required producer.','Physical route-revision, continuous50MHz REF_CLK, MDC edge/cycle timing and negotiation observations remain required.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
