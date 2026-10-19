"""Actual copied learner source compilation and intended rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_sync'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('serial-sync-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/'PR-T38-02';commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 assert sys.platform!='win32';run('init',[sys.executable,'-m','cmastery','init','PR-T38-02',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T38-02.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/serial_followthrough/sync/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/peripheral_depth/uart','-Ilabs/serial_depth/include',lab+'learner.c','labs/peripheral_depth/uart/uart.c','labs/serial_depth/src/serial_depth.c']
 run('tests.compile',[*common,lab+'tests.c',lab+'native.c','-o','sync-tests'],cwd=workspace)
 for case in range(4):run('tests.rejection.'+str(case),['./sync-tests',case],41,'SU_ASSERT:',workspace)
 run('mapped.compile',[*common,'-DPU_TARGET_MODEL','-DSU_TARGET_MODEL','labs/peripheral_depth/uart/target.c',lab+'mapped.c',lab+'mapped_native.c','-o','sync-mapped'],cwd=workspace);run('mapped.rejection',['./sync-mapped'],41,'SU_MAP_ASSERT:',workspace)
 for p in [workspace/'sync-tests',workspace/'sync-mapped']:outputs[rel(p)]=sha(p)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_serial_sync.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual independent workspace, public stubs only; no assessor source shipped.','No successful learner implementation, physical result or external review.']),indent=2)+'\n');print('PASS sync export eight commands')
if __name__=='__main__':main()
