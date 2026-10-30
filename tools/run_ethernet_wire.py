"""Execute actual retained Ethernet and new record/tag/bridge mechanisms."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_wire';PUBLIC=ROOT/'labs/ethernet_followthrough/wire';PRIVATE=ROOT/'assessor/ethernet_followthrough/wire';BUILD=ROOT/'build'/('ethernet-wire-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['stream','vlan','frame','bridge','loop','layers']
MUTANTS={'SUFFIX':(0,'retained_suffix','EW_MUTANT=1'),'TAG':(1,'tag_layout','EW_MUTANT=2'),'CRC':(1,'padding_integrity','EW_MUTANT=3'),'INGRESS':(3,'flood','EW_MUTANT=4'),'EVICT':(3,'oldest_evict','EW_MUTANT=5'),'ENDIAN':(0,'split_tail','EW_MUTANT=6'),'PAD':(2,'zero_padding','ED_MUTATE_FRAME'),'FRAMECRC':(2,'pad_not_payload','ED_MUTATE_FRAME2'),'WRAP':(5,'encapsulation','ED_MUTATE_LAYERS'),'SERVICE':(5,'stream_mismatch','ED_MUTATE_LAYERS2')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','assessor/ethernet_depth/ethernet_depth.c','labs/interrupt_depth/frame/target.S','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c','assessor/serial_followthrough/mcp/compare.c']
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
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['SUFFIX','CRC','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/ethernet_depth/include')]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'wire.c',ROOT/('labs/ethernet_depth/src/ethernet_depth.c' if stub else 'assessor/ethernet_depth/ethernet_depth.c'),PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in [('ethernet','labs/ethernet_depth/src/ethernet_depth.c' if stub else 'assessor/ethernet_depth/ethernet_depth.c'),('ethernet-include','labs/ethernet_depth/include'),('boot','assessor/interrupt_depth/frame/target.S'),('runtime','assessor/execution/support.c'),('compare','assessor/serial_followthrough/mcp/compare.c')]:args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'wire.c']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='WIRE_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='EW_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'WIRE_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker='OBS wire_'+c);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'EW_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],41,'EW_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['SUFFIX','CRC']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][2]),True,'EW_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual original C providers, native and selected M4 CPU execution; no physical Ethernet, negotiation or board observation.','Single-tag historical CFI=0 profile and finite bridge; not current full IEEE conformance, VLAN-aware bridging or spanning tree.','Original source/route diagrams still require learner reasoning; author tests cannot award independent mastery.','T39-08/T40 and mixed parent/sidecar reconciliation remain required.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
