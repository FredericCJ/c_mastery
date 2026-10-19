"""Actual public artifact-policy export; no private implementation shipped."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_artifacts'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('security-artifacts-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and marker in r.stdout if marker else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T35-06';run('init',[sys.executable,'-m','cmastery','init','PR-T35-06',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T35-06.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/security_depth/artifacts';command=workspace/'labs/security_depth/command';binary=workspace/'artifact-tests';ssl='/home/frede/cmastery-security-sources/openssl-dev/sysroot/usr/include'
 run('compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),'-I'+rel(command),'-I'+ssl,'-I'+ssl+'/x86_64-linux-gnu',rel(public/'learner.c'),rel(public/'tests.c'),rel(command/'learner.c'),'/usr/lib/x86_64-linux-gnu/libcrypto.so.3','-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 for case in ['artifact','rotation','service','shared','integrated']:run('rejection.'+case,prefix+[rel(binary),case],'SA_ASSERT:')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_security_artifacts.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported learner stubs compile and reject five semantic cases; no private implementation shipped.','Declared hosted OpenSSL provider remains a dependency; no learner mastery inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual artifacts export seven commands')
if __name__=='__main__':main()
