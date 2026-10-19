import importlib.util,json,sys
def run(c,root):
 c.group='can';pub=root/'labs/hosted_systems/can_depth';private=root/'assessor/hosted_systems/can_depth';source=private/'frames.c';flags=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(pub)]
 environment=json.loads(c.run([sys.executable,private/'environment.py']));c.require(environment['selected_backend']=='original-frame-adapter' and environment['kernel_can_execution'] is False,'explicit software backend',environment)
 spec=importlib.util.spec_from_file_location('can_tuple_oracle',private/'filter_oracle.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
 for cc in ('gcc','clang'):
  for profile,extra in [('debug',['-O0']),('release',['-O2','-DNDEBUG']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   prefix=cc+'-'+profile
   for name,test in [('core',pub/'tests.c'),('tuples',private/'filter_trace.c')]:
    exe=c.root/(prefix+'-'+name);c.run([cc,*flags,*extra,test,source,'-o',exe]);text=c.run([exe],contains='CAN-PASS:' if name=='core' else None)
    if name=='tuples':result=oracle.check(text);c.require(result['cases']==32768,'independent tuple oracle '+prefix,result)
  obj=c.root/(cc+'-frames.o');c.run([cc,*flags,'-O2','-c',source,'-o',obj]);imports=c.run(['nm','-u',obj]);c.require(not any(s in imports for s in ('malloc','calloc','realloc','free')),'no heap service '+cc,imports);c.run(['size','-A',obj])
 for name,macro in [('filter','CA_MUTANT_FILTER'),('length','CA_MUTANT_LENGTH'),('error','CA_MUTANT_ERROR'),('own','CA_MUTANT_OWN')]:
  exe=c.root/('can-'+name);c.run(['gcc',*flags,'-O2','-D'+macro,pub/'tests.c',source,'-o',exe]);c.run([exe],expected=1,contains='CAN-CHECK:')
 exe=c.root/'can-learner';c.run(['gcc',*flags,'-O2',pub/'tests.c',pub/'frames.c','-o',exe]);c.run([exe],expected=1,contains='CAN-CHECK:')
 c.run([sys.executable,pub/'check_bench.py','--self-test'],contains='"synthetic_math_cases": 6')
 c.run([sys.executable,pub/'check_bench.py','--plan',pub/'bench_plan.json'],expected=1,contains='positive integer decoded timing parameters required')
 c.observe('T24.10.c','Actual Linux-header classical frame adapter, lengths/rawDLC boundaries, unaligned spans, error classes and exact format/request filters;32768 independent tuple cells and explicit16-cell loopback model; no kernel CAN/bus execution',['tests.c','independent-tuples:32768','environment.py'])
 c.observe('T24.10.c','Complete two-node bench plan/capture/inspection/acceptance packet, decoded-timing arithmetic checker and rejection of unfilled hardware plan; no selected-board register values or physical observations fabricated',['BENCH.md','bench_plan.json','check_bench.py:self-test-and-unfilled-rejection'],True,limitations=['Physical execution and device-specific timing legality remain pending exact two-node hardware/manual/instrument selection.'])
