"""Observe two opaque implementations under the same callback contract."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['labs/dispatch/tests/modules_measure.c','assessor/dispatch/dispatch_modules.c','assessor/ownership/ownership.c','assessor/dod/dod.c']
PATHS=[*SOURCES,'assessor/dispatch/dispatch.c','assessor/dispatch/module_alternative.c','labs/dispatch/include/dispatch_modules.h','labs/dispatch/include/dispatch.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/dod/include/dod.h','tools/run_dispatch_modules_probe.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();report=(ROOT/a.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion');stamp='dispatch-modules-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
 def execute(cmd,label):
  cmd=list(map(str,cmd));start=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);log=logs/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');records.append({'command':cmd,'exit_code':r.returncode,'seconds':time.monotonic()-start,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==0,r.stderr;return r.stdout
 for compiler in ['gcc','clang']:
  for implementation in ['original','alternative']:
   tag=compiler+'-'+implementation;flags=['-std=c17','-O2','-fno-lto','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/dispatch/include','-Ilabs/dod/include','-Ilabs/ownership/include'];objects=[];frames=[]
   sources=[*SOURCES,'assessor/dispatch/'+('dispatch.c' if implementation=='original' else 'module_alternative.c')]
   for i,source in enumerate(sources):
    obj=build/(tag+'-'+str(i)+'.o');execute([compiler,*flags,'-c',source,'-o',obj],tag+'-compile-'+str(i));objects.append(obj);su=obj.with_suffix('.su');saved=logs/su.name;saved.write_bytes(su.read_bytes());frames.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
   binary=build/tag;mapfile=logs/(tag+'.map');execute([compiler,*flags,*objects,'-Wl,--gc-sections,-Map,'+str(mapfile),'-ldl','-o',binary],tag+'-link')
   lines=execute([binary],tag+'-run').splitlines();layout=list(map(int,lines[0].split()[1:]));assert layout==[80,160,16,208,32,32],layout
   samples=[]
   for line in lines[1:]:
    _,trial,ns,total,calls=line.split();assert int(total)==2800000 and int(calls)==400000;samples.append({'trial':int(trial),'nanoseconds':int(ns),'checksum':int(total),'callback_calls':int(calls)})
   assert len(samples)==7
   size=execute(['size',binary],tag+'-size');symbols=execute(['nm','-S',binary],tag+'-symbols');execute(['objdump','-dr',binary],tag+'-disassembly')
   # An actual isolated implementation recompilation and relink, preserving the public client object.
   execute([compiler,*flags,'-c',sources[-1],'-o',objects[-1]],tag+'-incremental-compile');execute([compiler,*flags,*objects,'-Wl,--gc-sections','-ldl','-o',binary],tag+'-incremental-link');execute([binary],tag+'-incremental-run')
   observations.append({'compiler':compiler,'implementation':implementation,'layout':layout,'samples':samples,'size':size,'private_pool_symbols':[line for line in symbols.splitlines() if line.endswith(' modules') or line.endswith(' pool')],'stack_artifacts':frames,'map':mapfile.relative_to(ROOT).as_posix(),'map_sha256':sha(mapfile)})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records,'limits':['Same successful two-read callback contract and200000 calls per trial; one warmup trial excluded.','Seven within-process observations per binary; scheduling/frequency/cache state uncontrolled, no universal performance ranking.','Reference counts and service contexts are not concurrently accessed; no physical-board evidence.']},indent=2)+'\n',encoding='utf8');print('dispatch modules: two implementations, two compilers,28 recorded timing samples and actual incremental rebuilds passed')
if __name__=='__main__':main()
