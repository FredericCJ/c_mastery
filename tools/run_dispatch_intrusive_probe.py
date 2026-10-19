"""Run bounded layout/recovery probes; record real code, frame and diagnostic observations."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
PATHS=['tools/run_dispatch_intrusive_probe.py','labs/dispatch/tests/intrusive_probe.c','assessor/dispatch/dispatch_intrusive.c','labs/dispatch/include/dispatch_intrusive.h','labs/dispatch/include/dispatch.h','labs/dod/include/dod.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();report=(ROOT/a.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion');stamp='dispatch-intrusive-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
 def execute(cmd,label):
  cmd=list(map(str,cmd));start=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);log=logs/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');records.append({'command':cmd,'exit_code':r.returncode,'seconds':time.monotonic()-start,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==0,r.stderr;return r.stdout
 for compiler in ['gcc','clang']:
  for level in ['0','2','3','2-sanitize']:
   tag=compiler+'-'+level;flags=['-std=c17','-O'+level[0],'-Wall','-Wextra','-Wpedantic','-Werror','-fstack-usage','-Ilabs/dispatch/include','-Ilabs/dod/include']
   if 'sanitize' in level:flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie']
   binary=build/tag
   objects=[]
   for index,source in enumerate(['labs/dispatch/tests/intrusive_probe.c','assessor/dispatch/dispatch_intrusive.c']):
    obj=build/(tag+'-'+str(index)+'.o');execute([compiler,*flags,'-c',source,'-o',obj],tag+'-compile-'+str(index));objects.append(obj)
   execute([compiler,*flags,*objects,*(['-no-pie'] if 'sanitize' in level else []),'-o',binary],tag+'-link')
   layout=list(map(int,execute([binary],tag+'-run').split()));assert layout==[88,40,48,64,28,16,48,8],layout
   execute(['objdump','-dr',binary],tag+'-disassembly');size=execute(['size',binary],tag+'-size')
   frames=[]
   for source in build.glob(tag+'-*.su'):
    target=logs/source.name;target.write_bytes(source.read_bytes());frames.append({'path':target.relative_to(ROOT).as_posix(),'sha256':sha(target)})
   observations.append({'compiler':compiler,'configuration':level,'layout':dict(zip(['dual_item','hook','dual_list','index_owner','index_slot','index_handle','second_hook_offset','dual_alignment'],layout)),'size':size,'stack_artifacts':frames})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records,'limits':['Original matching-member probes only; no invalid pointer conversion is executed.','GNU/Clang arbitrary-offset recovery observation on this x86-64 ABI is not an ISO C provenance theorem.','No target-board or independent learner result.']},indent=2)+'\n',encoding='utf8');print('dispatch intrusive layout/recovery: eight actual builds and executions passed')
if __name__=='__main__':main()
