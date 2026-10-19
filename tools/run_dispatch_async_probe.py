"""Measure bounded real plugin reload and retained state, preserving exact commands."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess,time,statistics
ROOT=Path(__file__).resolve().parents[1]
PATHS=['tools/run_dispatch_async_probe.py','labs/dispatch/tests/async.c','labs/dispatch/tests/plugin_async.c','labs/dispatch/tests/plugin.c','labs/dispatch/include/dispatch_async.h','labs/dispatch/include/capture.h','labs/dispatch/include/dispatch.h','labs/dod/include/dod.h','labs/ownership/include/ownership.h','assessor/dispatch/dispatch_async.c','assessor/dispatch/capture.c','assessor/dispatch/dispatch.c','assessor/dod/dod.c','assessor/ownership/ownership.c','fixtures/dispatch/async.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();report=(ROOT/a.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion')
 stamp='dispatch-async-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
 def execute(cmd,label):
  cmd=list(map(str,cmd));started=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);log=logs/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');records.append({'command':cmd,'exit_code':r.returncode,'seconds':time.monotonic()-started,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==0,(label,r.stderr);return r.stdout
 spec=json.loads((ROOT/'fixtures/dispatch/async.json').read_text())
 for compiler in ['gcc','clang']:
  tag=compiler+'-O2';flags=['-std=c17','-O2','-fstack-usage','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/dispatch/include','-Ilabs/dod/include','-Ilabs/ownership/include','-pthread'];libraries=[]
  execute([compiler,'--version'],tag+'-version')
  for schema in [1,2]:
   library=build/f'{tag}-{schema}.so';execute([compiler,*flags,'-fPIC','-shared',f'-DPLUGIN_SCHEMA={schema}','labs/dispatch/tests/plugin_async.c','-o',library],f'{tag}-plugin-{schema}');libraries.append(library)
  bad=build/(tag+'-bad.so');execute([compiler,*flags,'-fPIC','-shared','-DPLUGIN_ABI=9','labs/dispatch/tests/plugin.c','-o',bad],tag+'-bad');libraries.append(bad)
  binary=build/tag;objects=[]
  for i,source in enumerate(['labs/dispatch/tests/async.c','assessor/dispatch/dispatch_async.c','assessor/dispatch/dispatch.c','assessor/dispatch/capture.c','assessor/dod/dod.c','assessor/ownership/ownership.c']):
   obj=build/f'{tag}-{i}.o';execute([compiler,*flags,'-c',source,'-o',obj],f'{tag}-compile-{i}');objects.append(obj)
  execute([compiler,*objects,'-pthread','-ldl','-Wl,--export-dynamic','-o',binary],tag+'-link')
  layout=execute([binary,'layout',*libraries],tag+'-layout').strip();size=execute(['size',binary],tag+'-size');execute(['objdump','-dr',binary],tag+'-disassembly');execute(['readelf','-d',binary],tag+'-dependencies');frames=[]
  for su in build.glob(tag+'-*.su'):
   saved=logs/su.name;saved.write_bytes(su.read_bytes());frames.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
  execute([binary,'async_registry',*libraries],tag+'-registry');execute([binary,'reload_owned',*libraries],tag+'-retirement')
  rows=[]
  for line in execute([binary,'measure',*libraries],tag+'-measure').splitlines():
   marker,n,sample,ns,retained,live,peak,attempts=line.split();n,sample,ns,retained,live,peak,attempts=map(int,[n,sample,ns,retained,live,peak,attempts]);assert marker=='M' and ns>=0 and retained==8*n and live==peak==attempts==2*int(n>4);rows.append({'count':n,'sample':sample,'pause_ns':ns,'logical_retained_bytes':retained,'allocator_live_blocks':live,'allocator_peak_blocks':peak,'allocation_attempts':attempts})
  assert len(rows)==len(spec['measurement_counts'])*spec['measurement_trials']
  summary=[{'count':n,'recorded_samples':20,'discarded_first':1,'median_ns':statistics.median([r['pause_ns'] for r in rows if r['count']==n and r['sample']>0]),'min_ns':min(r['pause_ns'] for r in rows if r['count']==n and r['sample']>0),'max_ns':max(r['pause_ns'] for r in rows if r['count']==n and r['sample']>0)} for n in spec['measurement_counts']]
  observations.append({'compiler':compiler,'layout':layout,'image_size':size,'samples':rows,'summary':summary,'stack_artifacts':frames})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records,'limits':['Repeated local x86-64 WSL measurements; no cold loader/page-cache, physical target, worst-case latency or universal performance claim.','Timed prepare/commit includes dlopen, migration and two registrations; old callbacks are not queued in the timing cells. Separate deterministic real-thread tests hold a plugin callback across commit.','Logical state bytes exclude allocator row rounding, registry/control objects, module mappings, worker stacks, pthread/libdl internals and OS resources.','Failure gate is a pre-close retention model. No invalid handle or forced OS loader failure is used.','Serialized controller calls; no forceful cancellation of an executing callback.']},indent=2)+'\n',encoding='utf8');print('dispatch async:160 recorded reload samples plus8 warmups; real queued/running cancellation and retention checks passed')
if __name__=='__main__':main()
