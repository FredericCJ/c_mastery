import hashlib,importlib.util,itertools,json,os,sys,tempfile
from pathlib import Path

def run(c,root):
 c.group='completion';pub=root/'labs/hosted_systems/completion_depth';private=root/'assessor/hosted_systems/completion_depth';source=private/'completion.c';provider=root/'assessor/linux_io_depth/targets/raw_ring.c';raw=root/'labs/linux_io_depth/targets';flags=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(pub),'-I'+str(raw)]
 spec=importlib.util.spec_from_file_location('completion_oracle',private/'oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 environment=json.loads(c.run([sys.executable,private/'environment.py']));c.require(environment['header_query_exit']==0 and environment['physical_measurement'] is False,'actual guest ABI identity',environment)
 for cc in ('gcc','clang'):
  for profile,extra in [('debug',['-O0']),('release',['-O2','-DNDEBUG']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   prefix=cc+'-'+profile
   for name,test,providers in [('core',pub/'tests.c',[source]),('native',private/'native.c',[source,provider]),('orders',private/'order_trace.c',[source]),('stall',private/'stall.c',[])]:
    exe=c.root/(prefix+'-'+name);c.run([cc,*flags,*extra,test,*providers,'-o',exe])
    if name=='native':
     for mode,marker in [('cancel','COMPLETION-CANCEL:fixed=1;'),('boundary','COMPLETION-BOUNDARY:fixed=1;'),('failure','COMPLETION-FAILURE:acquisition_cuts=5;')]:c.run([exe,mode],contains=marker)
    elif name=='orders':c.require(True,'independent event sets '+prefix,oracle.check_order(c.run([exe])))
    else:c.run([exe],contains='COMPLETION-PASS:' if name=='core' else 'STALL-PASS:backend=sendfile;')
  for name,src in [('core',source),('raw',provider)]:
   obj=c.root/(cc+'-'+name+'.o');c.run([cc,*flags,'-O2','-fstack-usage','-c',src,'-o',obj]);imports=c.run(['nm','-u',obj]);c.require(not any(s in imports for s in ('malloc','calloc','realloc','free')),'no heap '+cc+'-'+name,imports);c.run(['size','-A',obj])
 for name,macro in [('token','CO_MUTANT_TOKEN'),('cancel','CO_MUTANT_CANCEL_RELEASE'),('pending','CO_MUTANT_PENDING_ACK')]:
  exe=c.root/('completion-'+name);c.run(['gcc',*flags,'-O2','-D'+macro,pub/'tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='COMPLETION-CHECK:')
 exe=c.root/'stall-early';c.run(['gcc',*flags,'-O2','-DCO_STALL_EARLY',private/'stall.c','-o',exe]);c.run([exe],expected=1,contains='STALL-CHECK:')
 for name,test,providers,mode,marker in [('core',pub/'tests.c',[pub/'completion.c'],None,'COMPLETION-CHECK:'),('native',pub/'native.c',[source,provider],'cancel','COMPLETION-NATIVE-CHECK:'),('provider',private/'native.c',[source,raw/'raw_ring.c'],'cancel','COMPLETION-UNAVAILABLE:'),('stall',pub/'stall.c',[],None,'STALL-CHECK:'),('workload',pub/'workload.c',[source,provider],None,'WORKLOAD-CHECK:')]:
  exe=c.root/('completion-learner-'+name);c.run(['gcc',*flags,'-O2',test,*providers,'-o',exe]);c.run([exe,*([mode] if mode else [])],expected=77 if name=='provider' else 1,contains=marker)
 prediction=root/'research/v5/expansion/legacy_completion_io/prediction.json';seal=c.root/'prediction_seal.json'
 c.run([sys.executable,pub/'seal.py','--prediction',prediction,'--implementation',private/'workload.c','--output',seal]);c.require(seal.is_file(),'prediction precedes final matrix',json.loads(seal.read_text()))
 # Own an ext4 file in this user's guest home. No global cache control or
 # external data. The directory is empty after unlinked fixtures close.
 bench=Path(tempfile.mkdtemp(prefix='cmastery-completion-',dir=Path.home()));outputs=[]
 try:
  for cc in ('gcc','clang'):
   # Clang's multi-source link invocation writes one .su name repeatedly.
   # Retain an explicit workload translation-unit audit, not its last writer.
   obj=c.root/(cc+'-workload-stack.o');c.run([cc,*flags,'-O2','-DNDEBUG','-fstack-usage','-c',private/'workload.c','-o',obj]);c.run(['nm','-u',obj]);c.run(['size','-A',obj])
   exe=c.root/(cc+'-workload');c.run([cc,*flags,'-O2','-DNDEBUG','-fstack-usage',private/'workload.c',source,provider,'-o',exe])
   for trial in range(3):
    backends=['readiness','ring','fixed','sendfile'];backends=backends[trial:]+backends[:trial]
    for depth,length,cache in itertools.product((1,4),(4096,64),('warm','cold')):
     for backend in backends:
      output=c.run([exe,backend,str(depth),str(length),cache,str(trial),bench]);result=oracle.check_workload(output);c.require(True,'independent workload '+cc+'/'+backend+'/'+str(depth)+'/'+str(length)+'/'+cache+'/'+str(trial),result);row=json.loads(output);row['compiler']=cc;outputs.append(row)
  exe=c.root/'workload-offset';c.run(['gcc',*flags,'-O2','-DCO_WORKLOAD_MUTANT_OFFSET',private/'workload.c',source,provider,'-o',exe]);output=c.run([exe,'readiness','4','64','warm','0',bench])
  rejected=False
  try:oracle.check_workload(output)
  except AssertionError:rejected=True
  c.require(rejected,'compiled offset mutant rejected by independent byte oracle',json.loads(output))
 finally:
  c.require(not list(bench.iterdir()),'no retained fixture files',str(bench));bench.rmdir()
 c.write('workloads.json',json.dumps(outputs,indent=2)+'\n')
 for oid,ordinary,changed,evidence in [
 ('a','Actual current operation probe and T62 raw-ring import; ordinary pipe cancellation, separate target/cancel CQEs, stale cancel, delayed dispatch, short read/EOF/badfd and five acquisition cuts;720 event permutations','Actual registered buffers/subrange17 and invalid range, target-plus-cancel drain then synchronous unregister before unmap; registered blocked-read cancellation',['native:cancel','native:boundary','native:failure','oracle:event-sets4320','import:T62']),
 ('b','Equivalent original file/socket bytes through copy/sendfile/ordinary-ring/fixed-ring; raw allocation, retained-byte, pin-interval, mapping/stack/import evidence and independent byte oracle','Actual stalled copy/sendfile sockets reach EAGAIN, retain immutable file until verified prefix/EOF; observed cancellation-to-drain samples and conditional bound analysis',['workload:192cells','stall:both-backends','native:cancel','ANALYSIS.md:resource-bound']),
 ('c','Two optimized compilers × four backends × two depths × two cache snapshots × two workload shapes × three trials; full64 latency samples/cell, whole/setup/workload/processCPU, cleanup and disclosed pilot-informed seal','Small scattered64-byte versus sequential4096-byte request generator, independent result oracle and pre-matrix prediction; no universal speed or physical-cache claim',['prediction_seal.json','workload:192cells','oracle:independent-bytes','native:failure'])]:
  c.observe('T24.12.'+oid,ordinary,evidence);c.observe('T24.12.'+oid,changed,evidence,True)
