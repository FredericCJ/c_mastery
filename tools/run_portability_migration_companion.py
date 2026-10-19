"""Separate legacy objects, prototype modes and intended diagnostic controls."""
from datetime import datetime,timezone
from pathlib import Path
import json,shutil,subprocess
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/migration_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/migration';private=ROOT/'assessor/language_portability/migration/migration.c'
 paths=['tools/run_portability_migration_companion.py','tools/common.py',private.relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('migration-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,negative=False,markers=()):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;log=runs/(f'{len(records):03d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_rejection=negative,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
  assert (p.returncode!=0 if negative else p.returncode==0) and all(m in out for m in markers),out
  return len(records)-1
 for tool in ['gcc','clang','nm','objdump']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],'version-'+tool)
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records);flags=[cc,opt,'-I'+str(public),'-Wall','-Wextra','-pedantic-errors']
   objects=[]
   if cc=='clang':execute([*flags,'-std=c90','-c',public/'old_consumer.c','-o',home/'strict-legacy.o'],'legacy-diagnostic-policy',True,['strict-prototypes'])
   for source,mode in [(public/'old_sum.c','c90'),(public/'old_consumer.c','c90'),(private,'c17'),(public/'old_driver.c','c17')]:
    obj=home/(source.stem+'.o');objects.append(obj);execute([*[('-pedantic' if f=='-pedantic-errors' and mode=='c90' and cc=='clang' else f) for f in flags],'-std='+mode,'-c',source,'-o',obj],source.stem+'-'+cc+opt)
   exe=home/'old-modern';execute([cc,*objects,'-o',exe],'old-modern-link');execute([exe],'old-modern-run',markers=['LP_OLD_MODERN 65618'])
   for obj in objects:execute(['nm',obj],'symbols');execute(['objdump','-dr',obj],'assembly')
   for mode,prototype in [('c17',False),('c17',True),('c23',True)]:
    stem=mode+('-prototype' if prototype else '-unprototyped');exe=home/stem
    execute([*[('-pedantic' if f=='-pedantic-errors' and mode=='c17' and not prototype and cc=='clang' else f) for f in flags],'-std='+mode,*(['-DLP_PROTOTYPE'] if prototype else []),public/'promotion_consumer.c',public/'promoted.c','-o',exe],stem+'-link');execute([exe],stem+'-run')
   execute([*flags,'-std=c23','-c',public/'promotion_consumer.c','-o',home/'c23-no.o'],'c23-empty-list',True,['lp_promoted','argument'])
   execute([*flags,'-std=c17','-DLP_CASE=1','-c',public/'negative.c','-o',home/'mismatch.o'],'header-mismatch',True,['lp_sum','conflicting'])
   execute([*flags,'-std=c17','-DLP_CASE=2','-c',public/'negative.c','-o',home/'void.o'],'void-argument',True,['no_arguments','argument'])
   execute([*flags,'-std=c17','-DLP_CASE=3','-c',public/'negative.c','-o',home/'cast.o'],'cast-analysis-only')
   execute([*flags,'-std=c17','-DLP_CASE=4','-c',public/'negative.c','-o',home/'implicit.o'],'implicit-type',True,['implicit_return'])
   profiles.append(dict(compiler=cc,optimization=opt,executions=list(range(start,len(records))),legacy_cases=65618,c17_float_promoted=True,c23_empty_list_rejected=True,mismatch_never_executed=True))
   for p in home.iterdir():
    if p.is_file():artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_migration_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C17 implementation baseline; C90/C23 are isolated compiler comparisons, not whole-edition conformance.','Incompatible cast body is compiled only and never invoked; missing prototype rules are tied to bounded draft passages.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
