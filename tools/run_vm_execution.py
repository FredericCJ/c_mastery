"""Exact T46.03/04 native, independent arithmetic, strict M4 and big-endian campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/runtime_followthrough/execution';REF=ROOT/'assessor/runtime_followthrough/execution'
PROVIDERS={'verifier':'assessor/runtime_followthrough/verifier/verifier.c','cvm':'assessor/runtime/cvm.c','runtime':'assessor/execution/support.c','division':'assessor/legacy_systems/freestanding_depth/divide.c','signed-c':'assessor/runtime_followthrough/representation/signed_division.c','signed-asm':'assessor/runtime_followthrough/representation/signed_division.S','driver':'assessor/runtime_followthrough/execution/target.c'}
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'vs_arithmetic(VS_WRAP_V2, OP_ADD, INT32_MAX, 1, &n) == CVM_OK',2:'vs_complete(&machine, r.epoch + 1, r.serial, 0, 99) == VS_STATE',3:'b.integer == 91',4:'vs_step(&machine, &out) >= VS_RUNNING',5:'vs_start(&machine, &p, 0, 8, 0, 1, 8, &scratch) == VS_STATE',6:'vs_take(&machine, &r) == VS_STATE'}
MARKER='VS_PASS checked_dispatch_owned_continuation_explicit_frames'

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return sorted({Path(__file__),*[ROOT/p for p in PROVIDERS.values()],ROOT/'labs/runtime/include/cvm.h',ROOT/'labs/runtime_followthrough/verifier/verifier.h',ROOT/'labs/runtime/SPEC.md',ROOT/'labs/machine_image/freestanding_depth/free.h',ROOT/'labs/smp_depth/acpi/big_endian.S',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def target_args(dest,cc,opt,variant=0):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(0 if variant=='stub' else variant),'--source',(LAB/'learner.c' if variant=='stub' else REF/'execution.c'),*[x for key,p in PROVIDERS.items() for x in ['--'+key,ROOT/p]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/runtime-execution'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False,stdin=None):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log');inp=None
  with log.open('w') as f:
   try:
    if stdin:inp=Path(stdin).open('rb')
    rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,stdin=inp,env=env,timeout=180).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
   finally:
    if inp:inp.close()
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer','VS_FORMATTER_UNSUPPORTED']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,stdin=str(stdin) if stdin else None,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm',str(qemu),str(lld)]
 toolhash={t:sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);inc=['-I'+str(p) for p in [LAB,ROOT/'labs/runtime_followthrough/verifier',ROOT/'labs/runtime/include',ROOT/'labs/machine_image/freestanding_depth']];fixture=w/'integers.txt'
 run('independent integer corpus',[sys.executable,REF/'oracle.py',fixture],marker='"cases": 18696')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'execution.c'
   if run(tag+' '+str(v)+' compile',[cc,*flags,'-DVS_MUTANT='+str(0 if v=='stub' else v),src,ROOT/PROVIDERS['cvm'],ROOT/PROVIDERS['verifier'],LAB/'tests.c','-o',exe]):run(tag+' '+str(v)+' execute',[exe],int(v!=0),MARKER if v==0 else 'vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == VS_RUNNING' if v=='stub' else MUTANTS[v])
  exe=w/(tag+'-oracle')
  if run(tag+' oracle compile',[cc,*flags,REF/'execution.c',ROOT/PROVIDERS['cvm'],ROOT/PROVIDERS['verifier'],REF/'oracle.c','-o',exe]):run(tag+' independent oracle',[exe],marker='VS_ORACLE_PASS cases=18696',stdin=fixture)
  exe=w/(tag+'-stack')
  if run(tag+' stack compile',[cc,*flags,'-Wno-pedantic','-finstrument-functions','-fstack-usage',REF/'execution.c',ROOT/PROVIDERS['verifier'],ROOT/PROVIDERS['cvm'],REF/'stack_probe.c','-o',exe]):run(tag+' stack execute',[exe],marker='VS_STACK_PASS guest_frames=16 interpreter_c_depth=1 native_depth=17 storage_modes=2')
 for opt in ['0','3']:

  for v in [0,*MUTANTS,'stub']:
   exe=w/('ppc64-O'+opt+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'execution.c';label='PPC64-O'+opt+' '+str(v)
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17','-O'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-Wl,--gc-sections','-DVS_STANDALONE','-DVS_PPC64','-DVS_MUTANT='+str(0 if v=='stub' else v),*inc]
   if run(label+' build',['clang',*flags,src,ROOT/PROVIDERS['cvm'],ROOT/PROVIDERS['verifier'],LAB/'tests.c',ROOT/PROVIDERS['runtime'],REF/'target.c',ROOT/'labs/smp_depth/acpi/big_endian.S','-o',exe]):
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],int(v!=0),MARKER if v==0 else 'vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == VS_RUNNING' if v=='stub' else MUTANTS[v])
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   label=cc+'-O'+opt;dest=w/label
   if run(label+' build',target_args(dest,cc,opt),marker='VS_TARGET_BUILD_PASS explicit_objects=10 original_limb_division'):
    elf=dest/'image.elf';run(label+' execute',emulator(elf),marker='VS_TARGET_PASS');run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' size',['arm-none-eabi-size','-A',elf]);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true')
 for v in [4,5,6,'stub']:
  label='target '+str(v);dest=w/label.replace(' ','-')
  if run(label+' build',target_args(dest,'arm-none-eabi-gcc','0',v),marker='VS_TARGET_BUILD_PASS explicit_objects=10 original_limb_division'):run(label+' execute',emulator(dest/'image.elf'),1,'vs_start(&machine, &p, 0, 10, 0, 0, 0, &scratch) == VS_RUNNING' if v=='stub' else MUTANTS[v])
 for stub in [False,True]:run('public '+str(stub),[sys.executable,LAB/'run.py','--verifier',ROOT/PROVIDERS['verifier'],'--cvm',ROOT/PROVIDERS['cvm'],'--build',w/('public-'+str(stub)),*([] if stub else ['--source',REF/'execution.c'])],int(stub),'VS_CHECK' if stub else MARKER)
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,'--verifier')
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=toolhash,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Original CVM1 provider unchanged; explicit execution successor owns its program and continuation, with checkedV1/wrappingV2 and verified terminal TAIL24.','18696 finite mathematical arithmetic cases per native profile, six compiling semantic mutations, invocation traces, pending completion identities and explicit target profiles.','M4 ILP32 little endian and PPC64 ELFv2 big endian are executed emulation, not physical observations or all-ABI proof.','Original signed division ABI wrapper closes emitted helper via actual T11 limb provider; no runtime archive or hosted target dependencies.','Assessor providers are explicit author artifacts; independent review, actual learner/delayed work and physical observation remain not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
