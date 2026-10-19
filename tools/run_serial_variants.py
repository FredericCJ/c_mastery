"""Source-bound author observations for exact T38 changed serial tasks."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_variants';PUBLIC=ROOT/'labs/serial_variants';PRIVATE=ROOT/'assessor/serial_variants';BUILD=ROOT/'build'/('serial-variants-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['baud','word','capture','receiver','stream','policy','bank'];MARKERS=['SV_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'FRACTION':(0,'out.integer==out.units/64u&&out.fraction==out.units%64u',1),'PARITY':(1,'out.length==1u+bits+stop&&out.payload==payload&&out.edges==2u*(bits-!l)&&out.wire[0]==0',2),'HOLD':(2,'sv_capture(p,h,160,e,18,&out)==SV_IO&&out.index==h&&out.defects==8',3),'HYSTERESIS':(3,'sv_receiver(-120,0,1,0,0,0,0,SV_HIGH,&value)==SV_OK&&value==SV_UNKNOWN',4),'GENERATION':(4,'sv_stream_feed(&s,2,bits+k,n-k,&used,&out)==SV_STALE',5),'EOF':(5,'sv_policy(0,0,1,0,0,0,&out)==SV_OK&&out.action==SV_ACCEPT&&out.overload==1',6),'FORMAT':(6,'sv_bank(0,&a,0,0x7f0,&filter)==SV_OK&&filter.first==0x24000000&&filter.second==0xfe000006',7)}
INPUTS={'serial':'assessor/serial_depth/serial_depth.c','codec':'assessor/serial_followthrough/can/codec.c','fault':'assessor/serial_followthrough/can/fault.c','controller':'assessor/serial_followthrough/can/controller.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c','serial-include':'labs/serial_depth/include','codec-include':'labs/serial_followthrough/can'}
PROVIDERS=[INPUTS[n] for n in ['serial','codec','fault','controller']];INCLUDES=[INPUTS[n] for n in ['serial-include','codec-include']]
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
 for m in ['FRACTION','GENERATION','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'variants.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',mutant=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'variants.c']
  if mutant:args+=['--define','SV_MUTANT='+str(mutant)]
  run(tag+'.build',args,marker='SV_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='SV_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'SV_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'SV_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-DSV_MUTANT='+str(definition)]);run(tag+'.mutation.'+m,[b,i],41,'SV_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['FRACTION','GENERATION']:execute('thumb.'+m,target('thumb.'+m,mutant=MUTANTS[m][2]),True,'SV_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual retained serial codec/FIFO and CAN stream/fault/controller queue are linked; six native and four Thumb model executions only.','Selected RP2040 divider and F439 word/filter policies are model execution, not real selected-peripheral or electrical observations.','Author evidence does not administer seven-day independent learner transfer or establish absolute mastery.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
