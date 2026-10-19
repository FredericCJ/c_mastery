"""Strict Cortex-M4 compiler/link closure and actual QEMU arena cases."""
from pathlib import Path
from datetime import datetime,timezone
import json,shutil,subprocess,time
from common import ROOT,dump,sha
REPORT='research/v5/expansion/b02_followthrough/region_target_validation.json'
def main():
 base=ROOT/'labs/b02_followthrough/region';private=ROOT/'assessor/b02_followthrough/region';runs=ROOT/'reviews/v5/expansion/b02_followthrough'/('arena-target-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 paths=['tools/run_b02_region_target.py','tools/common.py','labs/b02_depth/include/depth.h','assessor/b02_depth/depth.c','labs/modern_bridge/include/bridge.h','assessor/modern_bridge/bridge.c','labs/b02_followthrough/arena/arena.h','assessor/b02_followthrough/arena/arena.c','labs/containers_depth/byte_helpers.c','labs/b02_followthrough/map/abi_helpers.c']+[p.relative_to(ROOT).as_posix() for p in base.iterdir() if p.is_file()]+[(private/'region.c').relative_to(ROOT).as_posix()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[]
 def execute(cmd,label,timeout=60):
  cmd=list(map(str,cmd));start=time.monotonic()
  try:p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=timeout);code=p.returncode;out=p.stdout+p.stderr
  except subprocess.TimeoutExpired as e:code=None;out='TIMEOUT '+str(e)
  path=runs/(f'{len(records):04d}-'+label+'.log');path.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=code,seconds=time.monotonic()-start,log=path.relative_to(ROOT).as_posix(),log_sha256=sha(path)));return code,out
 for name in ['arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','clang','qemu-system-arm']:
  p=Path(shutil.which(name)).resolve();external[str(p)]=sha(p);assert execute([name,'--version'],'version-'+name)[0]==0
 for name,cc,opt,mutant in [('gcc-O0','arm-none-eabi-gcc','-O0',0),('gcc-O3','arm-none-eabi-gcc','-O3',0),('clang-O3','clang','-O3',0),('mutant-slice','arm-none-eabi-gcc','-O3',4),('learner','arm-none-eabi-gcc','-O0',0)]:
  home=runs/name;home.mkdir();flags=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Werror','-I'+str(base),'-I'+str(ROOT/'labs/b02_depth/include'),'-I'+str(ROOT/'labs/modern_bridge/include'),'-I'+str(ROOT/'labs/b02_followthrough/arena')]
  if cc=='clang':flags.insert(0,'--target=arm-none-eabi')
  sources=[base/'entry.S',base/'target.c',ROOT/'assessor/b02_depth/depth.c',ROOT/'assessor/modern_bridge/bridge.c',ROOT/'assessor/b02_followthrough/arena/arena.c',(base if name=='learner' else private)/'region.c',ROOT/'labs/containers_depth/byte_helpers.c',ROOT/'labs/b02_followthrough/map/abi_helpers.c'];objects=[];deps={};symbols={}
  for i,source in enumerate(sources):
   obj=home/(str(i)+'.o');objects.append(obj);cmd=[cc,*flags,*(['-DBF_REGION_MUTANT='+str(mutant)] if mutant else []),'-MD','-MF',str(obj)+'.d','-c',source,'-o',obj];code,out=execute(cmd,name+'-compile');assert code==0,out
   for token in Path(str(obj)+'.d').read_text().replace('\\\n',' ').split(':',1)[1].split():
    p=Path(token)
    if p.is_file():deps[str(p)]=sha(p)
   code,out=execute(['arm-none-eabi-nm','-u',obj],name+'-object-symbols');assert code==0;symbols[str(i)]=out.split();allowed={'U','_sdata','_edata','_sidata','_sbss','_ebss','bf_target','b2_arena_init','b2_reserve','b2_mark','b2_rollback','bf_arena_init','bf_arena_reserve','bf_arena_get','bf_arena_set','bf_arena_mark','bf_arena_register','bf_arena_rollback','bf_arena_commit','mb_region_init','mb_region_alloc','mb_region_read','mb_region_reset','mb_region_cleanup','bf_cohort_init','bf_cohort_batch','bf_cohort_reset','bf_tree_init','bf_tree_child_reset','bf_tree_reset','bf_slice_init','bf_slice_alloc','bf_slice_read','bf_slice_reset','bf_growth_init','bf_growth_alloc','bf_growth_read','bf_growth_reset','bf_retry_init','bf_retry_reserve','bf_retry_register','bf_retry_reset','memcpy','memset','__aeabi_memclr4','__aeabi_memcpy4'};assert set(out.split())<=allowed,out
   assert execute(['arm-none-eabi-objdump','-dr',obj],name+'-object-disassembly')[0]==0
  image=home/'image.elf';code,out=execute(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,--gc-sections','-Wl,-Map='+str(home/'image.map'),'-T',base/'target.ld',*objects,'-o',image],name+'-link');assert code==0,out
  code,out=execute(['arm-none-eabi-nm','-u',image],name+'-image-symbols');assert code==0 and not out.strip(),out
  code,out=execute(['arm-none-eabi-readelf','-d',image],name+'-image-dynamic');assert code==0 and 'NEEDED' not in out
  code,out=execute(['arm-none-eabi-readelf','-h',image],name+'-image-header');assert code==0 and 'ARM' in out and 'ELF32' in out
  cmd=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',image];code,out=execute(cmd,name+'-execute',20);positive=name not in ['mutant-slice','learner'];assert code==(0 if positive else 1) and ('BF_TARGET_PASS' in out if positive else 'BF_TARGET_CHECK' in out),out
  profiles.append(dict(profile=name,passed=True,compiler=cc,optimization=opt,actual_execution=True,semantic_rejection=not positive,execution=len(records)-1,object_symbols=symbols,compiler_dependencies=deps))
  for p,h in deps.items():
   path=Path(p)
   if path.is_relative_to(ROOT):inputs.setdefault(path.relative_to(ROOT).as_posix(),h)
   else:external.setdefault(p,h)
  for p in home.rglob('*'):
   if p.is_file():artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_b02_region_target.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Actual QEMU M4 semihosting checks, not a physical board observation.','Selected aggregate copies use explicitly linked original byte and Arm helpers; all object and image unresolved symbols audited.','No arbitrary-implementation or finite-test proof.']))
 print(dict(passed=True,commands=len(records),profiles=len(profiles)))
if __name__=='__main__':main()
