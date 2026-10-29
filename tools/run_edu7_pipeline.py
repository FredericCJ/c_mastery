"""Execute actual retained EDU7 and explicit boundary/route/representation mechanisms."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_edu7_pipeline';PUBLIC=ROOT/'labs/edu7_followthrough/pipeline';PRIVATE=ROOT/'assessor/edu7_followthrough/pipeline';BUILD=ROOT/'build'/('edu7-pipeline-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['exchange','identities','faults','adversarial']
MARKERS=['EW_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'HOP':(1,'got==1&&f.payload[3]','EW_MUTANT=1'),'LINK':(1,'event(&world.b','EW_MUTANT=2'),'PHYSICAL':(0,'ew_physical_verdict','EW_MUTANT=3'),'PLANT':(0,'world.a.endpoint.response.verb==3&&world.a.endpoint.response.request==44','EW_MUTANT=4'),'RESTART':(1,'ew_rebind(&world.a,2)','EW_MUTANT=5'),'TOKENSTALL':(3,'world.a.upper_drops==1','EW_MUTANT=6'),'DUPLICATE':(0,'request(2,42,-4000,1,3','ES_MUTANT=9')}
INCLUDE_MAP={'session':'labs/edu7_followthrough/session','transport':'labs/edu7_followthrough/transport','physical':'labs/edu7_followthrough/physical','foundation':'labs/edu7_followthrough/foundation','channel':'labs/serial_followthrough/rs485','uart':'labs/serial_followthrough/uart','byte':'labs/peripheral_depth/uart','serial':'labs/serial_depth/include','codec':'labs/communications/include','depth':'labs/edu7_depth/include'}
PROVIDER_MAP={'session':'assessor/edu7_followthrough/session/session.c','transport':'assessor/edu7_followthrough/transport/transport.c','physical':'assessor/edu7_followthrough/physical/physical.c','foundation':'assessor/edu7_followthrough/foundation/foundation.c','channel':'assessor/serial_followthrough/rs485/channel.c','byte':'assessor/peripheral_depth/uart/uart.c','serial':'assessor/serial_depth/serial_depth.c','codec':'assessor/communications/edu7.c','depth':'assessor/edu7_depth/edu7_depth.c'}
PROVIDERS=list(PROVIDER_MAP.values());INCLUDES=list(INCLUDE_MAP.values())
INPUTS={**PROVIDER_MAP,'direction':'assessor/serial_followthrough/rs485/port.c','byte-port':'assessor/peripheral_depth/uart/target.c','port':'assessor/edu7_followthrough/physical/port.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c',**{n+'-include':v for n,v in INCLUDE_MAP.items()}}
IMPORTS=['tools/run_thread_lifecycle.py','research/enhancement/networking_contracts.json',*PROVIDERS,*INPUTS.values(),*[p.as_posix() for d in INCLUDES for p in pathlib.Path(d).glob('*.h')],*[d+'/CONTRACTS.md' for d in INCLUDES]]
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
 for m in ['PHYSICAL','RESTART','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'pipeline.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False,real=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  args+=['--board-source',PRIVATE/'board.c']
  if not stub:args+=['--source',PRIVATE/'pipeline.c']
  if real:args+=['--board']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='EW_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='EW_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'EW_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'EW_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,'EW_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
    real=target(tag+'.real',cc,opt,real=True);assert not run(tag+'.real.closure',['arm-none-eabi-nm','-u',real/'image.elf']).strip();run(tag+'.real.disassembly',['arm-none-eabi-objdump','-d',real/'image.elf'])
  for m in ['PHYSICAL','RESTART']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,'EW_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual three independent node controls, four struct pu_driver/struct ep_tx owners, copied struct ef_route relay, struct et_tx/struct et_rx, struct e7_session and original application.','Six host and four Cortex-M4 struct lr_cpu model executions; selected F439 adapter final-linked separately without physical execution.','Actual software UART model captures and no-overlap counters do not measure cable/DE/termination; whole-family physical verdict remains pending.','Independent learner/external review not performed; finite author tests do not prove absolute mastery or universal liveness.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
