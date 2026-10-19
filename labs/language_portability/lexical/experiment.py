"""Original compiler experiment; records stages without claiming learner understanding."""
import argparse,hashlib,json,shlex,subprocess,shutil
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);p.add_argument('--mode',choices=['c17','c23'],required=True);p.add_argument('--optimization',default='-O0');p.add_argument('--output',required=True);a=p.parse_args();source=Path(__file__).resolve().parent;work=Path(a.output).resolve();work.mkdir(parents=True,exist_ok=False);rows=[];artifacts=[]
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def run(cmd,label):
 r=subprocess.run(list(map(str,cmd)),text=True,capture_output=True,timeout=60);log=work/(str(len(rows))+'-'+label+'.log');log.write_text(r.stdout+r.stderr,encoding='utf8');rows.append(dict(command=list(map(str,cmd)),exit_code=r.returncode,log=str(log),log_sha256=sha(log)));return r
flags=[a.compiler,'-std='+a.mode,a.optimization,'-Wall','-Wextra','-pedantic-errors']
observed={}
for name in ['splice','trigraph','literals']:
 src=source/(name+'.c');pp=run([*flags,'-E','-P',src],name+'-preprocess');assert pp.returncode==0
 # Display normalization only; no claim to implement a general C tokenizer.
 observed[name+'_preprocessed']=pp.stdout
 exe=work/name;r=run([*flags,src,'-o',exe],name+'-compile-link');assert r.returncode==0
 observed[name+'_exit']=run([exe],name+'-execute').returncode
bad=work/'bad';bad.mkdir();shutil.copyfile(source/'declaration.c',bad/'declaration.c');(bad/'config.h').write_text('#define TYPE int,\n')
pp=run([*flags,'-E','-P',bad/'declaration.c'],'bad-preprocess');observed['bad_preprocess_exit']=pp.returncode;observed['bad_tokens']=pp.stdout
r=run([*flags,'-fsyntax-only',bad/'declaration.c'],'bad-compile');observed['bad_compile_exit']=r.returncode
(bad/'bad-config.h').write_bytes((bad/'config.h').read_bytes());(bad/'config.h').write_text('#define TYPE int\n');pp=run([*flags,'-E','-P',bad/'declaration.c'],'repair-preprocess');assert pp.returncode==0;observed['repair_tokens']=pp.stdout
exe=bad/'fixed';assert run([*flags,bad/'declaration.c','-o',exe],'repair-link').returncode==0;observed['repair_exit']=run([exe],'repair-execute').returncode
# Actual depfile-driven incremental build. Rehash each declared dependency.
build=work/'dependency';build.mkdir();[shutil.copyfile(source/n,build/n) for n in ['a.c','b.c','main.c','config.h']];known={};events=[]
for step in range(3):
 if step==2:(build/'config.h').write_text('#define TYPE long\n')
 compiled=[]
 for name in ['a','b','main']:
  old=known.get(name);changed=old is None or any(sha(k)!=h for k,h in old.items())
  if changed:
   obj=build/(name+'.o');dep=build/(name+'.d');r=run([*flags,'-MD','-MF',dep,'-c',build/(name+'.c'),'-o',obj],'dependency-'+name);assert r.returncode==0
   deps=shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]);known[name]={str(Path(k).resolve()):sha(k) for k in deps};compiled.append(name)
 if compiled:assert run([a.compiler,*[build/(n+'.o') for n in ['a','b','main']],'-o',build/'app'],'dependency-link').returncode==0
 result=run([build/'app'],'dependency-run');events.append(dict(compiled=compiled,exit_code=result.returncode,object_sha256={n:sha(build/(n+'.o')) for n in ['a','b','main']},dependencies={n:dict(v) for n,v in known.items()}))
 (build/('config'+str(step)+'.h')).write_bytes((build/'config.h').read_bytes())
 for name in ['a','b','main']:shutil.copyfile(build/(name+'.o'),build/(name+str(step)+'.o'))
observed['dependency_events']=events
for path in work.rglob('*'):
 if path.is_file():artifacts.append(dict(path=str(path),sha256=sha(path)))
report=dict(schema_version=1,compiler=a.compiler,mode=a.mode,optimization=a.optimization,executions=rows,observed=observed,artifacts=artifacts)
(work/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(report=str(work/'report.json'),commands=len(rows))))
