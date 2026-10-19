"""Actual original fixture, imported decoder/startup and changed frame campaign."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_frame';PUBLIC=ROOT/'labs/architecture_frame';PRIVATE=ROOT/'assessor/architecture_frame';FRAME=ROOT/'labs/interrupt_depth/frame';REF=ROOT/'assessor/interrupt_depth/frame';BUILD=ROOT/'build'/('architecture-frame-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['reset','capture','ownership','nested'];PACKETS=dict(reset=5,capture=7,ownership=2,nested=1);PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS={'RESET':('reset','r.reset_fetch[0]==0 && r.reset_fetch[1]==4','MF_MUTANT_RESET'),'IRQ':('reset','r.exception==21 && r.irq_vector==0x08000054','MF_MUTANT_IRQ'),'POLICY':('capture','MF_UNSUPPORTED && equal(&f,&old,sizeof f)','MF_MUTANT_POLICY'),'LR':('capture','f.stacked_lr==0x08000111','MF_MUTANT_LR'),'R4':('ownership','r.hardware==0x1d00f','MF_MUTANT_R4'),'STACK':('nested','r.first_stack==0 && r.second_stack==1','MF_MUTANT_STACK'),'FP_OFFSET':('capture','mf_capture(w,size+pad,sp,er,1,1,material,1,0,&f)==MF_OK','XF_MUTANT=1'),'FP_LAZY':('capture','f.fp_materialized==material','XF_MUTANT=3')}
IMPORTS=['tools/run_thread_lifecycle.py','fixtures/algorithms/PR-T37-07.json','assessor/execution/support.c',*['labs/interrupt_depth/frame/'+n for n in ['frame.h','frame.c','target.c','target.S','target.ld','CONTRACTS.md']],*['assessor/interrupt_depth/frame/'+n for n in ['frame.c','target.c','target.S','fp_callee.c','target_oracle.py']]]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-objdump','arm-none-eabi-objcopy','arm-none-eabi-nm','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def target_tags():return [(cc,opt,fp,cc+'.O'+opt+'.fp'+str(fp)) for cc in ['arm-none-eabi-gcc','clang'] for opt in ['0','3'] for fp in [0,1]]
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.stack'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for _,_,_,tag in target_tags():out|={tag+'.'+n for n in ['build','execute','closure','disassembly','vectors','oracle','stack']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(FRAME)]
 def sources(stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'ledger.c',FRAME/'frame.c' if stub else REF/'frame.c',PUBLIC/'tests.c',PUBLIC/'native.c']
 def native(tag,cc,flags,stub=False):
  binary=BUILD/tag;run(tag+'.compile',[cc,*strict,*flags,*sources(stub),'-o',binary]);harness.OUTPUTS.add(binary);return binary
 def target(tag,cc='arm-none-eabi-gcc',opt='0',fp=0,definition=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--cc',cc,'--opt',opt,'--runtime',ROOT/'assessor/execution/support.c']
  if not stub:args+=['--source',PRIVATE/'ledger.c','--frame',REF/'frame.c','--driver',REF/'target.c','--boot',REF/'target.S']
  if fp:args+=['--fp-callee',REF/'fp_callee.c']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='MF_TARGET_BUILD_PASS');return d
 def execute(tag,d,bad=False):
  return run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-d','int','-D',d/'interrupts.log','-kernel',d/'image.elf'],1 if bad else 0,'MF_TARGET_FAIL' if bad else 'XF_TARGET_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for cc,opt,fp,tag in target_tags():
   d=target(tag,cc,opt,fp);text=execute(tag,d);assert 'MF_TARGET_PREFLIGHT_PASS' in text;assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.vectors',[sys.executable,PRIVATE/'vectors.py',d],marker='"passed": true');run(tag+'.oracle',[sys.executable,REF/'target_oracle.py',BUILD/(tag+'.execute.log'),d/'interrupts.log',fp],marker='XF_TARGET_ORACLE_PASS cases=16 tailchains=24');run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d,fp],marker='"passed": true')
  for m,(_,marker,definition) in MUTANTS.items():
   tag='thumb.'+m;d=target(tag,definition=definition);text=execute(tag,d,True);assert marker in text
  execute('thumb.stub',target('thumb.stub',stub=True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=native(tag,cc,flags)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=native(tag+'.stub',cc,flags,True)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'MF_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():mutant=native(tag+'.mutation.'+m,cc,[*flags,'-D'+definition]);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,independent_packets=PACKETS,reset_fetch=[0,4],extended_core_offset=0,physical=False,actual_producer='XfFrame and xf_priority'))
   obj=BUILD/(cc+'.core.o');stack=obj.with_suffix('.su');run(cc+'.core',[cc,*strict,'-O2','-fstack-usage','-ffreestanding','-fno-builtin','-c',PRIVATE/'ledger.c','-o',obj]);harness.OUTPUTS.update([obj,stack]);u=run(cc+'.core.undefined',['nm','-u',obj]);s={q.split()[-1] for q in u.splitlines()};assert {'xf_frame','xf_priority'}<=s and s<={'xf_frame','xf_priority','memcpy','memset','_GLOBAL_OFFSET_TABLE_'};run(cc+'.core.stack',['cat',stack])
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 files={p for p in BUILD.rglob('*') if p.is_file()}|harness.OUTPUTS;limits=['Exact three PR-T37-07 ordinary/changed pairs; PR08 remains separately required.','Actual frozen Xf decoder, priority converter, reset/vector/IRQ driver and optional FP callee compiled unchanged except main-symbol renaming for adapter preflight.','Eight actual MPS2 profiles, each16 frame/group cases: no F439 physical reset, alias switching, BOOT pin or errata observation.','F439 Flash-bank1/SRAM1 reset labels and remapping are a source-inspected admission model; no bank2/system-loader/other boot guarantee.','No physical latency, whole interrupt system, external review or independently delayed learner claim.'];r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':main()
