"""Compile and reject copied learner adapter/frame stubs without private code."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_frame'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('architecture-frame-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/'PR-T37-07';commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 assert sys.platform!='win32';run('init',[sys.executable,'-m','cmastery','init','PR-T37-07',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T37-07.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/architecture_frame/';run('native.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-Ilabs/architecture_frame','-Ilabs/interrupt_depth/frame',lab+'learner.c','labs/interrupt_depth/frame/frame.c',lab+'tests.c',lab+'native.c','-o','frame-tests'],cwd=workspace)
 for case in ['reset','capture','ownership','nested']:run('native.rejection.'+case,['./frame-tests',case],41,'MF_ASSERT:',workspace)
 run('target.build',[sys.executable,lab+'target_build.py','--build','target','--runtime','labs/containers_depth/byte_helpers.c'],marker='MF_TARGET_BUILD_PASS',cwd=workspace)
 run('target.rejection',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel','target/image.elf'],1,'MF_TARGET_FAIL',workspace)
 for p in [workspace/'frame-tests',*[p for p in (workspace/'target').iterdir() if p.is_file()]]:outputs[rel(p)]=sha(p)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_architecture_frame.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['All builds execute inside actual exported workspace with copied public sources.','Initial implementations compile and fail intended semantic assertions; no private decoder/driver/solution shipped.','No learner success or delayed/physical evidence inferred.']),indent=2)+'\n');print('PASS frame export eight commands')
if __name__=='__main__':main()
