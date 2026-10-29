"""Actual local descriptor/exec authority with source-bound prior MPU evidence."""
import datetime,hashlib,json,os,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_worker';PUBLIC=ROOT/'labs/security_depth/worker';PRIVATE=ROOT/'assessor/security_depth/worker';BUILD=ROOT/'build'/('security-worker-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['race','policy','inheritance'];PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS={'exclusive':('SD_MUTANT_EXCLUSIVE','race','sd_exclusive(d, "chosen", &sentinel) == EEXIST'),'policy':('SD_MUTANT_POLICY','policy','sd_request(1, SD_WRITE_CONFIG, fd, out) == EACCES'),'permission':('SD_MUTANT_PERMISSION','policy','sd_request(0, SD_READ_LOG, fd, out) == EACCES'),'inherit':('SD_MUTANT_INHERIT','inheritance','inventory=FAIL'),'intermediate':('PN_MUTANT=1','race','pn_open_confined(d, "middle/item", &sentinel) == ELOOP')}
MPU='research/v5/expansion/b06/boot_pair_validation.json'
IMPORTS=['tools/run_thread_lifecycle.py','labs/posix_lifecycle_depth/namespace/path.h','labs/posix_lifecycle_depth/namespace/path_stub.c','labs/posix_lifecycle_depth/namespace/CONTRACTS.md','assessor/posix_lifecycle_depth/namespace/path.c','labs/security_depth/command/process_boundary.c','fixtures/embedded/PR-T35-02.json',MPU]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 files += [ROOT/n for n in json.loads((ROOT/MPU).read_text())['input_sha256']]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))}
def expected_commands():
 out={'environment.kernel','environment.libc','environment.identity','models'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   t=cc+'.'+p;out|={t+'.'+n+'.compile' for n in ['worker','native','learner','process']};out|={t+'.'+c for c in CASES+['process']};out|={t+'.learner.'+c for c in CASES}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);owned=BUILD/'owned-temp';owned.mkdir();old=os.environ.get('SD_TMP_ROOT');os.environ['SD_TMP_ROOT']=str(owned);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};observations=[]
 def compile(cc,flags,ident,sources,out):run(ident,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*flags,'-I',PUBLIC,'-I',ROOT/'labs/posix_lifecycle_depth/namespace',*sources,'-o',out]);harness.OUTPUTS.add(out)
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version']);run('environment.identity',['id'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    t=cc+'.'+p;worker=BUILD/(t+'.worker');compile(cc,flags,t+'.worker.compile',[PRIVATE/'worker.c',PUBLIC/'echo_worker.c'],worker)
    sources=[PRIVATE/'worker.c',ROOT/'assessor/posix_lifecycle_depth/namespace/path.c',PUBLIC/'tests.c'];native=BUILD/(t+'.native');compile(cc,flags,t+'.native.compile',sources,native)
    for case in CASES:run(t+'.'+case,[native,case,*([worker] if case=='inheritance' else [])],marker='OBS '+case)
    stub=BUILD/(t+'.learner');compile(cc,flags,t+'.learner.compile',[PUBLIC/'learner.c',ROOT/'assessor/posix_lifecycle_depth/namespace/path.c',PUBLIC/'tests.c'],stub)
    for case in CASES:run(t+'.learner.'+case,[stub,case,*([worker] if case=='inheritance' else [])],41,'SD_ASSERT:')
    process=BUILD/(t+'.process');compile(cc,flags,t+'.process.compile',[ROOT/'labs/security_depth/command/process_boundary.c'],process);run(t+'.process',[process],marker='child_private_write=isolated')
    for name,(define,case,marker) in MUTANTS.items():
     binary=BUILD/(t+'.mutation.'+name);compile(cc,flags+['-D'+define],t+'.mutation.'+name+'.compile',sources,binary);run(t+'.mutation.'+name,[binary,case,*([worker] if case=='inheritance' else [])],41,marker)
    observations.append(dict(profile=t,actual_cases=CASES,descriptor_inventory=[0,4],high_parent_descriptor=900,acquisition_failures=4,exec_failure='ENOENT',argv_literal=True,child_option_interpretation=True,ambient_worker_reopen=True,physical=False))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'));assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 finally:
  assert owned.resolve().is_relative_to((ROOT/'build').resolve()) and owned.name=='owned-temp';shutil.rmtree(owned)
  if old is None:os.environ.pop('SD_TMP_ROOT',None)
  else:os.environ['SD_TMP_ROOT']=old
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,models=models,owned_temporary_tree_removed=not owned.exists(),limits=['Original owned synthetic files/children only, no external target or host security change.','Single-threaded bounded original launcher; logical concurrent-fork counterexample is not native thread stress.','Actual readonly descriptor and private mapping do not remove ambient reopen/network/same-user authority.','Actual frozen pn_open_confined producer used; no weaker fallback or authenticity claim.','MPU region analysis reuses source-bound prior MPS2 execution, not fresh struct lr_cpu or physical STM32 evidence.','Source-bound author validation is not independent review or delayed learner assessment; other T35/T36/T37 criteria remain.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
