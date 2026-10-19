import re
CASES={'coalesce':'COALESCE:generated=2;accepted=1;','realtime':'REALTIME:accepted=4;',
 'restart0':'RESTART:flag=0;return=-1;','restart1':'RESTART:flag=1;return=1;',
 'partial':'PARTIAL:requested=8;completed=2;','lost':'LOST-WAKE:repaired=0;',
 'repaired':'LOST-WAKE:repaired=1;','signalfd':'SIGNALFD:records=2;',
 'threads':'THREADS:workers=2;plugin=1;','bad-plugin':'BAD-PLUGIN:sole_unmasked_worker_received_process_signal=1;',
 'children':'CHILDREN:late=0;reaped=3;','late-child':'CHILDREN:late=1;reaped=3;',
 'deadline':'DEADLINE:real_ppoll_interruptions=3;'}
def run(c,root,args):
 c.group='events-signals';pub=root/'labs/hosted_systems/event_depth';private=root/'assessor/hosted_systems/event_depth';source=private/'signals.c';flags=['-D_GNU_SOURCE','-std=c17','-Wall','-Wextra','-Werror','-fno-stack-protector','-pthread','-I'+str(pub)]
 for cc in ('gcc','clang'):
  for profile,extra in [('debug',['-O0']),('release',['-O2','-DNDEBUG']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   prefix=cc+'-'+profile;exe=c.root/(prefix+'-signals');c.run([cc,*flags,*extra,pub/'signals_tests.c',source,'-o',exe]);c.run([exe],contains='counted_histories=9841;')
   probe=c.root/(prefix+'-signal-probes');c.run([cc,*flags,*extra,private/'signal_probes.c',source,'-o',probe])
   for mode,marker in CASES.items():c.run([probe,mode],contains=marker,timeout=25)
   failure=c.root/(prefix+'-signal-failures');c.run([cc,*flags,*extra,private/'signal_failures.c',source,'-Wl,--wrap=pipe2','-Wl,--wrap=sigaction','-o',failure]);c.run([failure],contains='SIGNAL-FAILURES:acquisition_cuts=4;')
  obj=c.root/(cc+'-signal-port.o');c.run([cc,*flags,'-O2','-c',source,'-o',obj]);imports=c.run(['nm','-u',obj]);c.require(not re.search(r'__atomic|__sync',imports),'no generated atomic helper '+cc,imports)
  asm=c.run(['objdump','-dr',obj]);c.write(cc+'-signal-port.dis',asm);blocks=re.findall(r'<handler>:\n(.*?)(?=\n[0-9a-f]+ <|\Z)',asm,re.S);c.require(len(blocks)==1,'handler body exists '+cc)
  calls=re.findall(r'R_X86_64_(?:PLT32|PC32)\s+([A-Za-z_][A-Za-z_0-9]*)',blocks[0]);calls=[x for x in calls if not x.startswith('.')];c.require(set(calls)<= {'write','__errno_location'} and 'write' in calls,'handler call closure '+cc,calls)
  c.run(['readelf','-h',obj]);c.run(['size','-A',obj])
 for name,macro in [('full','ES_MUTANT_FULL'),('errno','ES_MUTANT_ERRNO'),('clear-all','ES_MUTANT_CLEAR_ALL'),('restart-budget','ES_MUTANT_RESTART_BUDGET')]:
  exe=c.root/('signals-'+name);c.run(['gcc',*flags,'-O2','-D'+macro,pub/'signals_tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='SIGNALS-CHECK:')
 exe=c.root/'signals-one-child';c.run(['gcc',*flags,'-O2','-DES_MUTANT_ONE_CHILD',private/'signal_probes.c',source,'-o',exe]);c.run([exe,'children'],expected=1,contains='SIGNAL-PROBE-CHECK:')
 exe=c.root/'signals-learner';c.run(['gcc',*flags,'-O2',pub/'signals_tests.c',pub/'signals.c','-o',exe]);c.run([exe],expected=1,contains='SIGNALS-CHECK:')
 exe=c.root/'signals-thread-tsan';c.run(['gcc',*flags,'-O1','-fsanitize=thread','-fno-pie','-no-pie',private/'signal_probes.c',source,'-o',exe]);c.run([exe,'threads'],contains=CASES['threads'])
 for objective,ordinary,changed in [
 ('01.a','Two blocked ordinary generations coalesce to one synchronous acceptance; actual bounded counted queue retains two admitted values and rejects third','Four same-number RT payloads preserve7,9,8,9; owned process soft RLIMIT_SIGPENDING zero gives EAGAIN and is restored; no host-capacity claim'),
 ('01.b','Actual enrolled pipe read produces EINTR without restart or X with SA_RESTART; independent2of8 short read commits its cursor','Independent160801 integer-time oracle and scripted1/.6/.2 budgets reject restarted deadline; actual ppoll interrupted3times then timesout'),
 ('01.c','Premise-based unsafe-handler analysis plus lock-free descriptor, savederrno and durable predicates under full nonblocking selfpipe;4acquisition cuts balance descriptors/masks/dispositions','Actual synchronous receiver allocates ordinary diagnostic, transfers under mutex and joins4participants before free/destroy; six profiles plus TSan'),
 ('02.a','Controlled checkfalse-handler-set-pause specimen observed blocked and harmlessly rescued; actual sigsuspend, ppoll/selfpipe and signalfd repairs','Two independent predicates survive selected consumption; two signalfd records consumed thenEAGAIN; clear-all semantic mutant rejected'),
 ('02.b','Two workers inherit verified mask before enrollment and dedicated receiver accepts process-directed signal','Third plugin verifies inherited mask; isolated only-unmasked-plugin control consumes process signal before receiver, demonstrating broken routing contract'),
 ('02.c','All3already-exited owned children reaped with20/21/22statuses in one sweep alongside shutdown; repeated sweep0','Last child exits after first reap and before atomicwait, pending SIGCHLD reaches second sweep; one-child mutant rejected')]:
  c.observe('T24.'+objective,ordinary,['events-signals']);c.observe('T24.'+objective,changed,['events-signals'],True)
