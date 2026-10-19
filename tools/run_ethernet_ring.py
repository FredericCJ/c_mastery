"""Source-bound actual normal-ring implementation and changed-condition experiments."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_ring';PUBLIC=ROOT/'labs/ethernet_followthrough/ring';PRIVATE=ROOT/'assessor/ethernet_followthrough/ring';BUILD=ROOT/'build'/('ethernet-ring-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['init','tx','rx','recovery','drop','bounds','failures','port','chain','publication','visibility','consumer','order']
MUTANTS={
 'REBUILD':(1,'en_model_tx(&model,0)==ED_OK','EN_MUTANT=1'),
 'FCS':(2,'out.length==60','EN_MUTANT=2'),
 'BORROW':(2,'en_rx_take(&ring,&out)==ED_BUSY','EN_MUTANT=3'),
 'STOP':(5,'en_stop_step(&ring,0)==ED_BUSY','EN_MUTANT=4'),
 'RESET':(0,'en_reset_step(&ring,1)==ED_BUSY','EN_MUTANT=5'),
 'ACK':(4,'en_status(&ring,&s)==ED_OK','EN_MUTANT=6'),
 'PHY':(3,'en_tx_copy(&ring,bytes,64,&absent)==ED_STALE','EN_MUTANT=7'),
 'DELAY':(7,'(uint32_t)(count-start)>=720u','EN_TARGET_MUTANT=1'),
 'MODE':(7,'pmc==0x800000u','EN_TARGET_MUTANT=2'),
 'PINS':(7,'==11','EN_TARGET_MUTANT=3'),
 'CHAIN':(9,'ev_own(&s,0)==ED_BUSY','EV_MUTANT=1'),
 'COMPLETE':(9,'ev_complete(&s,0,0)==ED_BUSY','EV_MUTANT=2'),
 'STRIP':(8,'ev_span(bytes,60,1,&length)==ED_OK','EV_MUTANT=3'),
 'CLEAN':(10,'s.device[0]==7','EV_MUTANT=4'),
 'ORDER':(12,'mask==98','EV_MUTANT=5'),
 'RETAINED_RX':(8,'out.first_length+out.second_length==96','ED_MUTATE_RX2'),
 'RETAINED_TX':(9,'ed_tx_publish(&retained,0,1,count_barrier,&barriers)==ED_BUSY','ED_MUTATE_TX'),
 'RETAINED_ABORT':(9,'retained.slots[1].phase==4','ED_MUTATE_TX2')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','assessor/ethernet_depth/ethernet_depth.c','labs/ethernet_followthrough/phy/phy.h','labs/ethernet_followthrough/phy/model.h','labs/ethernet_followthrough/phy/model.c','labs/ethernet_followthrough/phy/CONTRACTS.md','assessor/ethernet_followthrough/phy/phy.c','labs/bringup_depth/bringup.h','labs/bringup_depth/CONTRACTS.md','assessor/bringup_depth/bringup.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.S','assessor/execution/support.c']
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
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','port.compile','port.audit','bad-placement']}
 for m in ['REBUILD','PHY','CHAIN','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DEN_TARGET_MODEL',*['-I'+str(ROOT/n) for n in ['labs/ethernet_followthrough/ring','labs/ethernet_followthrough/phy','labs/ethernet_depth/include','labs/bringup_depth']]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;s=[*(PUBLIC/n if stub else PRIVATE/n for n in (['learner.c','target_learner.c','variants_learner.c'] if stub else ['ring.c','target.c','variants.c'])),*[ROOT/n for n in ['assessor/ethernet_followthrough/phy/phy.c','assessor/ethernet_depth/ethernet_depth.c','assessor/bringup_depth/bringup.c','labs/ethernet_followthrough/phy/model.c']],*[PUBLIC/n for n in ['model.c','tests.c','mapped.c','variants_tests.c','native.c']]]
  run(tag+'.compile',[cc,*flags,*opt,*s,'-o',b]);return b
 def target_args(d,cc,opt,definition=None,stub=False):
  a=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('phy','assessor/ethernet_followthrough/phy/phy.c'),('phy-public','labs/ethernet_followthrough/phy'),('ethernet','assessor/ethernet_depth/ethernet_depth.c'),('ethernet-include','labs/ethernet_depth/include'),('bringup','assessor/bringup_depth/bringup.c'),('bringup-include','labs/bringup_depth'),('boot','assessor/interrupt_depth/frame/target.S'),('runtime','assessor/execution/support.c')]:a+=['--'+k,ROOT/v]
  if not stub:
   for k,v in [('source','ring.c'),('target','target.c'),('variants','variants.c')]:a+=['--'+k,PRIVATE/v]
  if definition:a+=['--define',definition]
  return a
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;run(tag+'.build',target_args(d,cc,opt,definition,stub),marker='EN_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='EN_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'EN_GUEST_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS ring_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'EN_PORT_ASSERT:' if c=='port' else 'EN_VARIANT_ASSERT:' if c in ['chain','publication','visibility','order'] else 'EN_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    port=d/'physical-port.o';run(tag+'.port.compile',[cc,*(['--target=arm-none-eabi'] if cc=='clang' else []),'-std=c17','-O'+opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-Wall','-Wextra','-Werror',*['-I'+str(ROOT/n) for n in ['labs/ethernet_followthrough/ring','labs/ethernet_followthrough/phy','labs/ethernet_depth/include']],'-c',PRIVATE/'target.c','-o',port]);run(tag+'.port.audit',[sys.executable,PRIVATE/'port_audit.py',port],marker='"passed": true')
    run(tag+'.bad-placement',target_args(BUILD/(tag+'.bad-placement'),cc,opt)+['--bad-placement'],1,'EN_DMA_REACHABILITY')
  for m in ['REBUILD','PHY','CHAIN']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual author implementations and original criterion/changed experiments; no independent learner or external review claim.','MPS2 executes actual Thumb code with original RAM MMIO/DMA/DWT effects, not F439 silicon, electrical signaling, clock timing or physical frames.','Non-model object audits establish selected access/barrier code and helper closure, not physical bus execution.','Bounded callbacks/polls assume terminating callbacks; IRQs/debug pauses and continuous REF_CLK remain separate assumptions, no WCET claim.','Normal hardware ring uses4RX/2TX2048-byte buffers; original T39 two-/three-descriptor and noncoherent variants are explicitly separate models.','Changed CSTF adapter normalizes a validated stored capture length; changed-policy hardware FL/capture interpretation remains to be established before silicon use.','Physical route, wire/peer evidence, external review and independent seven-day learner transfer remain required.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
