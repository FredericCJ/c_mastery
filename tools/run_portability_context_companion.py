"""Observe exact authority-related mode/header diagnostics and immutable history."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/context_companion_validation.json'

def main():
 public=ROOT/'labs/language_portability/context';private=ROOT/'assessor/language_portability/context/ledger.py'
 paths=['tools/run_portability_context_companion.py','tools/common.py',private.relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('context-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,negative=False,markers=()):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and any(x.endswith('.c') for x in cmd) and '-E' not in cmd:
   dep=runs/(f'{len(records):03d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;log=runs/(f'{len(records):03d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_rejection=negative,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode!=0 if negative else p.returncode==0) and all(m in out for m in markers),out
  if dep and dep.is_file():
   text=dep.read_text().replace('\\\n',' ');paths=[Path(x).resolve() for x in shlex.split(text.split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
    else:external.setdefault(str(q),sha(q))
   artifacts.append(dict(path=dep.relative_to(ROOT).as_posix(),sha256=sha(dep)))
  return len(records)-1
 for tool in ['gcc','clang']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],'version-'+tool)
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records);flags=[cc,opt,'-Wall','-Wextra','-pedantic-errors']
   for mode,header,negative in [('c17',True,False),('c17',False,True),('c23',True,False),('c23',False,False)]:
    stem=mode+('-header' if header else '-noheader');exe=home/stem
    execute([*flags,'-std='+mode,*(['-DLC_HEADER'] if header else []),public/'bool.c','-o',exe],stem+'-compile',negative,['bool'] if negative else [])
    if not negative:execute([exe],stem+'-run',markers=['BOOL_MACRO' if mode=='c17' else 'BOOL_NONMACRO'])
    execute([cc,'-std='+mode,*(['-DLC_HEADER'] if header else []),'-E','-dM',public/'bool.c'],stem+'-macros')
   for case in [0,4]:
    exe=home/f'constant{case}';execute([*flags,'-std=c23','-DLC_CASE='+str(case),public/'constexpr.c','-o',exe],'constexpr-positive');execute([exe],'constexpr-run')
   for case in [1,2,3]:execute([*flags,'-std=c23','-DLC_CASE='+str(case),'-c',public/'constexpr.c','-o',home/'rejected.o'],'constexpr-constraint'+str(case),True,['constexpr'])
   execute([*flags,'-std=c17','-DLC_CASE=4','-c',public/'constexpr.c','-o',home/'wrong-mode.o'],'constexpr-mode',True,['constexpr'])
   exe=home/'cleanup';execute([*flags,'-std=gnu17',public/'cleanup.c','-o',exe],'cleanup-extension');execute([exe],'cleanup-run',markers=['LC_CLEANUP normal21 early21'])
   profiles.append(dict(compiler=cc,optimization=opt,executions=list(range(start,len(records))),bool_modes=4,constexpr_valid=2,constexpr_constraints=3,wrong_mode_rejected=True,extension_cleanup=True))
   for p in home.iterdir():
    if p.is_file():artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 execute([sys.executable,public/'ledger_tests.py',private],'ledger-reference',markers=['LC_LEDGER preserved_history rejected6 deep_copy'])
 execute([sys.executable,public/'ledger_tests.py',public/'ledger.py'],'ledger-learner',True,['LC_LEDGER_CHECK unfinished'])
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_context_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,compiler_dependencies=dependencies,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C17 implementation baseline; C23 and GNU17 probes measure only named capabilities.','Synthetic issue update is labeled original; source authority and interpretation still require the separate analysis rubric.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))

if __name__=='__main__':main()
