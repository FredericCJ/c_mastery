"""Actual representation/dictionary/cache observations and executed Linux i386 ABI probe."""
from pathlib import Path
from datetime import datetime, timezone
import argparse, hashlib, json, statistics, struct, subprocess, time
ROOT=Path(__file__).resolve().parents[1]
PATHS=['labs/dispatch/tests/cost_measure.c','labs/dispatch/tests/cost_methods.c','labs/dispatch/tests/abi32.c','labs/dispatch/tests/abi32.S','labs/dispatch/include/dispatch.h','labs/dispatch/include/dispatch_resolution.h','labs/dod/include/dod.h','assessor/dispatch/dispatch.c','assessor/dispatch/dispatch_resolution.c','assessor/dod/dod.c','tools/run_dispatch_costs.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args();report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
 stamp='dispatch-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True)
 pins={p:sha(ROOT/p) for p in PATHS};records=[];measurements=[];layouts=[];abi=[]
 def execute(command,label,binary=False):
  command=list(map(str,command));start=time.perf_counter_ns();r=subprocess.run(command,cwd=ROOT,capture_output=True,timeout=180);elapsed=time.perf_counter_ns()-start
  text=('stdout_hex='+r.stdout.hex() if binary else r.stdout.decode('utf8',errors='replace'))+'\n'+r.stderr.decode('utf8',errors='replace');log=logs/(label+'.log');log.write_text(text,encoding='utf8');records.append({'command':command,'exit_code':r.returncode,'elapsed_ns':elapsed,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==0,(label,text);return r.stdout if binary else r.stdout.decode('utf8')
 values=[i%7-3 for i in range(16)]
 def expected_dictionary(selector,kind):
  if kind==1:return sum(values)*50000
  if kind<3:return (max([0,*values]) if selector else sum(values))*50000
  result=0
  for i,v in enumerate(values):result=max(result,v) if (selector+i)&1 else result+v
  return result*50000
 sources=['labs/dispatch/tests/cost_measure.c','labs/dispatch/tests/cost_methods.c','assessor/dispatch/dispatch.c','assessor/dispatch/dispatch_resolution.c','assessor/dod/dod.c']
 for compiler in ['gcc','clang']:
  execute([compiler,'--version'],compiler+'-version')
  for optimization,lto in [('O0',False),('O2',False),('O3',False),('O3',True)]:
   profile=compiler+'-'+optimization+('-lto' if lto else '-separate');directory=build/profile;directory.mkdir();flags=['-std=c17','-'+optimization,'-Wall','-Wextra','-Wpedantic','-Werror','-flto' if lto else '-fno-lto','-ffunction-sections','-fdata-sections','-Ilabs/dispatch/include','-Ilabs/dod/include'];objects=[]
   for i,source in enumerate(sources):
    obj=directory/(str(i)+'.o');execute([compiler,*flags,'-c',source,'-o',obj],profile+'-compile-'+str(i));objects.append(obj)
   executable=directory/'measure';execute([compiler,*flags,*objects,'-Wl,--gc-sections','-ldl','-o',executable],profile+'-link')
   execute(['size',executable],profile+'-size');execute(['nm','-S','--size-sort',executable],profile+'-symbols');execute(['objdump','-d',executable],profile+'-assembly')
   grouped={}
   for selector in (0,1):
    rows=execute([executable,selector],profile+'-run-'+str(selector)).splitlines();assert len(rows)==(99 if selector==0 else 36)
    layout=list(map(int,rows[0].split()[1:]));assert layout[0]==4 and layout[2] in (4,8);layouts.append({'profile':profile,'selector':selector,'int_long_pointer_view_align_box_align_offset':layout})
    for line in rows[1:]:
     row=line.split()
     if row[0]=='R':key=('representation',int(row[2]));duration=int(row[3]);assert int(row[4])==50050000
     elif row[0]=='D':
      selected,trial,kind=map(int,row[1:4]);key=('dictionary',selected,kind);duration=int(row[4]);assert int(row[5])==expected_dictionary(selected,kind)
     else:
      population,trial,kind=map(int,row[1:4]);key=('cache',population,kind);duration=int(row[4]);assert int(row[5])==240000;hits,misses=map(int,row[6:8]);want_miss=20000 if population==4 else population;assert (hits,misses)==((20000-want_miss,want_miss) if kind else (0,0))
     grouped.setdefault(key,[]).append(duration)
   for key,samples in grouped.items():assert len(samples)==7;measurements.append({'profile':profile,'workload':key,'samples_ns':samples,'median_ns':statistics.median(samples),'min_ns':min(samples),'max_ns':max(samples)})
   # Recompile exactly one source and relink: actual incremental observation,
   # not an invented estimate based on a clean command duration.
   execute([compiler,*flags,'-c',sources[1],'-o',objects[1]],profile+'-incremental-methods');execute([compiler,*flags,*objects,'-Wl,--gc-sections','-ldl','-o',executable],profile+'-incremental-link')
 # This exact extraction is a build adapter, not a rewritten mechanism. Its
 # source bytes include the actual checked direct/erased/embedded/fold functions.
 text=(ROOT/'assessor/dispatch/dispatch.c').read_text();body=text[text.index('static int sum('):text.index('int ds_closure_init(')];core=build/'abi-core.c';core.write_text('#include "dispatch.h"\n#include <limits.h>\n'+body)
 system_header=Path('/usr/include/x86_64-linux-gnu/asm/unistd_32.h');header=system_header.read_text();assert '#define __NR_exit 1\n' in header and '#define __NR_write 4\n' in header
 limits_dir=build/'abi-limits';limits_dir.mkdir();limits_header=limits_dir/'limits.h';limits_header.write_text('#ifndef CMASTERY_PROBE_LIMITS_H\n#define CMASTERY_PROBE_LIMITS_H\n#ifndef __INT_MAX__\n#error target compiler INT_MAX required\n#endif\n#define INT_MAX __INT_MAX__\n#define INT_MIN (-INT_MAX - 1)\n#endif\n')
 for compiler in ['gcc','clang']:
  execute([compiler,'-m32','-ffreestanding','-dM','-E','-x','c','/dev/null'],compiler+'-abi32-predefines')
  executable=build/(compiler+'-abi32');command=[compiler,'-m32','-ffreestanding','-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-fno-stack-protector','-nostdlib','-static','-I'+str(limits_dir),'-Ilabs/dispatch/include','-Ilabs/dod/include',core,'labs/dispatch/tests/abi32.c','labs/dispatch/tests/abi32.S','-Wl,-e,_start,-z,noexecstack','-o',executable];execute(command,compiler+'-abi32-build')
  elf=execute(['readelf','-h','-l',executable],compiler+'-abi32-elf');assert 'ELF32' in elf and 'Intel 80386' in elf and 'INTERP' not in elf
  unresolved=execute(['nm','-u',executable],compiler+'-abi32-undefined');assert not unresolved.strip()
  raw=execute([executable],compiler+'-abi32-execute',True);words=list(struct.unpack('<10I',raw));assert words==[4,4,4,8,4,8,4,4,6,33]
  execute(['objdump','-d',executable],compiler+'-abi32-assembly');abi.append({'compiler':compiler,'executed':True,'words':words,'binary_sha256':sha(executable),'no_unresolved_symbols':True,'no_interpreter':True,'environment':'Linux i386 system-call ABI process executed by current WSL kernel compatibility support; not a different physical CPU.'})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 result={'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'measurements':measurements,'layouts':layouts,'abi32':abi,'abi_adapter':{'source':core.relative_to(ROOT).as_posix(),'sha256':sha(core),'extracted_begin':'static int sum(','extracted_end_exclusive':'int ds_closure_init(','limits_bridge':{'path':limits_header.relative_to(ROOT).as_posix(),'sha256':sha(limits_header),'scope':'Only INT_MAX/INT_MIN needed by extracted core, derived from target compiler predefined __INT_MAX__; GCC i386 hosted libc headers are unavailable.'},'kernel_header':str(system_header),'kernel_header_sha256':sha(system_header),'inspected_definitions':['__NR_exit=1','__NR_write=4']},'executions':records,'limits':['1064 host timing samples in152 cells; exact finite outputs checked independently. No affinity/frequency control or confidence interval; other host tasks may interfere.','Uniform dictionary variants share semantics; alternating per-element policy variants form a separate comparison. Fixed-add measurements at selector1 must not be compared as a maximum operation.','Each representation benchmark uses1000 objects and100 rounds; direct typed storage, embedded tables and external views have explicitly different storage overhead. No universal layout ABI is inferred.','Warmth is uncontrolled. Cache observations count actual software hits/misses, not hardware cache events.','The initial GCC i386 build failed because installed hosted libc headers lacked bits/libc-header-start.h. This standalone probe supplies only the required INT_MAX/INT_MIN from compiler predefines, preserving the C arithmetic model; no libc-dependent program is claimed validated.','Linux i386 execution uses exact extracted original core functions and actual public types. It excludes dynamic loading, allocator and board/runtime components; no freestanding board execution claim.','Independent expected values are author-built models, not independent human assessment or learner mastery.']}
 report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print('dispatch costs passed:1064 samples; GCC/Clang separate/LTO; actual i386 outputs and link audits',flush=True)
if __name__=='__main__':main()
