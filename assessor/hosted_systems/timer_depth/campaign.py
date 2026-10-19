import importlib.util
def run(c,root):
 c.group='timer';pub=root/'labs/hosted_systems/timer_depth';private=root/'assessor/hosted_systems/timer_depth';source=private/'timer.c';provider=root/'assessor/hosted_systems/deadline.c';flags=['-std=c17','-Wall','-Wextra','-Werror','-pthread','-I'+str(pub),'-I'+str(root/'labs/hosted_systems/include')]
 spec=importlib.util.spec_from_file_location('timer_calendar_oracle',private/'policy_oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 for cc in ('gcc','clang'):
  for profile,extra in [('debug',['-O0']),('release',['-O2','-DNDEBUG']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   prefix=cc+'-'+profile
   for name,test,providers in [('core',pub/'tests.c',[source]),('native',private/'native.c',[source,provider]),('races',pub/'race_tests.c',[source]),('calendar',private/'policy_trace.c',[source])]:
    exe=c.root/(prefix+'-'+name);c.run([cc,*flags,*extra,test,*providers,'-o',exe])
    if name=='native':
     for mode,marker in [('event','TIMER-EVENT:sum=5;'),('periodic','TIMER-PERIODIC:first='),('rearm','TIMER-REARM:earlier_selected=1;'),('cached','TIMER-CACHED:old=1;new=2;'),('failure','TIMER-FAILURE:acquisition_cuts=3;invalid_arm=1;success=1;closed=9;')]:
      text=c.run([exe,mode],contains=marker)
      if mode=='periodic':c.require(True,'native clock-bracket calendar '+prefix,oracle.check_native(text))
    elif name=='calendar':
     result=oracle.check(c.run([exe]));c.require(result['cases']==14112,'independent calendar '+prefix,result)
    else:c.run([exe],contains='TIMER-PASS:' if name=='core' else 'TIMER-RACE:forced_running=1;forced_cancelled=1;races=200;')
  obj=c.root/(cc+'-timer.o');c.run([cc,*flags,'-O2','-c',source,'-o',obj]);imports=c.run(['nm','-u',obj]);c.require(not any(s in imports for s in ('malloc','calloc','realloc','free')),'no heap service '+cc,imports);c.run(['size','-A',obj])
  exe=c.root/(cc+'-races-tsan');c.run([cc,*flags,'-O1','-g','-fsanitize=thread',pub/'race_tests.c',source,'-o',exe]);c.run([exe],contains='TIMER-RACE:forced_running=1;forced_cancelled=1;races=200;',env={'TSAN_OPTIONS':'halt_on_error=1:exitcode=66'})
  exe=c.root/(cc+'-known-race');c.run([cc,*flags,'-O1','-g','-fsanitize=thread',root/'assessor/hosted_systems/reactor_depth/known_race.c','-o',exe]);c.run([exe],expected=66,contains='WARNING: ThreadSanitizer: data race',env={'TSAN_OPTIONS':'halt_on_error=1:exitcode=66'})
 for name,macro in [('generation','TD_MUTANT_GENERATION'),('wrap','TD_MUTANT_WRAP'),('count','TD_MUTANT_COUNT'),('drift','TD_MUTANT_DRIFT'),('early','TD_MUTANT_EARLY_RELEASE')]:
  exe=c.root/('timer-'+name);c.run(['gcc',*flags,'-O2','-D'+macro,pub/'tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='TIMER-CHECK:')
 exe=c.root/'timer-race-early';c.run(['gcc',*flags,'-O2','-DTD_MUTANT_EARLY_RELEASE',pub/'race_tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='TIMER-RACE-CHECK:')
 for name,test,providers,mode,marker in [('core',pub/'tests.c',[pub/'timer.c'],None,'TIMER-CHECK:'),('races',pub/'race_tests.c',[pub/'timer.c'],None,'TIMER-RACE-CHECK:'),('native',pub/'native.c',[source,provider],'event','TIMER-NATIVE-CHECK:'),('provider',private/'native.c',[source,root/'labs/hosted_systems/src/deadline.c'],'rearm','TIMER-NATIVE-CHECK:')]:
  exe=c.root/('timer-learner-'+name);c.run(['gcc',*flags,'-O2',test,*providers,'-o',exe]);c.run([exe,*([mode] if mode else [])],expected=1,contains=marker)
 for oid,ordinary,changed,evidence in [
 ('a','Actual native timerfd/eventfd full-width counters, ordinary sum5, semaphore3ones, saturation/EAGAIN and short-width rejection; owned acquisition cuts and invalid arm cleanup','Three-object earliest absolute one-shot, earlier insertion and actual rearm, restore late instant, empty disarm; actual inherited hs_read_until provider and cancellation',['native:event','native:failure','native:rearm','import:assessor/hosted_systems/deadline.c','tests.c:earliest']),
 ('b','Actual periodic timer stalled through at least6 expirations, before-read/dispatch clocks and independent calendar bounds; no kernel timestamp or accuracy inference','Catch-up versus skip preserve phase;14112 independent calendar cells plus full-width exhaustion and compiled drift/count controls',['native:periodic','independent-calendar:14112','tests.c:edges']),
 ('c','Actual cached epoll generation1 rejected after cancel/rearm generation2; repeated cancel/finish and reduced3-generation retirement including failed-rearm preservation','Actual running callback borrow retained through cancellation2; forced cancel-first/run-first and200 barrier races/profile, mutex serialization and instrumented race controls',['native:cached','tests.c:history','race_tests.c:one','tsan:correct-and-known-race'])]:
  c.observe('T24.09.'+oid,ordinary,evidence);c.observe('T24.09.'+oid,changed,evidence,True)
