"""Source-bound author observations for exact T39/T40 changed service tasks."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_link_transfer';PUBLIC=ROOT/'labs/link_transfer';PRIVATE=ROOT/'assessor/link_transfer';BUILD=ROOT/'build'/('link-transfer-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['serial','sizes','borrow','bridge','phy','partial'];MARKERS=['LT_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'MASK':(0,'r == LT_OK && v == expected',1),'TRAILER':(1,'lt_sizes(12, p, 2, out) == LT_OK',2),'PREFIX':(2,'r == (used == n + 2 ? LT_DONE : LT_OK)',3),'VID':(3,'lt_bridge(&s, 10, b, a, 3, 2, &m)',4),'SPEED':(4,'lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_OK',5),'EARLY':(5,'lt_offer(&s.lower, &p, &n) == LT_OK && n == 2',6)}
INPUTS={'ethernet':'assessor/ethernet_depth/ethernet_depth.c','sender':'assessor/edu7_depth/edu7_depth.c','codec':'assessor/communications/edu7.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c','ethernet-include':'labs/ethernet_depth/include','sender-include':'labs/edu7_depth/include','codec-include':'labs/communications/include'}
PROVIDERS=[INPUTS[n] for n in ['ethernet','sender','codec']];INCLUDES=[INPUTS[n] for n in ['ethernet-include','sender-include','codec-include']]
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
 for m in ['MASK','EARLY','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'transfer.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',mutant=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'transfer.c']
  if mutant:args+=['--define','LT_MUTANT='+str(mutant)]
  run(tag+'.build',args,marker='LT_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='LT_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'LT_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'LT_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-DLT_MUTANT='+str(definition)]);run(tag+'.mutation.'+m,[b,i],41,'LT_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['MASK','EARLY']:execute('thumb.'+m,target('thumb.'+m,mutant=MUTANTS[m][2]),True,'LT_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual retained sender, fragment codec, bridge and MDIO routines are linked; six native and four Thumb model executions only.','Selected changed DP83848-EP query is not a physical driver/board observation; no external communication.','Author evidence does not administer seven-day independent learner transfer or establish absolute mastery.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
