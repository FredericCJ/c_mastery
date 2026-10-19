"""Source-bound author observations for exact T38 changed serial tasks."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_retry';PUBLIC=ROOT/'labs/serial_retry';PRIVATE=ROOT/'assessor/serial_retry';BUILD=ROOT/'build'/('serial-retry-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['stretch','transaction','retry'];MARKERS=['SR_'+c.upper()+'_PASS' for c in CASES]
MUTANTS={'BUDGET':(0,'sr_timer_step(&t,(start+n-1)%256,&d)==SR_OK&&t.remaining==1',1),'STRETCH':(0,'sr_byte_sample(&s,0,1,1,1)==SR_TIMEOUT&&s.bit.bit==0',2),'READBIT':(1,'t.action==SD_RESTART_READ&&t.wire==addresses[a]*2+1',3),'BUSFREE':(2,'sr_retry_poll(&r,255,1,1,1)==SR_PENDING',4),'UNCERTAIN':(2,'sr_retry_complete(&r,2,2,PI_TIMEOUT,1)==SR_UNCERTAIN&&r.attempts==1',5),'ATTEMPTS':(2,'sr_retry_complete(&r,2,2,PI_ADDRESS_NACK,0)==SR_EXHAUSTED',6),'IDLE':(2,'sr_retry_poll(&r,10,1,1,0)==SR_PENDING&&r.attempts==0',7)}
INPUTS={'serial':'assessor/serial_depth/serial_depth.c','bits':'assessor/b045_systems/systems.c','timing':'assessor/legacy_constrained/timing.c','i2c':'assessor/peripheral_depth/i2c/i2c.c','boot':'labs/network_followthrough/driver/target.S','runtime':'assessor/execution/support.c','compare':'assessor/serial_followthrough/mcp/compare.c','serial-include':'labs/serial_depth/include','bits-include':'labs/b045_systems/include','timing-include':'labs/constrained_depth/timing','i2c-include':'labs/peripheral_depth/i2c','uart-include':'labs/peripheral_depth/uart'}
PROVIDERS=[INPUTS[n] for n in ['serial','bits','timing','i2c']];INCLUDES=[INPUTS[n] for n in ['serial-include','bits-include','timing-include','i2c-include','uart-include']]
IMPORTS=['tools/run_thread_lifecycle.py',*PROVIDERS,*[v for n,v in INPUTS.items() if 'include' not in n],*[p.as_posix() for d in INCLUDES for p in pathlib.Path(d).glob('*.h')]]

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]))}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','clang.target.atomic.refusal','gcc.provider.optimized.refusal'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.provider.'+str(i) for i in range(4)};out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack']}
 for m in ['BUDGET','READBIT','UNCERTAIN','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in INCLUDES]]
 provider_objects={}
 def compile(tag,cc,opt,stub=False):
  base='.'.join(tag.split('.')[:2])
  if base not in provider_objects:
   objects=[]
   for i,n in enumerate(PROVIDERS):
    obj=BUILD/(base+'.provider.'+str(i)+'.o');provider_opt=[x for x in opt if not x.startswith('-DSR_MUTANT')]
    if cc=='gcc' and n==INPUTS['bits']:provider_opt=[*provider_opt,'-O0']
    run(base+'.provider.'+str(i),[cc,*[f for f in flags if not f.startswith('-Wl,')],*provider_opt,'-c',ROOT/n,'-o',obj]);objects.append(obj)
   provider_objects[base]=objects
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'retry.c',*provider_objects[base],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',mutant=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt]
  for k,v in INPUTS.items():args+=['--'+k,ROOT/v]
  if not stub:args+=['--source',PRIVATE/'retry.c']
  if mutant:args+=['--define','SR_MUTANT='+str(mutant)]
  run(tag+'.build',args,marker='SR_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False,marker='SR_ASSERT:'):
  run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',d/'image.elf'],41 if bad else 0,marker if bad else 'SR_GUEST_PASS')
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES):run(tag+'.stub.'+c,[b,i],41,'SR_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-DSR_MUTANT='+str(definition)]);run(tag+'.mutation.'+m,[b,i],41,'SR_ASSERT:'+marker)
  for cc in ['arm-none-eabi-gcc']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true')
  for m in ['BUDGET','READBIT','UNCERTAIN']:execute('thumb.'+m,target('thumb.'+m,mutant=MUTANTS[m][2]),True,'SR_ASSERT:'+MUTANTS[m][1])
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  run('gcc.provider.optimized.refusal',['gcc',*[f for f in flags if not f.startswith('-Wl,')],'-O3','-c',ROOT/INPUTS['bits'],'-o',BUILD/'gcc-provider-refused.o'],1,'may be used uninitialized')
  run('clang.target.atomic.refusal',['clang','--target=arm-none-eabi','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-I'+str(ROOT/INPUTS['timing-include']),'-c',ROOT/INPUTS['timing'],'-o',BUILD/'clang-timing-refused.o'],1,'large atomic operation')
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Retained B045 whole-TU strict GCC O3 refusal in unrelated s_reduce is recorded; its unchanged provider is compiled at O0 in GCC profiles, adapter/tests retain declared optimization.', 'Actual T30 ct_elapsed, B045 SI2c, SDI2c and Pi retry policy are linked unchanged. Six native and two GCC Thumb model executions; no physical I2C.','The full unchanged T30 translation unit rejects strict Clang M4 compilation for unused 64-bit atomics. GCC final section-GC images have no atomic helpers or undefined symbols.','Timeout does not authorize bus clear, reset or freeing a hardware lease. Trusted monitor proves STOP/initial idle; true polling gaps below half range and no missed low edges are external premises.','Author execution does not administer delayed learner assessment or establish absolute mastery.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
