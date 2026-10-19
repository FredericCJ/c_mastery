"""Execute exact T04 floating contracts, changed assumptions and independent oracles."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/float_contract';REF=ROOT/'assessor/float_contract';REPORT='reviews/v5/expansion/legacy_float_contract/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKER='FF_PASS sum compensation rejection comparison ulps environment restoration_failures'
MUTANTS={1:'ff_sum(tri,3,FF_NEUMAIER,&out)==FF_OK && out==1',2:'ff_sum(bad,2,FF_NEUMAIER,&out)==FF_RANGE && out==917',3:'ff_compare(-0.0,0.0,0,0,1,&eq)==FF_OK && !eq',4:'ff_ulps(-DBL_TRUE_MIN,DBL_TRUE_MIN,&distance)==FF_OK && distance==2',5:'fegetround()==FE_DOWNWARD && fetestexcept(FE_ALL_EXCEPT)==flags && errno==EDOM'}
STUB=MUTANTS[1];FIXED=ROOT/'assessor/b02_followthrough/fixed_point/fixed_point.c';B02=ROOT/'assessor/b02_depth/depth.c';FIXED_INCLUDE=ROOT/'labs/b02_followthrough/fixed_point';B02_INCLUDE=ROOT/'labs/b02_depth/include'
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('ff_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',FIXED,B02,FIXED_INCLUDE/'fixed_point.h',B02_INCLUDE/'depth.h',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/float-contract'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;tools=['gcc','clang','nm','objdump','readelf','size']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  models=module('ff_models',REF/'models.py');c.group='exact-models';models.independent(c);cases,inp=models.prepare(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-frounding-math','-ffp-contract=off','-fno-fast-math','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'float_contract.c';c.run([cc,*flags,'-DFF_MUTANT='+str(0 if v=='stub' else v),src,LAB/'tests.c','-lm','-o',exe]);text=c.run([exe],expected=int(v!=0),contains=MARKER if v==0 else STUB if v=='stub' else MUTANTS[v],env=env);c.require(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'reference or defined semantic mutant outcome')
   for name in ['oracle_driver','boundaries']:
    c.group=tag+':'+name;exe=w/(tag+'-'+name);c.run([cc,*flags,REF/'float_contract.c',REF/(name+'.c'),'-lm','-o',exe]);text=c.run([exe,*([inp] if name=='oracle_driver' else [])],contains='FF_ORACLE_DRIVER_PASS cases=3917' if name=='oracle_driver' else 'FF_BOUNDARY_PASS double_pairs=24 float_pairs=8 algorithmic_neighbors',env=env)
    if name=='oracle_driver':models.check(c,cases,text)
  module('ff_experiments',REF/'experiments.py').run(c,ROOT)
  exact=json.loads((w/'exact-models.json').read_text())['fixed_point']
  for cc in ['gcc','clang']:
   c.group=cc+':fixed-prerequisite';exe=w/(cc+'-fixed');c.run([cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(FIXED_INCLUDE),'-I'+str(B02_INCLUDE),REF/'fixed_point.c',FIXED,B02,'-o',exe]);text=c.run([exe],contains='FF_FIXED_PASS prior_provider scale16 ties_even saturation');actual=[list(map(int,s.split()[1:])) for s in text.splitlines() if s.startswith('FIXED ')];c.require(actual==[[r[k] for k in ['a','b','rounded_raw','output']] for r in exact],'actual earlier fixed point provider matches independent rational rounding')
   for opt in ['0','3']:
    c.group=cc+':object-O'+opt;obj=w/(cc+'-O'+opt+'.o');c.run([cc,'-std=c17','-O'+opt,'-frounding-math','-ffp-contract=off','-fno-fast-math','-fstack-usage','-I'+str(LAB),'-c',REF/'float_contract.c','-o',obj]);c.run(['nm','-u',obj]);c.run(['objdump','-drwC',obj]);c.run(['size',obj]);c.require(any(p.name.startswith(cc+'-O'+opt) and p.suffix=='.su' and 'ff_sum' in p.read_text() and 'ff_probe_ops' in p.read_text() for p in w.iterdir()),'actual hosted object stack usage')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),*(['--source',REF/'float_contract.c'] if ref else [])],expected=int(not ref),contains=MARKER if ref else STUB)
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 external=['/home/frede/cmastery-library-tools/sysroot/usr/include/mpfr.h','/home/frede/cmastery-library-tools/sysroot/usr/include/x86_64-linux-gnu/gmp.h','/usr/lib/x86_64-linux-gnu/libmpfr.so.6','/usr/lib/x86_64-linux-gnu/libgmp.so.10']
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},external_oracle_sha256={p:sha(Path(p)) for p in external if Path(p).is_file()},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(profiles=6,mutants=5,rational_cases=3917,double_boundary_pairs=24,float_boundary_pairs=8,compiler_modes=7,timing_trials=35,rounding_modes=4,injected_environment_failures=7,changed_radix_models=4,fixed_cases=6),limits=d['limits']+['Hosted profile A, C17. GCC floating options and Clang FENV_ACCESS plus actual runtime observations; no claim all compilers implement the full pragma contract.','MPFR/GMP are host-side independent oracle dependencies, never target runtime or first-principles sum implementations. Rational model does not inherit nominal long-double precision.','Four actual rounding modes, process-local SSE FTZ with restoration, narrow -mlong-double-64 arithmetic probe, actual x87 and contracted/noncontracted objects; no physical target measurements.','Strict and fast kernels linked to strict caller; timings only on admitted exact finite dyadic workload. Exceptional controls violate the relaxed compiler contract: local observations, not portable C goldens. Clang negative translation is separate from semantic mutants.','Prior fixed-point reference provider is imported unchanged; no future T17/T12 learner component is required.','Restore failure is reported without publishing results, but a failing platform restore cannot guarantee unchanged environment.','External review, independent learner, seven-day transfer and physical evidence remain absent. Finite checks do not prove absolute mastery.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
