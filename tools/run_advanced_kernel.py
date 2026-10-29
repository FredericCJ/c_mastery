"""Source-bound copied-request, queue/backend and finite relation campaign."""
import datetime,hashlib,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_kernel';PUBLIC=ROOT/'labs/advanced_concurrency_depth/kernel';PRIVATE=ROOT/'assessor/advanced_concurrency_depth/kernel';BUILD=ROOT/'build'/('advanced-kernel-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['copy','backend'];PROFILES=[('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];MUTANTS={'copy':('COPY','copy','ak_copy(&p, n, copying, &c) == (r ? AK_FAULT : AK_OK)'),'sync':('SYNC','backend','c == d->maps && d->stage == 2'),'quiesce':('QUIESCE','backend','c == d->maps && !d->device && d->stage == 4')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/execution/include/cmx.h','labs/advanced_concurrency_depth/spsc/spsc.h','labs/advanced_concurrency_depth/spsc/CONTRACTS.md','labs/advanced_concurrency_depth/spsc/learner.c','assessor/advanced_concurrency_depth/spsc/spsc.c','fixtures/systems/PR-T26-03.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for t in [PUBLIC,PRIVATE] for p in t.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def expected_commands():
 out={'models'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   t=cc+'.'+p;out|={t+'.compile',t+'.learner.compile',*[t+'.'+c for c in CASES],*[t+'.learner.'+c for c in CASES]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;models={};observations=[]
 def compile(cc,flags,ident,sources,out):run(ident,[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*flags,'-I',PUBLIC,'-I',ROOT/'labs/advanced_concurrency_depth/spsc','-I',ROOT/'labs/execution/include',*sources,'-o',out]);harness.OUTPUTS.add(out)
 try:
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    t=cc+'.'+p;sources=[PRIVATE/'kernel.c',ROOT/'assessor/advanced_concurrency_depth/spsc/spsc.c',PUBLIC/'tests.c'];binary=BUILD/t;compile(cc,flags,t+'.compile',sources,binary)
    for c in CASES:run(t+'.'+c,[binary,c],marker='OBS '+c)
    stub=BUILD/(t+'.learner');compile(cc,flags,t+'.learner.compile',[PUBLIC/'learner.c',ROOT/'assessor/advanced_concurrency_depth/spsc/spsc.c',PUBLIC/'tests.c'],stub)
    for c in CASES:run(t+'.learner.'+c,[stub,c],41,'AK_ASSERT:')
    for name,(define,case,marker) in MUTANTS.items():
     mutant=BUILD/(t+'.mutation.'+name);compile(cc,flags+['-DAK_MUTANT_'+define],t+'.mutation.'+name+'.compile',sources,mutant);run(t+'.mutation.'+name,[mutant,case],41,marker)
    observations.append(dict(profile=t,copy_residual_cases=560,actual_spsc=True,retained_mapping_until_quiescence=True,physical=False,serialization='one external owner'))
  models=json.loads(run('models',[sys.executable,PRIVATE/'models.py'],marker='"status": "pass"'));assert set(x['id'] for x in harness.COMMANDS)==expected_commands()
 except Exception as ex:failure=str(ex)
 after=pins()
 if before!=after:failure='Scoped source changed'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,models=models,limits=['Source-bound author evidence, not external review or independent learner assessment.','Native copy callback is a controlled residual-count model; actual Linux user-copy belongs to the separately executed frozen T57 guest producer.','Queue is the actual frozen struct as_queue; backend is a serialized synthetic DMA ownership adapter, not board/cache/coherency evidence.','Hand-derived pinned LKMM MP fragment is not execution of herd7 or a complete memory-model interpreter.','Final published C17/C23, physical observations and unrestricted kernel/board claims remain separate.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
