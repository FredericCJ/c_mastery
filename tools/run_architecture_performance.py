"""Exact dimensional work plus native paired measurements of real prior kernels."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_performance';PUBLIC=ROOT/'labs/architecture_performance';PRIVATE=ROOT/'assessor/architecture_performance';BUILD=ROOT/'build'/('architecture-performance-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
CASES=['equation','stats','equal'];PACKETS=dict(equation=6,stats=1,equal=5125);PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS={'UNITS':('equation','r.baseline_ns.n==900000','PE_MUTANT_UNITS'),'COMM':('equation','r.new_ns.n==540000+costs[i]','PE_MUTANT_COMM'),'OUTLIER':('stats','s.maximum==110','PE_MUTANT_OUTLIER'),'TRUST':('equal','pe_asm(bad,2,1,&m)==ALGO_BAD','PE_MUTANT_TRUST'),'SIGNED':('equal','m==expected','ALGO_ASM_SIGNED')}
IMPORTS=['tools/run_thread_lifecycle.py','fixtures/algorithms/PR-T37-08.json','assessor/algorithms/algo.c','assessor/algorithm_abi/bound.c','assessor/algorithms/x86_64.S','labs/algorithms/src/x86_64.S','labs/algorithms/include/algo.h','labs/algorithm_abi/bound.h','labs/algorithm_abi/CONTRACTS.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','lscpu']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','cpu','cpuinfo','generate','drift'}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.stack',cc+'.core.sizes'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES],'measure.compile','measure','analysis','measure.disassembly','no_work.compile','no_work']};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated';strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/algorithm_abi'),'-I'+str(ROOT/'labs/algorithms/include')]
 def compile(tag,cc,flags,measure=False,stub=False):
  binary=BUILD/tag;src=[PUBLIC/'learner.c' if stub else PRIVATE/'performance.c',PUBLIC/'prior_learner.c' if stub else generated/'prior.c',ROOT/('labs/algorithms/src/x86_64.S' if stub else 'assessor/algorithms/x86_64.S'),PUBLIC/('measure.c' if measure else 'tests.c')];run(tag+'.compile',[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary);return binary
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('cpu',['lscpu']);run('cpuinfo',['cat','/proc/cpuinfo']);run('generate',[sys.executable,PRIVATE/'generate.py',generated],marker='OBS exact checked lower-bound C providers');harness.OUTPUTS.update(generated.iterdir());run('drift',[sys.executable,PRIVATE/'drift.py'],marker='"status": "pass"')
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=compile(tag,cc,flags)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=compile(tag+'.stub',cc,flags,stub=True)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'PE_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():mutant=compile(tag+'.mutation.'+m,cc,[*flags,'-D'+definition]);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    measure=compile(tag+'.measure',cc,flags,measure=True);run(tag+'.measure',[measure],marker='PE_MEASURE_PASS raw=248 no_speed_threshold');run(tag+'.analysis',[sys.executable,PRIVATE/'analyze.py',BUILD/(tag+'.measure.log')],marker='"status": "pass"');run(tag+'.measure.disassembly',['objdump','-d',measure]);harness.OUTPUTS.add(BUILD/(tag+'.measure.analysis.json'));bad=compile(tag+'.no_work',cc,[*flags,'-DPE_MUTANT_NO_WORK'],measure=True);run(tag+'.no_work',[bad],41,'PE_MEASURE_ASSERT: calibration result/checksum');observations.append(dict(profile=tag,independent_packets=PACKETS,raw_native_samples=248,samples_per_condition=31,conditions=8,physical_mcu=False,equal_admission=True,producer='algo_lower + ab_checked + original algo_asm_lower'))
   obj=BUILD/(cc+'.core.o');stack=obj.with_suffix('.su');run(cc+'.core',[cc,*strict,'-O2','-ffreestanding','-fno-builtin','-fstack-usage','-c',PRIVATE/'performance.c','-o',obj]);harness.OUTPUTS.update([obj,stack]);u=run(cc+'.core.undefined',['nm','-u',obj]);s={q.split()[-1] for q in u.splitlines()};assert {'algo_lower','ab_checked'}<=s and s<={'algo_lower','ab_checked','memcpy','memset','_GLOBAL_OFFSET_TABLE_'};run(cc+'.core.stack',['cat',stack]);run(cc+'.core.sizes',['nm','-S','--size-sort',obj])
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact three PR-T37-08 ordinary/changed mechanisms with native host observations; no independently assessed learner or elapsed delay.','All31 samples/condition retained; no required speedup threshold, outlier filtering or overhead subtraction.','Same sortedness admission on actual reused C and assembly providers; measured checked API includes validation/calls, not isolated raw loop.','Size and cache-preparation interventions are separate;8MiB displacement before a batch is not proof of a cold cache for every query.','Frequency, interrupts, virtualization and intervening struct lr_cpu migration remain uncontrolled; endpoint struct lr_cpu equality is not a no-migration proof.','Instrumented and uninstrumented profiles are distinguished; no physical Cortex-M cycles, WCET, energy or universal ranking.','Exact rational/drift models remain synthetic, separately labeled; final ISO and independent review gaps unchanged.'];r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':main()
