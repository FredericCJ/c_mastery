"""Actual isolated learner exports of both required physical milestones."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_mcp'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('mcp-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 for project in ['PR-T38-11','PR-T38-12']:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  lab='labs/serial_followthrough/mcp/';run(project+'.tests.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+lab,'-Ilabs/serial_depth/include','-Ilabs/peripheral_depth/spi','-Ilabs/peripheral_depth/uart',*[lab+n for n in ['learner.c','spi_adapter.c','tests.c','native.c']],'labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/uart/uart.c','-o','mcp-tests'],cwd=workspace)
  for case in range(8):run(project+'.tests.rejection.'+str(case),['./mcp-tests',case],41,'MC_ASSERT:',workspace)
  outputs[rel(workspace/'mcp-tests')]=sha(workspace/'mcp-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_serial_mcp.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspaces, no assessor shipped.','Compiling stub and rejecting behavior is not successful learner assessment.','No board or peer connected/flashed/measured.']),indent=2)+'\n');print('PASS MCP exports20 commands')
if __name__=='__main__':main()
