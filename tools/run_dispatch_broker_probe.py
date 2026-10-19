"""Execute original process-broker property violations and their repaired control."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
PATHS=['tools/run_dispatch_broker_probe.py','labs/dispatch/tests/broker.c','labs/dispatch/tests/broker_process.c','assessor/dispatch/dispatch_broker.c','labs/dispatch/include/dispatch_broker.h','labs/dispatch/include/dispatch.h','labs/dod/include/dod.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();report=(ROOT/a.report).resolve();assert report.is_relative_to(ROOT/'reviews/v5/expansion');stamp='dispatch-broker-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/stamp;logs=report.parent/'runs'/stamp;build.mkdir(parents=True);logs.mkdir(parents=True);pins={p:sha(ROOT/p) for p in PATHS};records=[];observations=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));started=time.monotonic();r=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=30);log=logs/(label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf8');records.append({'command':cmd,'exit_code':r.returncode,'expected_exit_code':expected,'seconds':time.monotonic()-started,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)});assert r.returncode==expected,(r.returncode,r.stderr);return r
 for compiler in ['gcc','clang']:
  for mode in ['O2','asan-ubsan']:
   for mutant in range(3):
    tag=f'{compiler}-{mode}-{mutant}';flags=['-std=c17','-O2','-fstack-usage','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/dispatch/include','-Ilabs/dod/include',f'-DDS_BROKER_MUTANT={mutant}']
    if mode=='asan-ubsan':flags+=['-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']
    binary=build/tag;objects=[]
    for index,source in enumerate(['labs/dispatch/tests/broker.c','labs/dispatch/tests/broker_process.c','assessor/dispatch/dispatch_broker.c']):
     obj=build/(tag+'-'+str(index)+'.o');execute([compiler,*[f for f in flags if f!='-no-pie'],'-c',source,'-o',obj],tag+'-compile-'+str(index));objects.append(obj)
    execute([compiler,*flags,*objects,'-o',binary],tag+'-link')
    layout=execute([binary,'layout'],tag+'-layout').stdout.strip();assert layout=='104 20 16',layout
    size=execute(['size',binary],tag+'-size').stdout
    execute(['objdump','-dr',binary],tag+'-disassembly')
    frames=[]
    for su in [*build.glob(tag+'-*.su'),build/(tag+'.su')]:
     if su.exists():
      saved=logs/su.name;saved.write_bytes(su.read_bytes());frames.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
    r=execute([binary,'broker'],tag+'-run',int(mutant!=0));expected=['frames=11 truncated=5 counters=5,100 next=10 child_ok=1 parent_private=17','frames=4 truncated=0 counters=12,100 next=4 child_ok=0 parent_private=17','frames=5 truncated=0 counters=5,107 next=5 child_ok=0 parent_private=17'][mutant];assert expected in r.stdout,(expected,r.stdout)
    assert not mutant or 'DS_CHECK:broker:' in r.stderr
    assert 'AddressSanitizer' not in r.stderr and 'runtime error:' not in r.stderr
    observations.append({'compiler':compiler,'mode':mode,'mutant':mutant,'layout':layout,'size':size,'stack_artifacts':frames,'observation':r.stdout.strip(),'property':'repaired authority boundary' if not mutant else 'stored-rights violation' if mutant==1 else 'confused-deputy cross-target violation'})
 assert pins=={p:sha(ROOT/p) for p in PATHS}
 report.write_text(json.dumps({'schema_version':1,'status':'passed','passed':True,'inspected_paths':pins,'scope_source_stable':True,'observations':observations,'executions':records,'limits':['Original unnamed local socketpair and its own forked child only; synthetic bounded counters.','Unauthorized counter change is demonstrated, not arbitrary execution or OS privilege escalation.','Endpoint grants are provisioned by the harness; arbitrary peer authentication, same-user debugger access and a compromised kernel are outside the model.','No host security setting or physical equipment changed.']},indent=2)+'\n',encoding='utf8');print('dispatch broker:12 repaired/flawed actual process experiments passed their expected property checks')
if __name__=='__main__':main()
