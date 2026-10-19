"""Actual exported public sort scaffolds: native and Thumb semantic rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_sort'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('algorithm-sort-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,exit=0,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==exit and (marker is None or marker in r.stdout);commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,expected_exit=exit,required_marker=marker,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T36-03';run('init',[sys.executable,'-m','cmastery','init','PR-T36-03',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T36-03.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/algorithm_sort';provider=workspace/'labs/algorithms';binary=workspace/'sort-tests';base=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),'-I'+rel(provider/'include')]
 sources=[public/n for n in ['learner.c','learner_x86.S','suite.c','native.c','instrument.c','wide_probe_x86.S','trace_x86.S']]+[provider/'src/x86_64.S',provider/'tests/abi_x86_64.S',provider/'adapters/checked_asm.c'];run('native.compile',prefix+['gcc',*base,*[rel(p) for p in sources],'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 for case in ['insertion','counts','descending','wide','failure','prior']:run('native.rejection.'+case,prefix+[rel(binary),case],41,'AS_ASSERT:')
 image=workspace/'sort-thumb.elf';sources=[public/n for n in ['learner.c','learner_thumb.S','suite.c','thumb_start.c','instrument.c','wide_probe_thumb.S']]+[provider/'src/thumb.S',provider/'tests/abi_thumb.S',provider/'adapters/checked_asm.c',workspace/'labs/containers_depth/byte_helpers.c'];run('thumb.compile',prefix+['arm-none-eabi-gcc',*base,'-O2','-DAS_NO_PRIOR','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*[rel(p) for p in sources],'-T',rel(public/'link.ld'),'-Wl,--no-undefined','-o',rel(image)]);outputs[rel(image)]=sha(image)
 run('thumb.rejection',prefix+['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',rel(image)],1,'AS_THUMB_FAIL')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_algorithm_sort.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported native/Thumb stubs compile and reject semantic cases without private implementations.','Prior artifacts remain learner prerequisites; no learner mastery or elapsed delayed assessment inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual sorting export ten commands')
if __name__=='__main__':main()
