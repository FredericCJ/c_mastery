"""Execute strict-C component, lifetime/qualification and actual object/image audits."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/qualified_views';REF=ROOT/'assessor/qualified_views';RUNTIME=ROOT/'assessor/execution/support.c';REPORT='reviews/v5/expansion/legacy_qualified_views/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['FV_INIT_PASS zero_values null_pointer designated assignment compound_durations','FV_VIEW_PASS caller_owned live_alias detached_snapshot empty capacity_atomic','FV_QUALIFIER_PASS restrict_disjoint overlap_direction volatile_RAM_only','FV_PASS qualified_views']
MUTANTS={1:'initialized.values[0]==0&&initialized.values[3]==0&&initialized.link==NULL',2:'snapshot[0]==31',3:'overlap[0]==1&&overlap[1]==1&&overlap[2]==2'};STUB='fv_reset(&initialized)==FV_OK'
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('fv_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),RUNTIME,ROOT/'assessor/legacy_systems/toolchain/common.py',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/qualified-views'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;count=None
 tools=['gcc','clang','nm','objdump','readelf','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  oracle,count=module('fv_oracle',REF/'oracle.py').generate(c)
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-ffreestanding','-fno-builtin','-I'+str(LAB)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'views.c'
    c.run([cc,*flags,'-DFV_MUTANT='+str(0 if v=='stub' else v),src,RUNTIME,LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all value/lifetime/view mechanisms executed')
   c.group=tag+':oracle';exe=w/(tag+'-oracle');c.run([cc,*flags,REF/'views.c',RUNTIME,oracle,'-o',exe]);c.run([exe],contains='FV_ORACLE_PASS snapshot_ranges=11979',env=env)
  module('fv_experiments',REF/'experiments.py').run(c,ROOT)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    c.group=cc+':target-O'+opt;dest=w/(cc+'-target-O'+opt)
    c.run([sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'views.c','--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FV_TARGET_BUILD_PASS explicit_objects=4 no_archives')
    c.run(emulator(dest/'image.elf'),contains='FV_TARGET_PASS caller_owner live_snapshot overlap reset opaque volatile_model')
    c.run([sys.executable,REF/'resource_audit.py',dest],contains='"passed": true')
    c.run(['arm-none-eabi-objdump','-drwC',dest/'image.elf'])
    if opt=='3':
     code=c.run(['arm-none-eabi-objdump','-d','--disassemble=fv_device_twice',dest/'image.elf'],contains='<fv_device_twice>');c.require(code.count('[r0, #0]')==2,'actual Thumb two volatile loads')
  c.group='target-stub';dest=w/'target-stub'
  c.run([sys.executable,LAB/'target_build.py','--build',dest,'--source',LAB/'learner.c','--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FV_TARGET_BUILD_PASS explicit_objects=4 no_archives');c.run(emulator(dest/'image.elf'),expected=1,contains='FV_TARGET_FAIL')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),'--runtime',RUNTIME,*(['--source',REF/'views.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  c.run([sys.executable,LAB/'run.py','--build',w/'missing-provider'],expected=2,contains='--runtime')
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(snapshot_ranges=count,profiles=6,semantic_mutants=3,code_access_mutant=1,target_profiles=4,target_stub=1,max_extent=8),limits=d['limits']+['Strict profile C component has no source calls to external C library; actual native objects and four direct-linked Thumb images audited. Explicit original memory helpers close implicit calls. Hosted harness/sanitizers are verification only.','No libc/libgcc/compiler-rt/libatomic archive in target link; complete object/image/map/generated-code/stack records retained.','Volatile RAM model and observed two native/Thumb loads do not establish real device width/order, barriers, cache or interthread synchronization.','Expired pointers, uninitialized values and qualification-breaking execution remain analysis-only; negative translations are separate from compiled semantic mutants.','Overlap oracle uses saved original snapshots,11979finite ranges. No arbitrary pointer validity inference from capacities.','External review, independent learner and seven-day performance remain unassessed; emulator is not physical observation.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
