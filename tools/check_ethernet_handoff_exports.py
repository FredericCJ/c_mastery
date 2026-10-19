"""Actual learner export, semantic stub controls and explicit missing-import failure."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_handoff'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('handoff-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();project='PR-T39-08';workspace=folder/project
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and(marker is None or marker in r.stdout);commands.append(dict(id=project+'.'+name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 run('init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/ethernet_followthrough/handoff/';flags=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/ethernet_depth/include']
 run('tests.compile',[*flags,'-DEH_CORE_ONLY',*[lab+n for n in ['learner.c','tests.c','native.c']],'labs/ethernet_depth/src/ethernet_depth.c','-o','handoff-core'],cwd=workspace)
 for case in range(4):run('tests.rejection.'+str(case),['./handoff-core',case],41,'EH_ASSERT:',workspace)
 sys.path.insert(0,str(ROOT/'labs/ethernet_followthrough/handoff'));from run import INCLUDES
 for name in ['stack','reassembly']:run(name+'.compile',[*flags,*['-I'+n for n in INCLUDES],'-c',lab+name+'.c','-o',name+'.o'],cwd=workspace)
 (workspace/'missing.json').write_text('{}');run('providers.missing',[sys.executable,lab+'run.py','--root','.', '--providers','missing.json','--build','missing-providers'],2,'missing explicit providers',workspace)
 for n in ['handoff-core','stack.o','reassembly.o','missing.json']:outputs[rel(workspace/n)]=sha(workspace/n)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_ethernet_handoff.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspace; private assessor sources absent.','Core stubs compile and fail semantically; actual consumer syntax compiles against exported public contracts.','Complete stack link requires explicit completed learner imports and is deliberately refused without them; no fabricated learner completion.']),indent=2)+'\n');print('PASS handoff exports9 commands')
if __name__=='__main__':main()
