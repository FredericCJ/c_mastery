"""Source-bound author observations for exact T38 changed serial tasks."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_diagnostic_ring';PUBLIC=ROOT/'labs/diagnostic_ring';PRIVATE=ROOT/'assessor/diagnostic_ring';BUILD=ROOT/'build'/('diagnostic-ring-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['tx','rx'];MARKERS=['DR_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'COPY':(0,'dr_send(&bridge, &m) == DG_OK && bridge.phase == 1',1),'QUARANTINE':(0,'dr_poll(&bridge, &raw) == DG_IO && bridge.phase == 3 && ring.tx_phase[bridge.ticket.slot] == 2',2),'COMPLETE':(0,'dr_poll(&bridge, &raw) == DG_BUSY && bridge.phase == 2 && raw == 123',3),'RELEASE':(1,'dr_receive(&bridge, &session, &out) == DG_OK && session.last == 1 && out.payload[1] == 0x55 && ring.released == 1 && ring.rx_held[0] == 0',4),'RELEASEFAIL':(1,'dr_receive(&bridge, &session, &out) == DG_IO && session.last == 1 && !memcmp(&prior, &out, sizeof out)',5)}
INPUTS={'diagnostic':'assessor/peripheral_depth/diagnostic/diagnostic.c','ethernetadapter':'assessor/peripheral_depth/diagnostic/ethernet.c','ring':'assessor/ethernet_followthrough/ring/ring.c','ringmodel':'labs/ethernet_followthrough/ring/model.c','phy':'assessor/ethernet_followthrough/phy/phy.c','phymodel':'labs/ethernet_followthrough/phy/model.c','ethernet':'assessor/ethernet_depth/ethernet_depth.c','bringup':'assessor/bringup_depth/bringup.c','foundation':'assessor/foundations/foundation.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c','labs-include':'labs','diagnostic-include':'labs/peripheral_depth/diagnostic','ring-include':'labs/ethernet_followthrough/ring','phy-include':'labs/ethernet_followthrough/phy','ethernet-include':'labs/ethernet_depth/include','bringup-include':'labs/bringup_depth','foundation-include':'labs/foundations/include'}
PROVIDERS=[INPUTS[n] for n in ['diagnostic','ethernetadapter','ring','ringmodel','phy','phymodel','ethernet','bringup','foundation']];INCLUDES=[v for k,v in INPUTS.items() if k.endswith('-include')]
IMPORTS=['tools/run_thread_lifecycle.py',*PROVIDERS,*[v for n,v in INPUTS.items() if 'include' not in n],*[p.as_posix() for d in INCLUDES for p in pathlib.Path(d).glob('*.h')]]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]))}
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
 for m in ['QUARANTINE','RELEASE','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'bridge.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',mutant=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'bridge.c']
  if mutant:args+=['--define','DR_MUTANT='+str(mutant)]
  run(tag+'.build',args,marker='DR_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='DR_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'DR_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'DR_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-DDR_MUTANT='+str(definition)]);run(tag+'.mutation.'+m,[b,i],41,'DR_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['QUARANTINE','RELEASE']:execute('thumb.'+m,target('thumb.'+m,mutant=MUTANTS[m][2]),True,'DR_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual unchanged DG codec/Ethernet adapter, EnRing/EpPhy, ED frame codec, bringup range and foundation cursor are linked.','Six native and four strict Thumb register-model executions; no physical Ethernet controller, PHY, cable or peer behavior is observed.','Copied TX admission, publication, local DMA completion and remote delivery remain separate; failed doorbell and RX release retain their explicit quarantine/publication rules.','CAN and USB routes remain independent; no learner delayed attempt or external review is supplied.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
