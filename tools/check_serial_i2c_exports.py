"""Actual isolated CLI learner exports; compilation succeeds, unfinished semantics fail."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_i2c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 assert sys.platform!='win32';folder=ROOT/'build'/('serial-i2c-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 for project in ['PR-T38-03','PR-T38-04']:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  lab='labs/serial_followthrough/i2c/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/peripheral_depth/i2c','-Ilabs/peripheral_depth/uart','-Ilabs/serial_depth/include']
  run(project+'.pure.compile',[*common,lab+'learner.c',lab+'electrical.c',lab+'electrical_native.c','-o','pure'],cwd=workspace)
  for i in range(2):run(project+'.pure.rejection.'+str(i),['./pure',i],41,'IA_ASSERT:',workspace)
  common += [lab+'learner.c',lab+'clear_learner.c','labs/peripheral_depth/i2c/i2c.c','labs/peripheral_depth/uart/uart.c','labs/serial_depth/src/serial_depth.c']
  run(project+'.tests.compile',[*common,lab+'electrical.c',lab+'transaction.c',lab+'cases.c',lab+'native.c','-o','i2c-tests'],cwd=workspace)
  for case in range(2,6):run(project+'.tests.rejection.'+str(case),['./i2c-tests',case],41,'IA_TRANSACTION_ASSERT:',workspace)
  run(project+'.mapped.compile',[*common,'-DPI_TARGET_MODEL','labs/peripheral_depth/i2c/target.c',lab+'mapped.c',lab+'mapped_native.c','-o','i2c-mapped'],cwd=workspace);run(project+'.mapped.rejection',['./i2c-mapped'],41,'IA_MAP_ASSERT:',workspace)
  for n in ['pure','i2c-tests','i2c-mapped']:p=workspace/n;outputs[rel(p)]=sha(p)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_serial_i2c.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Two actual isolated learner workspaces; public stubs and contracts only.','No assessor implementation shipped or successful learner attempt claimed.']),indent=2)+'\n');print('PASS I2C export22 commands')
if __name__=='__main__':main()
