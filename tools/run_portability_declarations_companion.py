"""Isolated actual C23 declarations, C17 fallback and explicit wire ABI."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/declarations_companion_validation.json'
POSITIVE=[1,2,3,4,5,8,10,14];NEGATIVE=[6,7,9,11,12,13]
def main():
 public=ROOT/'labs/language_portability/declarations';private=ROOT/'assessor/language_portability/declarations/declarations.c'
 paths=['tools/run_portability_declarations_companion.py','tools/common.py',private.relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[];boundaries=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('declarations-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,expect=0,markers=()):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd and expect==0:
   dep=runs/(f'{len(records):03d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);out=p.stdout+p.stderr;log=runs/(f'{len(records):03d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expect,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==expect if expect is not None else p.returncode!=0) and all(m in out for m in markers),out
  if expect is None:assert 'error:' in out and 'internal compiler error' not in out
  if dep:
   deps=[Path(x).resolve() for x in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in deps]))
   for q in deps:
    if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
    else:external.setdefault(str(q),sha(q))
   artifacts.append(dict(path=dep.relative_to(ROOT).as_posix(),sha256=sha(dep)))
  return out
 for cc in ['gcc','clang']:
  tool=Path(shutil.which(cc)).resolve();external[str(tool)]=sha(tool);execute([cc,'--version'],cc+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records);flags=[opt,'-std=c23','-Wall','-Wextra','-pedantic-errors']
   execute([cc,*flags,'-DLD_PROBE=1','-E','-dM',public/'probes.c'],cc+'-macros',markers=['__STDC_VERSION__ 202311L'])
   for probe in range(1,15):
    obj=home/(str(probe)+'.o');exe=home/str(probe)
    execute([cc,*flags,'-DLD_PROBE='+str(probe),'-c',public/'probes.c','-o',obj],cc+'-probe'+str(probe),0 if probe in POSITIVE else None)
    if probe in POSITIVE:execute([cc,obj,'-o',exe],cc+'-link');execute([exe],cc+'-run')
   for probe in POSITIVE:
    execute([cc,opt,'-std=c17','-pedantic-errors','-DLD_PROBE='+str(probe),'-fsyntax-only',public/'probes.c'],cc+'-baseline-reject',None)
   obj=home/'false-vm.o';exe=home/'false-vm';execute([cc,*flags,'-DLD_PROBE=4','-DLD_VM_EXPECTATION=0','-c',public/'probes.c','-o',obj],cc+'-false-vm-compile');execute([cc,obj,'-o',exe],cc+'-false-vm-link');execute([exe],cc+'-false-vm-run',1)
   core=home/'core.o';execute([cc,opt,'-std=c17','-pedantic-errors','-I'+str(public),'-c',private,'-o',core],cc+'-fallback-core')
   execute([sys.executable,public/'generate_table.py',home/'table.h'],cc+'-generate-table')
   obj=home/'table.o';exe=home/'table';execute([cc,opt,'-std=c17','-pedantic-errors','-I'+str(public),'-I'+str(home),'-c',public/'table_main.c','-o',obj],cc+'-table-compile');execute([cc,core,obj,'-o',exe],cc+'-table-link');execute([exe],cc+'-table-run',markers=['LD_TABLE translation constants and runtime function agree for32 entries'])
   wrong=home/'wrong';wrong.mkdir();execute([sys.executable,public/'generate_table.py',wrong/'table.h','--mutant'],cc+'-table-mutant');execute([cc,opt,'-std=c17','-pedantic-errors','-I'+str(public),'-I'+str(wrong),'-c',public/'table_main.c','-o',wrong/'bad.o'],cc+'-table-mutant-reject',None,markers=['static assertion'])
   for producer in ['gcc','clang']:
    obj=home/(producer+'-provider.o');main=home/(producer+'-main.o');exe=home/(producer+'-boundary')
    execute([producer,opt,'-std=c23','-pedantic-errors','-I'+str(public),'-c',public/'bit_provider.c','-o',obj],producer+'-provider')
    execute([cc,opt,'-std=c17','-pedantic-errors','-I'+str(public),'-c',public/'boundary_main.c','-o',main],cc+'-consumer')
    execute([cc,core,obj,main,'-o',exe],cc+'-boundary-link');execute([exe],cc+'-boundary-run',markers=['LD_BOUNDARY 8194 values 65536 encodings equivalent explicit-standard-type-boundary'])
    execute(['readelf','-h',obj],cc+'-object-model',markers=['ELF64','little endian','Advanced Micro Devices X86-64'])
    symbols=execute(['nm','-u',obj],cc+'-undefined');assert not symbols.strip()
    execute(['objdump','-dr',obj],cc+'-assembly');execute(['size',exe],cc+'-size')
    runtime=execute(['ldd',exe],cc+'-runtime')
    for token in runtime.split():
     q=Path(token)
     if q.is_absolute() and q.is_file():external[str(q.resolve())]=sha(q.resolve())
    boundaries.append(dict(producer=producer,consumer=cc,optimization=opt,public_abi='int32_t and caller-owned octets only',encodings=65536,values=8194))
   profiles.append(dict(compiler=cc,optimization=opt,positive_probes=POSITIVE,diagnostic_rejections=NEGATIVE,baseline_rejections=POSITIVE,vm_false_expectation_rejected=True,corrupted_table_rejected=True,executions=list(range(start,len(records)))))
   for p in home.rglob('*'):
    if p.is_file():artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_declarations_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,compiler_dependencies=dependencies,boundaries=boundaries,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Exact published C17/C23 remain unavailable; N3096 is a working draft.','Opposite-compiler composition exposes only standard types on this recorded ABI; no bit-precise ABI guarantee.','Observed constexpr-function rejection is not relabeled a universally mandatory N3096 constraint diagnostic.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
