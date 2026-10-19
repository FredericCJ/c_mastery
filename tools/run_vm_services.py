"""Exact T46.10 native, independent receipt protocol, strict M4 and big-endian campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/runtime_followthrough/services';REF=ROOT/'assessor/runtime_followthrough/services'
PROVIDERS={'cvm':'assessor/runtime/cvm.c','store':'assessor/storage/store.c','adapter':'labs/runtime/adapters/store_host.c','runtime':'assessor/execution/support.c','driver':'assessor/runtime_followthrough/services/target.c'}
HEADERS=Path('/home/frede/cmastery-freestanding-tools/root/usr/include/newlib')
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'call(&reader, 1, 7, 1, 0, &out) == CVM_HOST_ERROR',2:'call(&host, 2, 0, 1, 0, &out) == CVM_HOST_ERROR',3:'disk.event == events',4:'call(&host, 1, 20, 1, 0, &out) == CVM_HOST_ERROR',5:'n == 17'}
MARKER='VH_PASS actual_VM_store_capabilities_versioned_receipts'
STUB='call(&writer, 1, 7, 1, 0, &out) == CVM_OK'

CORE=['cvm','store','adapter']


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return sorted({Path(__file__),*[ROOT/p for p in PROVIDERS.values()],ROOT/'labs/runtime/include/cvm.h',ROOT/'labs/runtime/SPEC.md',ROOT/'labs/storage/include/store.h',ROOT/'labs/storage/FORMAT.md',ROOT/'labs/storage/INTEGRATION.md',ROOT/'labs/runtime/adapters/store_host.h',ROOT/'labs/smp_depth/acpi/big_endian.S',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})

def target_args(dest,cc,opt,variant=0):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(0 if variant=='stub' else variant),'--headers',HEADERS,'--source',(LAB/'learner.c' if variant=='stub' else REF/'services.c'),*[x for key,p in PROVIDERS.items() for x in ['--'+key,ROOT/p]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/runtime-services'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False,stdin=None):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log');inp=None
  with log.open('w') as f:
   try:
    if stdin:inp=Path(stdin).open('rb')
    rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,stdin=inp,env=env,timeout=180).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
   finally:
    if inp:inp.close()
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer','VH_FORMATTER_UNSUPPORTED']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,stdin=str(stdin) if stdin else None,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm',str(qemu),str(lld)]
 toolhash={t:sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);inc=['-I'+str(p) for p in [LAB,ROOT/'labs/runtime/include',ROOT/'labs/storage/include',ROOT/'labs/runtime/adapters']];fixture=w/'requests.txt'
 run('independent protocol corpus',[sys.executable,REF/'oracle.py',fixture],marker='"cases": 1944')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])
  for v in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'services.c'
   if run(tag+' '+str(v)+' compile',[cc,*flags,'-DVH_MUTANT='+str(0 if v=='stub' else v),src,*[ROOT/PROVIDERS[k] for k in CORE],LAB/'medium.c',LAB/'tests.c','-o',exe]):run(tag+' '+str(v)+' execute',[exe],int(v!=0),MARKER if v==0 else STUB if v=='stub' else MUTANTS[v])
  exe=w/(tag+'-oracle')
  if run(tag+' oracle compile',[cc,*flags,REF/'services.c',*[ROOT/PROVIDERS[k] for k in CORE],LAB/'medium.c',REF/'oracle.c','-o',exe]):run(tag+' independent oracle',[exe],marker='VH_ORACLE_PASS histories=648 cases=1944',stdin=fixture)
 for opt in ['0','3']:

  for v in [0,*MUTANTS,'stub']:
   exe=w/('ppc64-O'+opt+'-'+str(v));src=LAB/'learner.c' if v=='stub' else REF/'services.c';label='PPC64-O'+opt+' '+str(v)
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17','-O'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-Wl,--gc-sections','-DVH_STANDALONE','-DVH_PPC64','-DVH_MUTANT='+str(0 if v=='stub' else v),'-isystem',str(HEADERS),*inc]
   if run(label+' build',['clang',*flags,src,*[ROOT/PROVIDERS[k] for k in CORE],LAB/'medium.c',LAB/'tests.c',ROOT/PROVIDERS['runtime'],REF/'target.c',ROOT/'labs/smp_depth/acpi/big_endian.S','-o',exe]):
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],int(v!=0),MARKER if v==0 else STUB if v=='stub' else MUTANTS[v])
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   label=cc+'-O'+opt;dest=w/label
   if run(label+' build',target_args(dest,cc,opt),marker='VH_TARGET_BUILD_PASS explicit_objects=9 SSRAM4M'):
    elf=dest/'image.elf';run(label+' execute',emulator(elf),marker='VH_TARGET_PASS');run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' size',['arm-none-eabi-size','-A',elf]);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true')
 for v in [1,2,3,4,5,'stub']:
  label='target '+str(v);dest=w/label.replace(' ','-')
  if run(label+' build',target_args(dest,'arm-none-eabi-gcc','0',v),marker='VH_TARGET_BUILD_PASS explicit_objects=9 SSRAM4M'):run(label+' execute',emulator(dest/'image.elf'),1,STUB if v=='stub' else MUTANTS[v])
 for stub in [False,True]:run('public '+str(stub),[sys.executable,LAB/'run.py',*[x for k in CORE for x in ['--'+k,ROOT/PROVIDERS[k]]],'--build',w/('public-'+str(stub)),*([] if stub else ['--source',REF/'services.c'])],int(stub),'VH_CHECK' if stub else MARKER)
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,'--cvm')
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=toolhash,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Actual retained CVM1 and fixed-key store adapter imported; capabilities are embedding policy, not native memory safety.', '648 independent finite request histories /1944 transitions per native profile, all74 modeled failure cuts and five compiling semantic controls.', 'M4 SSRAM4M is an explicit separate QEMU10.2.1 AN386 emulator mapping profile with256KiB stack reserve, not the original64KiB target or physical observation.', 'PPC64 ELFv2 big endian executes original codecs; strict images link explicit original objects without runtime archives.', 'One serialized client, latest receipt, exclusive valid store; no general exactly-once, arbitrary crash medium or multi-client claim.', 'Assessor providers are explicit author artifacts; independent review, actual learner/delayed work and physical observation remain not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
