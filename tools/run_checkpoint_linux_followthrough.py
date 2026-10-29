"""Execute exact P5 providers, real callback integration and declared fault seams."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/checkpoint_linux_followthrough';REF=ROOT/'assessor/checkpoint_linux_followthrough';BASE=ROOT/'labs/checkpoint_depth';OLD=ROOT/'assessor/checkpoint_depth'
REPORT='reviews/v5/expansion/legacy_checkpoint_linux/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
DEPTH=[OLD/('checkpoint_'+n+'.c') for n in ['depth','async','linux','operation']]
IMPORTS=[ROOT/p for p in ['assessor/integration/integration.c','assessor/foundations/foundation.c','assessor/algorithms/algo.c']]
INCLUDES=[LAB,BASE/'include',*[ROOT/('labs/'+n+'/include') for n in ['integration','foundations','algorithms']]]
RACE=ROOT/'labs/thread_depth/lifecycle/race_control.c'
s=importlib.util.spec_from_file_location('p5_common',ROOT/'assessor/legacy_systems/toolchain/common.py');common=importlib.util.module_from_spec(s);s.loader.exec_module(common);sha=common.sha
def cases():
 ordinary=[*DEPTH,*[BASE/'tests'/f'{n}.c' for n in ['async','linux','operation']],REF/'ordinary.c']
 rows=[('ordinary',ordinary,[],'P5_ORDINARY_PASS',0),('bridge',[REF/'bridge.c',LAB/'bridge_tests.c',*IMPORTS],[],'P5_BRIDGE_PASS',0),('ownership',[OLD/'checkpoint_async.c',REF/'ownership.c'],[],'P5_OWNER_PASS',0),('waiter',[OLD/'checkpoint_async.c',REF/'waiter.c'],['-Wl,--wrap=pthread_cond_wait'],'P5_WAIT_PASS',0),('fd7',DEPTH+[REF/'fd_reuse.c'],[],'P5_FD_PASS',0),('failures',[REF/'bridge.c',REF/'failures.c',*IMPORTS],['-Wl,--wrap=pthread_create','-Wl,--wrap=close'],'P5_FAILURE_PASS',0)]
 rows.append(('report-order',[OLD/'checkpoint_operation.c',REF/'report_trace.c'],[],'P5_TRACE_PASS',0))
 for n in range(23,33):rows.append((f'core-{n}',ordinary,['-DCP_MUTANT='+str(n)],'CP_CHECK:',1))
 for n in range(1,4):rows.append((f'bridge-{n}',[REF/'bridge.c',LAB/'bridge_tests.c',*IMPORTS],['-DP5_MUTANT='+str(n)],'P5_CHECK',1))
 rows.append(('bridge-stub',[LAB/'bridge_stub.c',LAB/'bridge_tests.c',*IMPORTS],[],'P5_CHECK',1))
 rows.append(('waiter-27',[OLD/'checkpoint_async.c',REF/'waiter.c'],['-DCP_MUTANT=27','-Wl,--wrap=pthread_cond_wait'],'P5_WAIT_CHECK',1))
 return rows
def inputs():
 files={Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',RACE,*DEPTH,*IMPORTS,*[p for _,ss,_,_,_ in cases() for p in ss]}
 for directory in [LAB,REF,*INCLUDES[1:]]:files.update(p for p in directory.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(files)
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/checkpoint-linux-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None
 try:
  c.group='identities'
  for t in ['gcc','clang']:c.run([t,'--version'])
  c.run(['uname','-a'])
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-pthread','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in INCLUDES]]+(['-O1','-g'] if 'asan' in tag else [])
   for name,sources,extra,marker,expected in cases():
    c.group=tag+':'+name;exe=w/(tag+'-'+name);c.run([cc,*flags,*extra,*sources,'-Wl,--gc-sections','-o',exe]);out=c.run([exe],expected=expected,contains=marker,env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'},timeout=30)
    c.require('runtime error:' not in out and 'ERROR: AddressSanitizer' not in out,'defined bounded result')
  for cc in ['gcc','clang']:
   flags=['-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-ffunction-sections','-fdata-sections',*['-I'+str(p) for p in INCLUDES]]
   for name,sources,extra,marker,expected in cases():
    if expected or name=='fd7':continue
    c.group=cc+'-tsan:'+name;exe=w/(cc+'-tsan-'+name);c.run([cc,*flags,*extra,*sources,'-Wl,--gc-sections','-o',exe]);out=c.run([exe],contains=marker,env={'TSAN_OPTIONS':'halt_on_error=1:exitcode=66'},timeout=30);c.require('WARNING: ThreadSanitizer' not in out,'bounded synchronized execution')
   c.group=cc+'-tsan:race-control';exe=w/(cc+'-race');c.run([cc,*flags,RACE,'-o',exe]);c.run([exe],expected=66,contains='WARNING: ThreadSanitizer: data race',env={'TSAN_OPTIONS':'halt_on_error=1:exitcode=66'},timeout=30)
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in ['gcc','clang']},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(profiles=6,positive_cases=7,core_mutants=10,bridge_mutants=3,compiling_stubs=1,forced_waiter_mutants=1,copied_workers=2,workload=64,tsan_profiles=2,tsan_positive_cases=6,tsan_race_controls=2),limits=d['limits']+['Linux hosted C17/POSIX pthreads and GNU linker wrapper controls; no strict target claim.','Actual unchanged struct f_async/in_shutdown/algo_ring imports, source-selected hosted linkage; unrelated functions are discarded.','Real local pipes/epoll/read/poll/thread synchronization, with explicitly injected creation/late-close outcomes; no disk failure or delivered signal is claimed.','Wait wrapper creates a real unlocked consumer-steal interval; no specific libc internal schedule is claimed.','Caller serializes bridge API and preserves context until join/destroy; no pthread_cancel or asynchronous signal-handler use.','Copy/shared-mutable comparison has equal values and different capacity/concurrency; no throughput equivalence.','Finite tests/sanitizers do not prove race freedom, termination, fairness or independent learner mastery.','Intentional original race control has no portable output oracle; only detector sensitivity is assessed in an isolated local process.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
