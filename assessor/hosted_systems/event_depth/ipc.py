"""Original owned-process IPC/local connection experiments and independent oracles."""
import importlib.util,json,tempfile
from pathlib import Path

def run(c,root,args):
 c.group='events-ipc';pub=root/'labs/hosted_systems/event_depth';private=root/'assessor/hosted_systems/event_depth';flags=['-D_GNU_SOURCE','-std=c17','-Wall','-Wextra','-Werror','-fno-stack-protector','-pthread','-I'+str(pub)]
 spec=importlib.util.spec_from_file_location('ipc_history_oracle',private/'ipc_oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 inputs=c.root/'ipc-histories.txt';rows,states=oracle.write(inputs);c.require(states==26 and len(rows)==208,'IPC independent history graph',dict(states=states,edges=len(rows)))
 def build(cc,prefix,name,sources,extra):
  out=c.root/(prefix+'-'+name);c.run([cc,*flags,*extra,*sources,'-o',out]);return out
 for cc in ('gcc','clang'):
  for profile,extra in [('debug',['-O0']),('release',['-O2','-DNDEBUG']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   prefix=cc+'-'+profile
   exe=build(cc,prefix,'ipc',[pub/'ipc_tests.c',private/'ipc.c'],extra);c.run([exe],contains='queue_capacity=4;bytes=32;wire_sum=643;')
   exe=build(cc,prefix,'ipc-trace',[private/'ipc_trace.c',private/'ipc.c'],extra);out=c.run([exe,inputs]);checked=oracle.check(out,rows);c.require(checked['rows']==208,'IPC independent graph '+prefix,checked)
   exe=build(cc,prefix,'ipc-native',[private/'ipc_native.c',private/'ipc.c',private/'signals.c'],extra)
   for mode in range(3):c.run([exe,str(mode),'512'],contains=f'IPC-NATIVE:mode={mode};completed=512;',timeout=25)
   if profile=='release':
    for sample in range(8):
     for offset in range(3):
      mode=(sample+offset)%3
      c.run([exe,str(mode),'512'],contains=f'IPC-NATIVE:mode={mode};completed=512;',timeout=25,env={'CM_EVENT_MEASURE':str(sample),'CM_EVENT_PHASE':'warmup' if sample==0 else 'sample'})
   exe=build(cc,prefix,'ipc-protocol',[private/'ipc_protocol.c',private/'ipc.c',private/'signals.c'],extra)
   for mode in range(3):c.run([exe,str(mode)],contains=f'IPC-PROTOCOL:mode={mode};effect={0 if mode==0 else 1};',timeout=20)
   exe=build(cc,prefix,'ipc-restart',[private/'ipc_restart.c',private/'ipc.c'],extra);c.run([exe],contains='independent_execs=3;epochs=2;stale_request_rejected=1;',timeout=20)
   exe=build(cc,prefix,'ipc-budget',[pub/'ipc_budget_tests.c',private/'ipc_budget.c',private/'signals.c'],extra);c.run([exe],contains='IPC-BUDGET:independent_cells=81;')
   exe=build(cc,prefix,'ipc-budget-native',[private/'ipc_budget_native.c',private/'ipc_budget.c',private/'signals.c'],extra);c.run([exe],contains='IPC-BUDGET-NATIVE:sent=8;partial_reply=2;timeout=1;')
   exe=build(cc,prefix,'local',[pub/'local_tests.c',private/'local.c',private/'signals.c'],extra);c.run([exe],contains='LOCAL-PASS:')
   exe=build(cc,prefix,'local-probes',[private/'local_probes.c',private/'local.c',private/'signals.c'],extra)
   for mode,marker in [('candidates','LOCAL-CANDIDATES:attempts=2;'),('failures','LOCAL-FAILURES:socket_bind_listen_connect_accept=5;')]:c.run([exe,mode],contains=marker)
   # drvfs does not implement pathname Unix sockets; use a native Linux private
   # directory, not a different transport. Temporary directory owner cleans it.
   with tempfile.TemporaryDirectory(prefix='cmastery-events-') as d:c.run([exe,'families',Path(d)/'owned.sock'],contains='LOCAL-FAMILIES:actual_ipv6_loopback=1;actual_private_unix=1;')
   exe=build(cc,prefix,'local-abstract',[private/'local_abstract.c'],extra);c.run([exe],contains='LOCAL-ABSTRACT:embedded_nul_names_distinct=1;')
 for name,macro in [('early-release','EI_MUTANT_EARLY_RELEASE'),('ack-stops','EI_MUTANT_ACK_STOPS')]:
  exe=build('gcc','ipc',name,[pub/'ipc_tests.c',private/'ipc.c'],['-O2','-D'+macro]);c.run([exe],expected=1,contains='IPC-CHECK:')
 exe=build('gcc','ipc','reset-budget',[pub/'ipc_budget_tests.c',private/'ipc_budget.c',private/'signals.c'],['-O2','-DEI_MUTANT_RESET_BUDGET']);c.run([exe],expected=1,contains='IPC-BUDGET-CHECK:')
 for name,macro,mode in [('writable-success','EL_MUTANT_WRITABLE_SUCCESS','candidates'),('cancel-leak','EL_MUTANT_CANCEL_LEAK','failures')]:
  exe=build('gcc','local',name,[private/'local_probes.c',private/'local.c',private/'signals.c'],['-O2','-D'+macro]);c.run([exe,mode],expected=1,contains='LOCAL-PROBE-CHECK:')
 for name,test,stub,providers,marker in [('ipc',pub/'ipc_tests.c',pub/'ipc.c',[],'IPC-CHECK:'),('ipc-budget',pub/'ipc_budget_tests.c',pub/'ipc_budget.c',[],'IPC-BUDGET-CHECK:'),('local',pub/'local_tests.c',pub/'local.c',[],'LOCAL-CHECK:')]:
  exe=build('gcc',name,'learner',[test,stub,*providers],['-O2']);c.run([exe],expected=1,contains=marker)
 for objective,ordinary,changed in [
 ('03.a','Actual512 equal8-byte requests/4-byte replies on pipe pair, Unix socketpair and process-shared mutex record; golden643 and bounded ownership ledger','Three independent exec workers attach named data across2epochs, reject stale request, observe CLOEXEC and quiescent mutex reinitialization; no power-loss claim'),
 ('03.b','Independent26-history-state208-edge oracle plus controlled real preaccept-purge and postaccept-disconnect effect witnesses distinguish certain/uncertain result','Actual accepted cancellation acknowledgment retains server access until separate stop confirmation; effect can occur between acknowledgment and quiescence'),
 ('03.c','All3 actual transports queue4 while server gated and reject fifth without owner transfer;512 equal requests with raw wall/struct lr_cpu/bytes/mapping measurements','81 independently calculated clock cells reject budget restart and retain partial progress; real stalled response times out after8sent/2received without output publication or remote rollback'),
 ('04.a','Numeric resolver list freed; first reserved unlistening loopback candidate rejected and next owned listener reached, failed descriptors closed before fresh attempts','Actual nonblocking POLLOUT plus SO_ERROR distinguishes failure/success; wrong writable-success mutant fails peer/completion assertion'),
 ('04.b','Five real-operation injection boundaries socket/bind/listen/connect/accept balance per-acquisition descriptor ledger and preserve primary error','Cancellation after successful accept before publication consumes exactly one new descriptor, no output transfer; leak mutant fails ledger'),
 ('04.c','Actual IPv4, IPv6 loopback and private pathname Unix sockets; correct lengths/network-order0x1234bytes and unsupported/oversize rejection','Exact Linux abstract-address analysis and actual embedded-NUL-distinct endpoints, duplicate-name rejection and last-close rebind; filesystem permissions never inferred')]:
  c.observe('T24.'+objective,ordinary,['events-ipc']);c.observe('T24.'+objective,changed,['events-ipc'],True)
