"""Explicit cumulative learner providers; public async and provenance workflow."""
from pathlib import Path
import argparse,hashlib,json,shlex,subprocess
from provenance import record
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=HERE/'learner.c')
for k in ['cvm','compiler','execution','verifier','services','store','adapter']:p.add_argument('--'+k,type=Path,required=True)
p.add_argument('--cc',default='gcc');p.add_argument('--build',type=Path,required=True);a=p.parse_args();a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=True);rows=[]
inc=['-I'+str(p) for p in [HERE,LABS/'runtime/include',LABS/'runtime/adapters',LABS/'runtime_followthrough/execution',LABS/'runtime_followthrough/verifier',LABS/'runtime_followthrough/services',LABS/'storage/include']]
flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*inc];compiler=a.build/'compiler';vm=a.build/'vm';async_exe=a.build/'async'
for tag,sources,defines,exe in [('compiler',[a.compiler,a.cvm,HERE/'chain_driver.c'],['-DVP_COMPILER'],compiler),('vm',[a.cvm,HERE/'chain_driver.c'],[],vm),('async',[a.source,*[getattr(a,k) for k in ['cvm','execution','verifier','services','store','adapter']],HERE/'async_tests.c'],['-pthread'],async_exe)]:
 cmd=[a.cc,*flags,*defines,*map(str,sources),'-o',str(exe)];r=subprocess.run(cmd,capture_output=True,text=True);rows.append(dict(command=cmd,returncode=r.returncode,stdout=r.stdout,stderr=r.stderr));print(r.stdout+r.stderr,end='')
 if r.returncode:raise SystemExit(r.returncode)
r=subprocess.run([str(async_exe)]); 
if r.returncode:raise SystemExit(r.returncode)
files={Path(__file__),HERE/'provenance.py',HERE/'chain_driver.c',HERE/'async_tests.c',a.source,*[getattr(a,k) for k in ['cvm','compiler','execution','verifier','services','store','adapter']]}
# Exact included public headers, and actual compiler executable, are build inputs.
for folder in [HERE,LABS/'runtime/include',LABS/'runtime/adapters',LABS/'runtime_followthrough/execution',LABS/'runtime_followthrough/verifier',LABS/'runtime_followthrough/services',LABS/'storage/include']:files.update(folder.glob('*.h'))
import shutil
files.add(Path(shutil.which(a.cc)));sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
build=a.build/'build.json';build.write_text(json.dumps(dict(passed=True,commands=rows,inputs={str(p.resolve()):sha(p) for p in files},outputs={str(p):sha(p) for p in [compiler,vm,async_exe]}),indent=2)+'\n')
d=record(HERE/'source.cvm',compiler,vm,build,a.build/'chain',1);print('VP_CHAIN_PASS source_compile_verify_image_VM_result42')
