"""Source-bound ordinary and changed compiler/control mechanisms."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_vm_compiler';PUBLIC=ROOT/'labs/runtime_followthrough/compiler';PRIVATE=ROOT/'assessor/runtime_followthrough/compiler';BUILD=ROOT/'build'/('vm-compiler-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['names','scopes','bounds','branches','controls','fuel']
MUTANTS={'SHADOW':(1,'cvm_run(&vm, &program, &result) == CVM_OK && result.tag == tag && result.integer == expected',1),'INITIALIZER':(0,'compile(s, 256, fold) == CVM_OK',2),'TARGET':(3,'compile(s, 256, fold) == CVM_OK',3),'EAGER':(3,'cvm_run(&vm, &program, &result) == CVM_OK && result.tag == tag && result.integer == expected',4),'BREAK':(4,'cvm_run(&vm, &program, &result) == CVM_OK && result.tag == tag && result.integer == expected',5),'FUEL':(5,'compile("(fn main () (+ 2 3))", 64, 0) == CVM_OK && report.folded == 0 && program.length == 12',6),'CANARY':(0,'!memcmp(&prior, &program, sizeof program)',7)}
PROVIDERS=['assessor/runtime_followthrough/verifier/verifier.c','assessor/runtime/cvm.c','assessor/runtime/compiler.c'];INCLUDES=['labs/runtime_followthrough/compiler','labs/runtime_followthrough/verifier','labs/runtime/include'];IMPORTS=['tools/run_thread_lifecycle.py',*PROVIDERS,'labs/runtime/include/cvm.h','labs/runtime/SPEC.md','labs/runtime_followthrough/verifier/verifier.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set([p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]))}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','audit.compile','audit.ast','audit.acyclic','audit.recursive.ast','audit.recursive.refusal','gcc.lto.compile','gcc.lto.oracle','gcc.lto.resources'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','resources','oracle','stub.compile',*CASES,*['stub.'+c for c in CASES]]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*['-I'+str(ROOT/n) for n in INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;run(tag+'.compile',[cc,*flags,*opt,PUBLIC/'learner.c' if stub else PRIVATE/'compiler.c',*[ROOT/n for n in PROVIDERS],PUBLIC/'tests.c',PUBLIC/'native.c','-o',b]);return b
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt);run(tag+'.resources',[b,'--resources'],marker='VC_RESOURCES')
    for i,case in enumerate(CASES):run(tag+'.'+case,[b,i],marker='VC_'+case.upper()+'_PASS')
    run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',b,BUILD/(tag+'.packets.json')],marker='"packets": 469')
    b=compile(tag+'.stub',cc,opt,True)
    for i,case in enumerate(CASES):run(tag+'.stub.'+case,[b,i],41,'VC_ASSERT:')
    for m,(i,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-DVC_MUTANT='+str(definition)]);run(tag+'.mutation.'+m,[b,i],41,'VC_ASSERT:'+marker)
  b=compile('gcc.lto','gcc',['-O3','-flto','-DNDEBUG']);run('gcc.lto.oracle',[sys.executable,PRIVATE/'oracle.py',b,BUILD/'gcc.lto.packets.json'],marker='"packets": 469');run('gcc.lto.resources',[b,'--resources'],marker='VC_RESOURCES')
  run('audit.compile',['gcc',*flags,'-O0','-fno-inline','-fstack-usage','-c',PRIVATE/'compiler.c','-o',BUILD/'compiler.o'])
  run('audit.ast',['clang',*flags,'-Xclang','-ast-dump=json','-fsyntax-only',PRIVATE/'compiler.c'])
  run('audit.acyclic',[sys.executable,PRIVATE/'stack.py',BUILD/'audit.ast.log',BUILD/'compiler.su'],marker='"passed": true')
  run('audit.recursive.ast',['clang',*flags,'-Xclang','-ast-dump=json','-fsyntax-only',PUBLIC/'recursive_control.c'])
  run('audit.recursive.refusal',[sys.executable,PRIVATE/'stack.py',BUILD/'audit.recursive.ast.log',BUILD/'compiler.su'],41,'VC_RECURSION:vc_recursive_control')
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as e:failure=str(e)
 if before!=pins() or ext!=external():failure='Scoped source identity changed'
 limits=['Hosted C17 compiler; actual unchanged CVM verifier/VM and retained compiler are linked.','Independent tree/bytecode models cover53 selected programs,106 lowerings and469 invocations per profile; finite evidence is not universal equivalence.','Own AST call graph and GCC static frames exclude verifier/libc frames and caller-owned workspace; no freestanding compiler or physical target claim.','No external review or independent minimum-seven-day learner assessment is supplied.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(report['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
