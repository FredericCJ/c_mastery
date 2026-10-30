"""Execute exact T04 bit/codec/register mechanisms, independent models and target closure."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/bit_contract';REF=ROOT/'assessor/bit_contract';RUNTIME=ROOT/'assessor/execution/support.c';INTEGER=ROOT/'assessor/integer_kernel/integer.c';INTEGER_INCLUDE=ROOT/'labs/integer_kernel';REPORT='reviews/v5/expansion/legacy_bit_contract/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKERS=['FB_BITS_PASS logical_mask counts half_open overlap zero_stats','FB_CODEC_PASS endian offsets split_spans reject_skip unknown_payload_unread','FB_PROTOCOL_PASS W1C RTC no_unrequested_read bounded_version_size','FB_PASS bit_contract']
MUTANTS={1:'fb_shift(8,0x1234,4,FB_RIGHT,&out)==FB_OK&&out==3',2:'FB_CONFLICT&&out==917',3:'stats.leading==8',4:'FB_SHORT&&out==917',5:'r.state==0xa3&&r.reads==0',6:'p.max_items==0'};STUB='fb_shift(16,0x1234,-1,FB_LEFT,&out)==FB_RANGE&&out==917'

def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('fi_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():return sorted({Path(__file__),RUNTIME,ROOT/'assessor/legacy_systems/toolchain/common.py',INTEGER,INTEGER_INCLUDE/'integer.h',ROOT/'labs/support/cm_parse.c',ROOT/'labs/support/cm_parse.h',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/bit-contract'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None
 tools=['gcc','clang','nm','objdump','readelf','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  for cc in ['gcc','clang']:c.run([cc,'-dumpmachine'])
  c.run(['uname','-a'])
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-ffreestanding','-fno-builtin','-I'+str(LAB),'-I'+str(INTEGER_INCLUDE)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for v in [0,*MUTANTS,'stub']:
    c.group=tag+':'+str(v);exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'bits.c'
    c.run([cc,*flags,'-DFB_MUTANT='+str(0 if v=='stub' else v),src,INTEGER,RUNTIME,LAB/'tests.c','-o',exe])
    out=c.run([exe],expected=int(v!=0),contains=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v],env=env)
    c.require(not any(x in out for x in ['runtime error:','ERROR: AddressSanitizer']),'defined semantic rejection or reference execution')
    if v==0:c.require(all(x in out for x in MARKERS),'all bit and protocol contract mechanisms executed')
   c.group=tag+':oracle';exe=w/(tag+'-oracle');c.run([cc,*flags,REF/'bits.c',INTEGER,RUNTIME,REF/'oracle.c','-o',exe]);c.run([exe],contains='FB_ORACLE_PASS stats=2048 shifts=39936 fields=548864 patches=51984',env=env)
  module('fb_experiments',REF/'experiments.py').run(c,ROOT)
  c.group='runtime-provider';obj=w/'original-runtime.o';c.run(['gcc','-std=c17','-O2','-ffreestanding','-fno-builtin','-c',RUNTIME,'-o',obj]);undefined=c.run(['nm','-u',obj]);defined=c.run(['nm','--defined-only',obj]);c.require(not undefined.strip() and {'memcpy','memset','memmove'}<={line.split()[-1] for line in defined.splitlines()},'original helper provider closes itself and defines allowed calls')
  for cc in ['gcc','clang']:
   for opt in ['0','3']:
    c.group=cc+':object-O'+opt;obj=w/(cc+'-O'+opt+'.o');c.run([cc,'-std=c17','-O'+opt,'-ffreestanding','-fno-builtin','-fstack-usage','-I'+str(LAB),'-I'+str(INTEGER_INCLUDE),'-c',REF/'bits.c','-o',obj]);und=c.run(['nm','-u',obj]);c.require(set(line.split()[-1] for line in und.splitlines())<={'fi_encode_cells','fi_decode_cells','memcpy','memset','memmove'},'native component calls only actual integer prerequisite and audited original helpers');c.run(['objdump','-drwC',obj])
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    c.group=cc+':target-O'+opt;dest=w/(cc+'-target-O'+opt)
    c.run([sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'bits.c','--integer-source',INTEGER,'--integer-include',INTEGER_INCLUDE,'--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FB_TARGET_BUILD_PASS explicit_objects=5 no_archives')
    c.run(emulator(dest/'image.elf'),contains='FB_TARGET_PASS shifts fields codec prior_integer register_models version_bounds')
    c.run([sys.executable,REF/'resource_audit.py',dest],contains='"passed": true');c.run(['arm-none-eabi-objdump','-drwC',dest/'image.elf'])
  c.group='target-stub';dest=w/'target-stub';c.run([sys.executable,LAB/'target_build.py','--build',dest,'--source',LAB/'learner.c','--integer-source',INTEGER,'--integer-include',INTEGER_INCLUDE,'--runtime',RUNTIME,'--driver',REF/'target.c'],contains='FB_TARGET_BUILD_PASS explicit_objects=5 no_archives');c.run(emulator(dest/'image.elf'),expected=1,contains='FB_TARGET_FAIL')
  c.group='public-runner'
  for cc in ['gcc','clang']:
   for ref in [False,True]:c.run([sys.executable,LAB/'run.py','--cc',cc,'--build',w/(cc+'-public-'+str(ref)),'--integer-source',INTEGER,'--integer-include',INTEGER_INCLUDE,'--runtime',RUNTIME,*(['--source',REF/'bits.c'] if ref else [])],expected=int(not ref),contains=MARKERS[-1] if ref else STUB)
  c.run([sys.executable,LAB/'run.py','--build',w/'missing-provider'],expected=2,contains='--runtime')
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(stats=2048,shifts=39936,fields=548864,patches=51984,intrinsic_cases=1048576,profiles=6,semantic_mutants=6,intrinsic_mutant=1,target_profiles=4,layout_profiles=8,plugin_providers=8,plugin_consumers=16),limits=d['limits']+['Strict C17 core implements logical widths1..16 and imports actual prior integer cell codec. Independent bit-list oracle covers all8-bit values and all4-bit simultaneous/sequential edit combinations.','Actual native objects and four direct-linked M4 images audited; five explicit objects and no libc/libgcc/compiler-rt/libatomic archive. Original helpers explicitly named.','Actual packing/bit-field layout and shared plugins are Linux x86-64 ABI observations. The wire descriptor is separately encoded; plugin version cannot prove foreign code safety or a different ABI.','Teaching register W1C/RTC is an original explicit state machine; no actual MMIO, device behavior, barriers or physical timing claim.','Intrinsic capability/zero wrapper is a separate comparison; unsafe overlays/clz(0)/shift examples compile-only.','External review, independent learner, seven-day transfer and physical evidence remain absent. Finite checks do not prove absolute mastery.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
