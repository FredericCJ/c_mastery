"""Compile actual exported native and target scaffolds, require semantic rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_algorithm_abi'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('algorithm-abi-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,exit=0,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==exit and (marker is None or marker in r.stdout);commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,expected_exit=exit,required_marker=marker,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T36-02';run('init',[sys.executable,'-m','cmastery','init','PR-T36-02',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T36-02.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/algorithm_abi';provider=workspace/'labs/algorithms';binary=workspace/'abi-tests';base=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),'-I'+rel(provider/'include')]
 sources=[public/n for n in ['learner.c','learner_x86.S','callback_probe_x86.S','callback_entry.c','poison_x86.S','native.c']]+[provider/'src/x86_64.S',provider/'tests/abi_x86_64.S'];run('native.compile',prefix+['gcc',*base,*[rel(p) for p in sources],'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 for case in ['search','abi','callback','windows','records','guard','scale']:run('native.rejection.'+case,prefix+[rel(binary),case],41,'AB_ASSERT:')
 image=workspace/'abi-thumb.elf';sources=[public/n for n in ['learner.c','learner_thumb.S','callback_probe_thumb.S','callback_entry.c','poison_thumb.S','thumb_start.c']]+[provider/'src/thumb.S',provider/'tests/abi_thumb.S',workspace/'labs/containers_depth/byte_helpers.c'];run('thumb.compile',prefix+['arm-none-eabi-gcc',*base,'-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*[rel(p) for p in sources],'-T',rel(provider/'thumb/link.ld'),'-Wl,--no-undefined','-o',rel(image)]);outputs[rel(image)]=sha(image)
 run('thumb.rejection',prefix+['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',rel(image)],1,'AB_THUMB_FAIL')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_algorithm_abi.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported native/Thumb stubs compile and reject named semantic cases; no private source shipped.','Fresh independent learner completion is still required; author export checks do not satisfy the seven-day delayed assessment.']),indent=2)+'\n',encoding='utf8');print('PASS actual ABI export eleven commands')
if __name__=='__main__':main()
