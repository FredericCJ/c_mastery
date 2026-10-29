"""Execute actual retained EDU7 and explicit boundary/route/representation mechanisms."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_edu7_session';PUBLIC=ROOT/'labs/edu7_followthrough/session';PRIVATE=ROOT/'assessor/edu7_followthrough/session';BUILD=ROOT/'build'/('edu7-session-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['dialogue','receipts','resume']
MARKERS=['ES_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'EARLYTOKEN':(0,'!es_permitted(&a)','ES_MUTANT=1'),'NEGOTIATE':(0,'es_deliver(&y','ES_MUTANT=2'),'EARLYCOMMIT':(1,'es_deliver(&b,80,&e)==E7_REJECTED && b.core.receipt','ES_MUTANT=3'),'CAPACITY':(1,'es_deliver(&b,1','ES_MUTANT=4'),'CANCEL':(0,'es_uncertain(&c,1,2)','ES_MUTANT=5'),'DIRECTION':(1,'b.core.receipt[0]','E7_MUTATE_SESSION2'),'EPOCH':(2,'es_deliver(&b,80,&e)==E7_STALE','ES_MUTANT=7'),'REBOOT':(2,'raw(&a,&reboot','ES_MUTANT=8'),'DUPLICATE':(1,'es_emit(&a,3,7,1,1,request,n,4','ES_MUTANT=9')}
IMPORTS=['assessor/edu7_followthrough/transport/transport.c','labs/edu7_followthrough/transport/transport.h','labs/edu7_followthrough/transport/learner.c','labs/edu7_followthrough/transport/CONTRACTS.md','tools/run_thread_lifecycle.py','labs/communications/include/com_edu7.h','labs/edu7_depth/include/edu7_depth.h','labs/edu7_depth/src/edu7_depth.c','labs/edu7_depth/CONTRACTS.md','research/enhancement/networking_contracts.json','assessor/communications/edu7.c','assessor/edu7_depth/edu7_depth.c','labs/network_followthrough/driver/target.S','assessor/execution/support.c','assessor/serial_followthrough/mcp/compare.c']

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')}|{p for p in (ROOT/'build/posix-io-toolchain/sysroot').rglob('*') if p.is_file() and (p.suffix in ['.h','.o','.a'] or '.so' in p.name)})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['EARLYTOKEN','CAPACITY','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 out|={'i386.compile','i386.abi',*[('i386.'+c) for c in CASES],*[('i386.'+c+'.oracle') for c in CASES]}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/edu7_followthrough/transport'),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/edu7_depth/include')]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'session.c',ROOT/'assessor/edu7_followthrough/transport/transport.c',ROOT/'assessor/communications/edu7.c',ROOT/'assessor/edu7_depth/edu7_depth.c',PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('transport','assessor/edu7_followthrough/transport/transport.c'),('transport-include','labs/edu7_followthrough/transport'),('codec','assessor/communications/edu7.c'),('codec-include','labs/communications/include'),('depth','assessor/edu7_depth/edu7_depth.c'),('depth-include','labs/edu7_depth/include'),('boot','labs/network_followthrough/driver/target.S'),('runtime','assessor/execution/support.c'),('compare','assessor/serial_followthrough/mcp/compare.c')]:args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'session.c']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='ES_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='ES_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'ES_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'ES_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,'ES_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['EARLYTOKEN','CAPACITY']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,'ES_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  root=ROOT/'build/posix-io-toolchain/sysroot';libs=root/'usr/lib32';loader=libs/'ld-linux.so.2';gccdir=root/'usr/lib/gcc/x86_64-linux-gnu/15/32'
  b=compile('i386','gcc',['-O3','-m32','--sysroot='+str(root),'-isystem',str(root/'usr/include/x86_64-linux-gnu'),'-B'+str(gccdir)+'/', '-L'+str(libs),'-Wl,--dynamic-linker='+str(loader)])
  run('i386.abi',['readelf','-h',b],marker='ELF32')
  for i,c in enumerate(CASES):run('i386.'+c,[loader,'--library-path',libs,b,i],marker=MARKERS[i]);run('i386.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/('i386.'+c+'.log'),c],marker='"status": "pass"')
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual two independent peers compose struct e7_session, struct et_tx/struct et_rx and original codecs/application; no shared global permission state.','Six host profiles, actual i386 and four Cortex-M4 executions are bounded struct lr_cpu observations, not physical RS485 or whole routed pipeline.','Original session wire is preserved; protocol0x84 lower epoch is explicit and unauthenticated. RAM receipts cannot promise durable external exactly-once effects.','Independent learner, external review and physical observations remain unperformed; PR08 and mixed-parent reconciliation remain required.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
