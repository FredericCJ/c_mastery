"""Actual public hardening export, compiled unfinished semantic controls."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_hardening'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('security-hardening-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and marker in r.stdout if marker else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T35-04';run('init',[sys.executable,'-m','cmastery','init','PR-T35-04',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T35-04.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/security_depth/hardening';binary=workspace/'hardening-tests'
 run('compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),*[rel(public/n) for n in ['learner.c','flawed.c','consume.c','tests.c']],'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 for case in ['bounds','policy']:run('rejection.'+case,prefix+[rel(binary),case],'SH_ASSERT:')
 handler=workspace/'handler-tests';run('handler.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-fno-stack-protector','-I'+rel(public),rel(public/'target_stub.c'),rel(public/'target_tests.c'),'-o',rel(handler)]);outputs[rel(handler)]=sha(handler);run('handler.rejection',prefix+[rel(handler)],'SH_ASSERT:bounded_failure_record')
 run('audit.rejection',[sys.executable,public/'audit_tests.py',public/'audit_stub.py',workspace/'fixtures/embedded/PR-T35-04.json'],'SH_ASSERT:declared_fixture')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_security_hardening.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported unfinished C repairs/handler and Python classifier; four intended semantic rejections.','No assessor implementation shipped or accepted learner work inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual hardening export seven commands')
if __name__=='__main__':main()
