"""Execute exact T04 integer mechanisms, independent models and target closure."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/integer_kernel';REF=ROOT/'assessor/integer_kernel';RUNTIME=ROOT/'assessor/execution/support.c';REPORT='reviews/v5/expansion/legacy_integer_kernel/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['FI_ARITHMETIC_PASS reject_saturate sign_partitions division_conventions symmetric_range','FI_BOUNDARY_PASS signed_before_size narrow_count checked_layout logical_offsets octet_cells','FI_PASS integer_kernel']
MUTANTS={1:'fi_binary(byte,FI_ADD,120,10,&out)==FI_RANGE&&out==917',2:'r==remainders[i]',3:'fi_to_size(-1,SIZE_MAX,&out)==FI_RANGE&&out==917',4:'fi_extent(1,0,0,0,&out)==FI_RANGE&&out==917',5:'clipped==1'};STUB=MUTANTS[1]
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('fi_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),RUNTIME,ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'assessor/foundation_depth/depth.c',ROOT/'labs/foundation_depth/include/depth.h',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/integer-kernel'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None
 tools=['gcc','clang','nm','objdump','readelf','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  c.group='models';models=module('fi_models',REF/'models.py');native=models.generate(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-ffreestanding','-fno-builtin','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'integer.c'
    c.run([cc,*flags,'-DFI_MUTANT='+str(0 if v=='stub' else v),src,RUNTIME,LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all integer contract mechanisms executed')
   for kind,src,marker in [('oracle',REF/'oracle.c','FI_ORACLE_PASS arithmetic=392451 saturation=392451 division=392451'),('native',native,'FI_NATIVE_PASS arithmetic=972 division=972 independent_bigints')]:
    c.group=tag+':'+kind;exe=w/(tag+'-'+kind);c.run([cc,*flags,REF/'integer.c',RUNTIME,src,'-o',exe]);c.run([exe],contains=marker,env=env)
  models.run(c,ROOT)
  for cc in ['gcc','clang']:
   for opt in ['0','3']:
    c.group=cc+':object-O'+opt;obj=w/(cc+'-O'+opt+'.o');c.run([cc,'-std=c17','-O'+opt,'-ffreestanding','-fno-builtin','-fstack-usage','-I'+str(LAB),'-c',REF/'integer.c','-o',obj]);und=c.run(['nm','-u',obj]);c.require(not und.strip(),'native component closure without runtime helpers');c.run(['objdump','-drwC',obj])
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    c.group=cc+':target-O'+opt;dest=w/(cc+'-target-O'+opt)
    c.run([sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'integer.c','--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FI_TARGET_BUILD_PASS explicit_objects=4 no_archives')
    c.run(emulator(dest/'image.elf'),contains='FI_TARGET_PASS manual_arithmetic no_wider_guards size32_input64 logical_offsets wire_octets')
    c.run([sys.executable,REF/'resource_audit.py',dest],contains='"passed": true');c.run(['arm-none-eabi-objdump','-drwC',dest/'image.elf'])
  c.group='target-stub';dest=w/'target-stub';c.run([sys.executable,LAB/'target_build.py','--build',dest,'--source',LAB/'learner.c','--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FI_TARGET_BUILD_PASS explicit_objects=4 no_archives');c.run(emulator(dest/'image.elf'),expected=1,contains='FI_TARGET_FAIL')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),'--runtime',RUNTIME,*(['--source',REF/'integer.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  c.run([sys.executable,LAB/'run.py','--build',w/'missing-provider'],expected=2,contains='--runtime')
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(reduced_arithmetic=392451,reduced_saturation=392451,reduced_division=392451,native_arithmetic=972,native_division=972,builtin_operations=196608,prior_extents=6561,profiles=6,semantic_mutants=5,publication_mutant=1,target_profiles=4),limits=d['limits']+['Manual C17 core uses no wider type for arithmetic guards; independent host oracles use representable small math or Python arbitrary precision. Native edge oracle explicitly int32.','Actual native objects and four direct-linked M4 images audited. M4 size32/intmax64 boundary is executed; native intmax cannot represent SIZE_MAX. No libc/libgcc/compiler-rt/libatomic archive in target link.','Actual compiler builtin and capability-checked C23 stdckdint experiments are separate from C17 core; imported unchanged foundation_depth provider has CHAR_BIT8 restriction.','Hypothetical int16 and CHAR_BIT16 are analysis/model results, not target observations. Analysis-only undefined specimens never linked or executed.','External review, independent learner, seven-day transfer and physical evidence remain absent. Finite checks do not prove absolute mastery.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
