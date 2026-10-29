"""Exact T46.09 native, independent graph reachability, strict M4 and big-endian campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/runtime_followthrough/budgets';REF=ROOT/'assessor/runtime_followthrough/budgets'
PROVIDERS={'execution':'assessor/runtime_followthrough/execution/execution.c','verifier':'assessor/runtime_followthrough/verifier/verifier.c','objects':'assessor/object_runtime/object_runtime.c','registrations':'assessor/object_runtime/object_runtime_vm.c','pool':'assessor/algorithms/algo.c','cvm':'assessor/runtime/cvm.c','runtime':'assessor/execution/support.c','division':'assessor/legacy_systems/freestanding_depth/divide.c','signed-c':'assessor/runtime_followthrough/representation/signed_division.c','signed-asm':'assessor/runtime_followthrough/representation/signed_division.S','driver':'assessor/runtime_followthrough/budgets/target.c'}
HEADERS=Path('/home/frede/cmastery-freestanding-tools/root/usr/include/newlib')
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'r.fuel == 2 && r.gc == 0 && r.host == 0 && r.allocations == 0 && r.collections == 0',2:'r.fuel == 5 && r.gc == 3',3:'r.host == 1 && calls == 1',4:'out.integer == 91',5:'r.fuel == 9 && r.gc == 19 && r.host == 1'}
MARKER='VL_PASS independent_limits_distinct_work_pending'
STUB='vl_run(&vm, &p, &out, &r) == CVM_OK && out.integer == 7'
CORE=['cvm','execution','verifier','objects','registrations','pool']


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return sorted({Path(__file__),*[ROOT/p for p in PROVIDERS.values()],ROOT/'labs/runtime/include/cvm.h',ROOT/'labs/runtime_followthrough/execution/execution.h',ROOT/'labs/runtime_followthrough/verifier/verifier.h',ROOT/'labs/runtime_followthrough/collection/port_registration.py',ROOT/'labs/object_runtime/include/object_runtime.h',ROOT/'labs/object_runtime/include/object_runtime_vm.h',ROOT/'labs/algorithms/include/algo.h',ROOT/'labs/runtime/SPEC.md',ROOT/'labs/machine_image/freestanding_depth/free.h',ROOT/'labs/smp_depth/acpi/big_endian.S',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})
def target_args(dest,cc,opt,variant=0):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(0 if variant=='stub' else variant),'--headers',HEADERS,'--source',(LAB/'learner.c' if variant=='stub' else REF/'budgets.c'),*[x for key,p in PROVIDERS.items() for x in ['--'+key,ROOT/p]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/runtime-budgets'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False,stdin=None):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log');inp=None
  with log.open('w') as f:
   try:
    if stdin:inp=Path(stdin).open('rb')
    rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,stdin=inp,env=env,timeout=180).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
   finally:
    if inp:inp.close()
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer','VL_FORMATTER_UNSUPPORTED']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,stdin=str(stdin) if stdin else None,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm',str(qemu),str(lld)]
 toolhash={t:sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);inc=['-I'+str(p) for p in [LAB,ROOT/'labs/runtime_followthrough/execution',ROOT/'labs/runtime_followthrough/verifier',ROOT/'labs/object_runtime/include',ROOT/'labs/algorithms/include',ROOT/'labs/runtime/include',ROOT/'labs/machine_image/freestanding_depth']];fixture=w/'ledgers.txt'
 run('independent ledger corpus',[sys.executable,REF/'oracle.py',fixture],marker='"cases": 693')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'budgets.c'
   if run(tag+' '+str(v)+' compile',[cc,*flags,'-DVL_MUTANT='+str(0 if v=='stub' else v),src,*[ROOT/PROVIDERS[k] for k in CORE],LAB/'tests.c','-o',exe]):run(tag+' '+str(v)+' execute',[exe],int(v!=0),MARKER if v==0 else STUB if v=='stub' else MUTANTS[v])
  exe=w/(tag+'-oracle')
  if run(tag+' oracle compile',[cc,*flags,REF/'budgets.c',*[ROOT/PROVIDERS[k] for k in CORE],REF/'oracle.c','-o',exe]):run(tag+' independent oracle',[exe],marker='VL_ORACLE_PASS cases=693',stdin=fixture)
  exe=w/(tag+'-blocking')
  if run(tag+' blocking compile',[cc,*flags,REF/'budgets.c',*[ROOT/PROVIDERS[k] for k in ['cvm','execution','verifier']],REF/'blocking.c','-o',exe]):
   for mode in ['blocking','pending','cancel']:run(tag+' '+mode+' experiment',[exe,mode],marker='VL_BLOCK_PASS mode='+mode)
  exe=w/(tag+'-nonblocking-control')
  if run(tag+' nonblocking control compile',[cc,*flags,'-DVL_BLOCK_SKIP_READ=1',REF/'budgets.c',*[ROOT/PROVIDERS[k] for k in ['cvm','execution','verifier']],REF/'blocking.c','-o',exe]):run(tag+' nonblocking control execute',[exe,'blocking'],1,'VL_BLOCK_REFUSE no_completed_result_before_release')
 for opt in ['0','3']:

  for v in [0,*MUTANTS,'stub']:
   exe=w/('ppc64-O'+opt+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'budgets.c';label='PPC64-O'+opt+' '+str(v)
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17','-O'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-Wl,--gc-sections','-DVL_STANDALONE','-DVL_PPC64','-DVL_MUTANT='+str(0 if v=='stub' else v),'-isystem',str(HEADERS),*inc]
   if run(label+' build',['clang',*flags,src,*[ROOT/PROVIDERS[k] for k in CORE],LAB/'tests.c',ROOT/PROVIDERS['runtime'],REF/'target.c',ROOT/'labs/smp_depth/acpi/big_endian.S','-o',exe]):
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],int(v!=0),MARKER if v==0 else STUB if v=='stub' else MUTANTS[v])
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   label=cc+'-O'+opt;dest=w/label
   if run(label+' build',target_args(dest,cc,opt),marker='VL_TARGET_BUILD_PASS explicit_objects=14 original_limb_division'):
    elf=dest/'image.elf';run(label+' execute',emulator(elf),marker='VL_TARGET_PASS');run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' size',['arm-none-eabi-size','-A',elf]);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true')
 for v in [2,3,4,5,'stub']:
  label='target '+str(v);dest=w/label.replace(' ','-')
  if run(label+' build',target_args(dest,'arm-none-eabi-gcc','0',v),marker='VL_TARGET_BUILD_PASS explicit_objects=14 original_limb_division'):run(label+' execute',emulator(dest/'image.elf'),1,STUB if v=='stub' else MUTANTS[v])
 for stub in [False,True]:run('public '+str(stub),[sys.executable,LAB/'run.py',*[x for k in CORE for x in ['--'+k,ROOT/PROVIDERS[k]]],'--build',w/('public-'+str(stub)),*([] if stub else ['--source',REF/'budgets.c'])],int(stub),'VL_CHECK' if stub else MARKER)
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,'--cvm')
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=toolhash,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Actual retained CVM1/struct or_vm and frozen Vs/verifier imported; semantic ledgers are not cycles or wall-clock bounds.', '693 independently derived fuel/GC/host combinations per native profile; seven limit causes, five compiling accounting controls and actual changed moving-collector cost25 versus original21.', 'Owned local POSIX pipe provider deliberately lacks its own deadline; harness observes100ms no completion then releases or kills/reaps only its child. Finite observations are not proof of infinite waiting.', 'M4 ILP32 little endian and PPC64 ELFv2 big endian are executed emulation, not physical observations or all-ABI proof; original helpers only, explicit struct or_vm enum source port.', 'Assessor providers are explicit author artifacts; independent review, actual learner/delayed work and physical observation remain not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
