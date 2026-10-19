"""Actual exported public heap scaffolds compile and reject on both selected ABIs."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_heap'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('algorithm-heap-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,exit=0,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==exit and (marker is None or marker in r.stdout);commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,expected_exit=exit,required_marker=marker,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T36-04';run('init',[sys.executable,'-m','cmastery','init','PR-T36-04',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T36-04.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/algorithm_heap';provider=workspace/'labs/algorithms';binary=workspace/'heap-tests';base=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),'-I'+rel(provider/'include'),'-I'+rel(workspace/'labs/containers_depth')]
 sources=[public/n for n in ['learner.c','learner_x86.S','suite.c','native.c','instrument.c','probe_x86.S','trace_x86.S']]+[provider/'src/x86_64.S',provider/'tests/abi_x86_64.S',provider/'adapters/checked_asm.c'];run('native.compile',prefix+['gcc',*base,*[rel(p) for p in sources],'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 for case in ['premise','scalar','observed','prior','minimum','ternary','binary']:run('native.rejection.'+case,prefix+[rel(binary),case],41,'AH_ASSERT:')
 image=workspace/'heap-thumb.elf';sources=[public/n for n in ['learner.c','learner_thumb.S','suite.c','thumb_start.c','instrument.c','probe_thumb.S']]+[provider/'src/thumb.S',provider/'tests/abi_thumb.S',provider/'adapters/checked_asm.c',workspace/'labs/containers_depth/byte_helpers.c'];run('thumb.compile',prefix+['arm-none-eabi-gcc',*base,'-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*[rel(p) for p in sources],'-T',rel(public/'link.ld'),'-Wl,--no-undefined','-o',rel(image)]);outputs[rel(image)]=sha(image)
 run('thumb.rejection',prefix+['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',rel(image)],1,'AH_THUMB_FAIL')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_algorithm_heap.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported native/Thumb stubs compile and reject all named native semantic cases without private implementations.','The prior heap remains a learner prerequisite; its import placeholders are explicitly unsolved. No learner mastery or elapsed delayed assessment inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual heap export eleven commands')
if __name__=='__main__':main()
