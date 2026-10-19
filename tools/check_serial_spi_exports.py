"""Actual isolated learner export; original implementations remain withheld."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_spi'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 assert sys.platform!='win32';folder=ROOT/'build'/('serial-spi-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 project='PR-T38-05';workspace=folder/project;run('init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/serial_followthrough/spi/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+lab,'-Ilabs/peripheral_depth/spi','-Ilabs/peripheral_depth/uart','-Ilabs/serial_depth/include','-Ilabs/algorithms/include',lab+'learner.c',lab+'ports_learner.c',lab+'ring_prerequisite.c','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/uart/uart.c','labs/serial_depth/src/serial_depth.c']
 run('tests.compile',[*common,lab+'tests.c',lab+'native.c','-o','spi-tests'],cwd=workspace)
 for case in range(5):run('tests.rejection.'+str(case),['./spi-tests',case],41,'SS_ASSERT:',workspace)
 run('mapped.compile',[*common,'-DPS_TARGET_MODEL','labs/peripheral_depth/spi/target.c',lab+'mapped.c',lab+'mapped_native.c','-o','spi-mapped'],cwd=workspace);run('mapped.rejection',['./spi-mapped'],41,'SS_MAP_ASSERT:',workspace)
 for n in ['spi-tests','spi-mapped']:p=workspace/n;outputs[rel(p)]=sha(p)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_serial_spi.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspace; public stubs and contracts only.','No assessor implementation or successful independent learner attempt.']),indent=2)+'\n');print('PASS SPI export9 commands')
if __name__=='__main__':main()
