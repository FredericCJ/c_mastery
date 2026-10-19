"""Separate actual stack-frame and linker-map observations for dispatch costs."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['labs/dispatch/tests/cost_measure.c','labs/dispatch/tests/cost_methods.c','assessor/dispatch/dispatch.c','assessor/dispatch/dispatch_resolution.c','assessor/dod/dod.c']
PATHS=[*SOURCES,'labs/dispatch/include/dispatch.h','labs/dispatch/include/dispatch_resolution.h','labs/dod/include/dod.h','tools/run_dispatch_stack.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args();report=(ROOT/args.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion');stamp='dispatch-stack-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
 def execute(cmd,label):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);log=logs/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr);records.append({'command':cmd,'exit_code':r.returncode,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==0,r.stderr;return r.stdout
 for compiler in ['gcc','clang']:
  flags=['-std=c17','-O2','-fno-lto','-fstack-usage','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/dispatch/include','-Ilabs/dod/include'];objects=[];frames={}
  for i,source in enumerate(SOURCES):
   obj=build/(compiler+'-'+str(i)+'.o');execute([compiler,*flags,'-c',source,'-o',obj],compiler+'-compile-'+str(i));objects.append(obj);su=obj.with_suffix('.su');text=su.read_text();log=logs/(compiler+'-'+str(i)+'.su');log.write_text(text)
   for line in text.splitlines():
    fields=line.split();frames[fields[0].split(':')[-1]]={'bytes':int(fields[1]),'classification':' '.join(fields[2:]),'artifact':log.relative_to(ROOT).as_posix(),'sha256':sha(log)}
  binary=build/(compiler+'-measure');mapfile=logs/(compiler+'.map');execute([compiler,*flags,*objects,'-Wl,--gc-sections,-Map,'+str(mapfile),'-ldl','-o',binary],compiler+'-link');size=execute(['size',binary],compiler+'-size');observations.append({'compiler':compiler,'optimization':'O2 separate compilation','frames':frames,'map':mapfile.relative_to(ROOT).as_posix(),'map_sha256':sha(mapfile),'size':size,'maximum_internal_measure_chain':frames['ds_measure']['bytes']+frames['dc_measure']['bytes']+16,'runtime_dictionary_chain':sum(frames[f]['bytes'] for f in ['ds_fold','selected','dc_choose','dc_add'])+32,'limits':'Conservative sum of recorded nonrecursive frames plus8 bytes per frame for return-address allowance on this x86-64 ABI; excludes driver main, OS/libc, unknown client callbacks and tool instrumentation. Tail-call overlap is not subtracted.'})
 assert pins=={p:sha(ROOT/p) for p in PATHS};report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records},indent=2)+'\n');print('dispatch stack/map observations passed')
if __name__=='__main__':main()
