"""Record actual strict/extended variadic and hygiene phase behavior."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/wrappers_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/wrappers';private=ROOT/'assessor/language_portability/wrappers';inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.iterdir() if p.is_file()}
 for p in ['tools/run_portability_wrappers_companion.py','tools/common.py']:inputs[p]=sha(ROOT/p)
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('wrappers-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];profiles=[];external={};dependencies=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-o' in cmd and expected==0:dep=home/(str(len(records))+'-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);out=p.stdout+p.stderr;log=home/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,out
  if dep:
   paths=[Path(q).resolve() for q in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if not q.is_relative_to(ROOT):external[str(q)]=sha(q)
  if Path(cmd[0]).is_relative_to(home):
   runtime,_=execute(['ldd',cmd[0]],label+'-runtime')
   for token in runtime.split():
    q=Path(token)
    if q.is_absolute() and q.is_file():external[str(q.resolve())]=sha(q.resolve())
  return p.stdout,p.stderr
 for cc in ['gcc','clang','ldd']:
  tool=Path(shutil.which(cc)).resolve();external[str(tool)]=sha(tool);execute([cc,'--version'],cc+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for opt in ['O0','O3']:
   label=cc+'-'+opt;start=len(records);observed={}
   for case in range(1,9):
    mode='c23' if case==3 else 'gnu17' if case in [4,5,8] else 'c17';flags=[cc,'-std='+mode,'-'+opt,'-Wall','-Wextra','-Werror']+([] if mode=='gnu17' else ['-pedantic-errors'])+['-DCASE='+str(case)]
    out,err=execute([*flags,'-E','-P',public/'probes.c'],label+'-preprocess'+str(case),1 if case==2 else 0);observed[str(case)]=dict(mode=mode,preprocessed=out,preprocess_exit=1 if case==2 else 0)
    if case==2:assert 'error:' in err and ('argument' in err or 'variadic' in err);continue
    if case==4:
     assert ''.join(out.split())=='target(0)target(0,)target(0,)target(0,)';continue
    if case in [1,6,7]:
     out,err=execute([*flags,'-fsyntax-only',public/'probes.c'],label+'-semantic'+str(case),1);assert 'error:' in err
     assert ('argument' in err if case==1 else 'else' in err if case==6 else 'redefinition' in err)
     observed[str(case)]['compile_exit']=1;continue
    exe=home/(label+'-'+str(case));execute([*flags,public/'probes.c','-o',exe],label+'-compile'+str(case));out,_=execute([exe],label+'-execute'+str(case));assert out==('1 1 1 1 3\n' if case==3 else '');observed[str(case)]['stdout']=out
   out,err=execute([cc,'-std=c17','-pedantic-errors','-DCASE=3','-E','-P',public/'probes.c'],label+'-strict-vaopt',1);assert 'error:' in err;profiles.append(dict(profile=label,executions=list(range(start,len(records))),observed=observed,strict_vaopt_rejected=True,passed=True))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,executions=records,profiles=profiles,compiler_dependencies=dependencies,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Compiler/mode observations are separate from exact published edition access.','GNU dangling-comma samples are preprocessed only;no invalid executable result.','__COUNTER__ is implementation-specific;no claim of ISO namespace hygiene.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
