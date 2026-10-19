"""Actual nonzero realloc and bounded correction/mode observations."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/resize_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/resize';private=ROOT/'assessor/language_portability/resize/resize.c';cap=ROOT/'labs/language_portability/capability'
 paths=['tools/run_portability_resize_companion.py','tools/common.py','fixtures/language/PR-T05-03.json',private.relative_to(ROOT).as_posix(),(cap/'features.c').relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[];observations=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('resize-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,markers=()):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd:
   dep=runs/(f'{len(records):03d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);out=p.stdout+p.stderr;log=runs/(f'{len(records):03d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==0 and all(m in out for m in markers),out
  if dep:
   paths=[Path(x).resolve() for x in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
    else:external.setdefault(str(q),sha(q))
   artifacts.append(dict(path=dep.relative_to(ROOT).as_posix(),sha256=sha(dep)))
  return out
 for cc in ['gcc','clang']:
  tool=Path(shutil.which(cc)).resolve();external[str(tool)]=sha(tool);execute([cc,'--version'],cc+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records)
   for mode in ['c11','c17']:
    flags=[opt,'-std='+mode,'-Wall','-Wextra','-pedantic-errors','-I'+str(public)]
    objects=[]
    for i,src in enumerate([private,public/'libc_adapter.c']):
     obj=home/(mode+str(i)+'.o');execute([cc,*flags,'-c',src,'-o',obj],mode+'-compile');objects.append(obj)
    exe=home/mode;execute([cc,*objects,'-o',exe],mode+'-link');execute([exe],mode+'-run',markers=['LP_REALLOC nonzero-only failure-preserves grow-shrink explicit-release'])
    linked=execute(['ldd',exe],mode+'-runtime-closure')
    for token in linked.split():
     q=Path(token)
     if q.is_absolute() and q.is_file():external[str(q.resolve())]=sha(q.resolve())
    obj=home/(mode+'-temporary.o');execute([cc,*flags,'-c',public/'temporary.c','-o',obj],mode+'-temporary-compile');exe=home/(mode+'-temporary');execute([cc,obj,'-o',exe],mode+'-temporary-link');out=execute([exe],mode+'-temporary-run',markers=['returned_value=42']);assert out.startswith(('temporary_address_equal=0 ','temporary_address_equal=1 '));observations.append(dict(kind='temporary-address',compiler=cc,optimization=opt,mode=mode,output=out,portable_equality_expected=False))
    execute([cc,*flags,'-DLP_FEATURE=3','-E','-dM',cap/'features.c'],mode+'-version-macros')
   for feature in [2,4]:
    obj=home/('earlier'+str(feature)+'.o');execute([cc,opt,'-std=c11','-pedantic-errors','-DLP_FEATURE='+str(feature),'-c',cap/'features.c','-o',obj],'earlier-compile');exe=home/('earlier'+str(feature));execute([cc,obj,'-o',exe],'earlier-link');execute([exe],'earlier-run')
   profiles.append(dict(compiler=cc,optimization=opt,executions=list(range(start,len(records))),nonzero_realloc_modes=['c11','c17'],temporary_modes=['c11','c17'],earlier_features=[2,4]))
   for p in home.iterdir():
    if p.is_file():artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 fixture=read_json(ROOT/'fixtures/language/PR-T05-03.json');year=fixture['input']['selected'];actual=dict(introduced=[r['introduced']<=year for r in fixture['input']['claims']],correction_applies=[r['introduced']<=r['corrected']<=year for r in fixture['input']['claims']]);assert actual==fixture['expected']
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_resize_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,compiler_dependencies=dependencies,observations=observations,synthetic_applicability=actual,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Never calls libc realloc with zero; original differing zero-policy adapters are independently exercised by main tests.','Temporary address equality is recorded without a universal golden; exact published C17 remains unavailable.','Installed compiler execution is not GCC8 execution or a whole-library conformance claim.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
