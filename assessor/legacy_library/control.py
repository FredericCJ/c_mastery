import hashlib,re
from pathlib import Path

def run(c,root,args):
 c.group='library-control';pub=root/'labs/foundations/library_depth';private=root/'assessor/legacy_library';source=private/'control.c';common=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(pub)]
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name+'-control');c.run([cc,*common,*flags,pub/'control_tests.c',source,'-o',exe]);c.run([exe],contains='CONTROL-PASS:')
   for backend,define in [('c-threads',['-DLG_USE_C_THREADS']),('pthreads',[])]:
    exe=c.root/(cc+'-'+name+'-control-'+backend);c.run([cc,*common,*flags,*define,private/'control_threads.c',source,'-pthread','-o',exe]);c.run([exe],contains='CONTROL-PASS:32 actual thread publications')
  exe=c.root/(cc+'-control-signal');c.run([cc,*common,'-O2',private/'control_signal.c','-o',exe]);c.run([exe],contains='CONTROL-PASS:owned child signal')
 for name,flag,selected in [('control-signal','LG_MUTANT_SIGNAL',source),('control-cleanup','LG_MUTANT_CLEANUP',source),('control-payload','LG_MUTANT_PAYLOAD',source),('control-learner','UNSOLVED',pub/'control.c')]:
  exe=c.root/name;c.run(['gcc',*common,'-O2','-D'+flag,pub/'control_tests.c',selected,'-o',exe]);c.run([exe],expected=1,contains='CONTROL-CHECK:')
 tool=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc');model=private/'control_model.c';toolroot=tool.parents[2]
 for label,flag in [('release-acquire',[]),('missing-release',['-DLG_WEAK_RELEASE']),('missing-acquire',['-DLG_WEAK_ACQUIRE'])]:
  command=[tool,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--','-std=c17',*flag,model];out=c.run(command,expected=42 if flag else 0,contains='Error: Safety violation!' if flag else 'Number of complete executions explored:');count=re.search(r'Number of complete executions explored: (\d+)',out);c.require(count and int(count[1])>0 and ('Rrlx (payload, 0) [INIT]' in out if flag else 'Error:' not in out),'actual RC11 publication model '+label,dict(executions=int(count[1]) if count else 0,role='bounded counterexample' if flag else 'finite no-error exploration',payload='atomic in model; actual implementation uses non-atomic payload with derived happens-before',raw=out))
 c.require(True,'actual GenMC identity',{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [tool,toolroot/'doc/manual/usage.md',toolroot/'doc/manual/features.md']})
 entry=c.write('control-entry.c','#include "control.h"\nstatic struct lg_publication p;volatile int observed;void _start(void){int value=0;lg_init(&p);lg_publish(&p,37);if(lg_receive(&p,&value)==LG_OK)observed=value;for(;;){}}\n')
 for cpu in ('cortex-m0','cortex-m4'):
  obj=c.root/(cpu+'-publication.o');common_target=['clang','--target=arm-none-eabi','-mcpu='+cpu,'-mthumb','-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(pub)]
  c.run([*common_target,'-c',private/'control_atomic.c','-o',obj]);imports=c.run(['arm-none-eabi-nm','-u',obj]);c.run(['arm-none-eabi-objdump','-dr',obj]);image=c.root/(cpu+'-publication.elf');entry_obj=c.root/(cpu+'-entry.o');c.run([*common_target,'-c',entry,'-o',entry_obj]);link=['arm-none-eabi-ld','-e','_start','-Map='+str(c.root/(cpu+'-publication.map')),entry_obj,obj,'-o',image]
  if cpu=='cortex-m0':
   c.require('__atomic_load_4' in imports and '__atomic_store_4' in imports,'actual M0 atomic helper dependency',imports);c.run(link,expected='nonzero',contains='__atomic_')
  else:
   c.run(link);final=c.run(['arm-none-eabi-nm','-u',image]);c.require(not final.strip(),'actual M4 publication final closure is empty',final)
 for key,ordinary,changed in [('15.a','Single-thread C signal/raise uses only volatile sig_atomic_t assignment and restores original disposition; I/O happens after return','Actual POSIX sigaction/mask/sigsuspend adapter receives one signal from an owned child without lost wakeup, restores mask/action and joins child; no async allocator or stdio handler'),('15.b','Four live-frame nonlocal paths including longjmp zero-to-one and injected unavailable allocation retain volatile local and release heap owner stored outside skipped frame; invalid nonvolatile read explained only','Cross-thread,returned destination and exited VLA scope are explicitly invalid premise cases with separate assessor proof; no undefined execution used as a golden fixture'),('15.c','Actual C threads and pthread backends each execute32 one-shot release/acquire publications in six profiles; lifetime extends through successful matching join and actual lock-free property is observed','RTOS port receives explicit start/join/lifetime/order/closure obligations and rejection cases; actual RC11 finite model plus two missing-edge safety counterexamples delimit guarantees instead of inferring them from x86 runs')]:c.observe('T12.'+key,ordinary,['library-control']);c.observe('T12.'+key,changed,['library-control'],True)
