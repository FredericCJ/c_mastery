"""Source-bound first-principles command, quota and recovery campaign."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_commands';PUBLIC=ROOT/'labs/security_depth/command';PRIVATE=ROOT/'assessor/security_depth/command';BUILD=ROOT/'build'/('security-commands-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CORE=['base','authority','extensions','arithmetic'];SERVICE=['quota','schedules','recovery'];PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS={'trailing':('TRAILING','base','sc_decode(p, n, &a, 0, &b, &m) == expected'),'publication':('PUBLICATION','base','!memcmp(&b, &old, sizeof b)'),'authority':('AUTHORITY','authority','sc_decode(p, 2, &a, 0, &b, &m) == SC_DENIED'),'stale':('STALE','authority','sc_apply(&accepted, &a, &ordinary, &protected) == SC_STALE'),'expansion':('EXPANSION','extensions','sc_decode(expand, 4, &a, 1, &b, &m) == SC_BUDGET'),'extent':('EXTENT','arithmetic','sc_extent(3, 2, 4, 11, &out) == SC_OK'),'quota':('QUOTA','quota','sc_submit(&s, &b, &t[2]) == SC_BUDGET'),'terminal':('TERMINAL','schedules','sc_finish(&s, t, !order, &a, &p, &q) == SC_STALE'),'retry':('RETRY','schedules','sc_retry(&s, t) == SC_EXHAUSTED'),'recovery':('RECOVERY','recovery','sc_recover(&s, ops, 0) == SC_EXHAUSTED && s.state == 2')}
IMPORTS=['tools/run_thread_lifecycle.py','fixtures/legacy_depth/format_authority.c','assessor/legacy_depth/security_format.c','fixtures/embedded/PR-T35-01.json','fixtures/embedded/PR-T35-03.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def expected_commands():
 out={'environment.kernel','environment.libc','models'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   t=cc+'.'+p
   out|={t+'.'+n+'.compile' for n in ['core','service','oracle','process','format.flawed','format.repaired','learner.core','learner.service']}
   out|={t+'.'+n for n in CORE+SERVICE+['differential','process','format.flawed','format.repaired']}
   out|={t+'.learner.'+n for n in CORE+SERVICE}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};observations=[]
 def compile(cc,flags,ident,sources,out):
  run(ident,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*flags,'-I',PUBLIC,*sources,'-o',out]);harness.OUTPUTS.add(out)
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    t=cc+'.'+p;core=[PRIVATE/'command.c',PUBLIC/'flawed_authority.c',PUBLIC/'tests.c'];service=[PRIVATE/'command.c',PRIVATE/'service.c',PUBLIC/'service_tests.c']
    for kind,cases,sources in [('core',CORE,core),('service',SERVICE,service)]:
     binary=BUILD/(t+'.'+kind);compile(cc,flags,t+'.'+kind+'.compile',sources,binary)
     for case in cases:run(t+'.'+case,[binary,case],marker='OBS '+case)
     stub=BUILD/(t+'.learner.'+kind);compile(cc,flags,t+'.learner.'+kind+'.compile',[PUBLIC/'learner.c',PUBLIC/'flawed_authority.c',PUBLIC/('tests.c' if kind=='core' else 'service_tests.c')],stub)
     for case in cases:run(t+'.learner.'+case,[stub,case],41,'SC_ASSERT:' if kind=='core' else 'SC_SERVICE_ASSERT:')
    oracle=BUILD/(t+'.oracle');compile(cc,flags,t+'.oracle.compile',[PRIVATE/'command.c',PUBLIC/'oracle_driver.c'],oracle);diff=json.loads(run(t+'.differential',[sys.executable,PRIVATE/'differential.py',oracle],marker='"status": "pass"'))
    process=BUILD/(t+'.process');compile(cc,flags,t+'.process.compile',[PUBLIC/'process_boundary.c'],process);run(t+'.process',[process],marker='child_private_write=isolated')
    for kind,path in [('flawed','fixtures/legacy_depth/format_authority.c'),('repaired','assessor/legacy_depth/security_format.c')]:
     binary=BUILD/(t+'.format.'+kind);compile(cc,flags+(['-DSC_REPAIRED_FORMAT'] if kind=='repaired' else []),t+'.format.'+kind+'.compile',[ROOT/path,PUBLIC/'format_test.c'],binary);run(t+'.format.'+kind,[binary],marker='synthetic_marker_only=1')
    for name,(define,case,marker) in MUTANTS.items():
     binary=BUILD/(t+'.mutation.'+name);compile(cc,flags+['-DSC_MUTANT_'+define],t+'.mutation.'+name+'.compile',core if case in CORE else service,binary);run(t+'.mutation.'+name,[binary,case],41,marker)
    observations.append(dict(profile=t,differential=diff,base_cases=360,burst_cycles=1024,cancel_complete_schedules=4,retry_shutdown_orders=2,recovery_faults=5,process_private_write_observed=True,format_capacities=[1,2,3],physical=False))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'));assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,models=models,limits=['Bounded author execution, not independent reviewer or delayed learner assessment.','Original local synthetic targets only; %n demonstrates a permitted-type marker write, not arbitrary code execution.','Owned copied input assumes live stable storage during the call; ordinary fields require external serialization.','Finite logical schedules, not native-thread concurrency or external callback quiescence.','Fork private-memory observation is not complete isolation; inherited descriptors and OS policy are separate.','Work counters and fixed storage are not measured worst-case latency or resistance to an unlimited external flood.','Exact final published C17/C23 and physical observations remain separate; other T35/T36/T37 obligations not closed.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
