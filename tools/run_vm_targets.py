"""Actual same-core Linux/BE/M4/LTO and frozen independently implemented guest."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/runtime_followthrough/targets';REF=ROOT/'assessor/runtime_followthrough/targets'
PROVIDERS={'cvm':'assessor/runtime/cvm.c','probe':'labs/kernel_followthrough/runtime/probe.c','runtime':'assessor/execution/support.c','driver':'assessor/runtime_followthrough/targets/target.c'}
KERNEL='reviews/v5/expansion/legacy_kernel_followthrough/runtime_frozen.json'
HEADERS=Path('/home/frede/cmastery-freestanding-tools/root/usr/include/newlib')
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MARKER='VX_PROBE_PASS same_core_six_rows'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 paths={Path(__file__),ROOT/KERNEL,ROOT/'tools/run_kernel_runtime_followthrough.py',*[ROOT/p for p in PROVIDERS.values()],ROOT/'labs/runtime/include/cvm.h',ROOT/'labs/runtime/src/cvm.c',ROOT/'labs/runtime/SPEC.md',ROOT/'labs/smp_depth/acpi/big_endian.S',*[p for base in [LAB,REF,ROOT/'labs/kernel_followthrough/runtime'] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]}
 # Pinned guest receipts include exact source, command, image and raw output.
 d=json.loads((ROOT/KERNEL).read_text());paths.update(ROOT/p for p in d['inspected_paths'])
 for g in d['guests']:
  paths.add(ROOT/g['receipt']);r=json.loads((ROOT/g['receipt']).read_text());paths.update(ROOT/p for p in r['artifact_sha256'])
 return sorted(paths)
def target_args(dest,cc,opt,probe=None,extra=()):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--headers',HEADERS,*[x for key,p in PROVIDERS.items() for x in ['--'+key,probe if key=='probe' and probe else ROOT/p]],*extra]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/runtime-targets'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,env=env,timeout=180).returncode
   except (OSError,subprocess.TimeoutExpired) as e:f.write(str(e));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer','VX_FORMATTER_UNSUPPORTED']);rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return log if ok else None
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm',str(qemu),str(lld)];toolhash={t:sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);inc=['-I'+str(p) for p in [ROOT/'labs/runtime/include',ROOT/'labs/kernel_followthrough/runtime']];alternate=w/'probe_plus2.c';run('adapter successor',[sys.executable,LAB/'port_adapter.py','--source',ROOT/PROVIDERS['probe'],'--output',alternate],marker='R-VX-HOST-PLUS2-2026-01')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])
  for adapter in [1,2]:
   exe=w/(tag+'-'+str(adapter));label=tag+' adapter'+str(adapter);probe=ROOT/PROVIDERS['probe'] if adapter==1 else alternate
   if run(label+' build',[cc,*flags,ROOT/PROVIDERS['cvm'],probe,LAB/'driver.c','-o',exe]):
    log=run(label+' execute',[exe],marker=MARKER)
    if log:run(label+' compare',[sys.executable,REF/'compare.py',log,'--adapter',adapter],marker='"passed": true')
 for opt in ['0','3']:
  for adapter in [1,2]:
   exe=w/('ppc64-O'+opt+'-'+str(adapter));label='PPC64-O'+opt+' adapter'+str(adapter);probe=ROOT/PROVIDERS['probe'] if adapter==1 else alternate
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17','-O'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-Wl,--gc-sections','-DVX_PPC64','-isystem',str(HEADERS),*inc]
   if run(label+' build',['clang',*flags,ROOT/PROVIDERS['cvm'],probe,LAB/'driver.c',ROOT/PROVIDERS['runtime'],REF/'target.c',ROOT/'labs/smp_depth/acpi/big_endian.S','-o',exe]):
    run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);log=run(label+' execute',[qemu,exe],marker=MARKER)
    if log:run(label+' compare',[sys.executable,REF/'compare.py',log,'--adapter',adapter],marker='"passed": true')
 for cc,opt,lto in [('arm-none-eabi-gcc','0',False),('arm-none-eabi-gcc','3',False),('clang','0',False),('clang','3',False),('arm-none-eabi-gcc','3',True)]:
  for adapter in [1,2]:
   label=cc+'-O'+opt+('-LTO' if lto else '')+' adapter'+str(adapter);dest=w/label.replace(' ','-');probe=ROOT/PROVIDERS['probe'] if adapter==1 else alternate
   if run(label+' build',target_args(dest,cc,opt,probe,['--poison-bss']+(['--lto'] if lto else [])),marker='VX_TARGET_BUILD_PASS'):
    log=run(label+' execute',emulator(dest/'image.elf'),marker='VX_TARGET_PASS')
    if log:run(label+' compare',[sys.executable,REF/'compare.py',log,'--adapter',adapter],marker='"passed": true')
    run(label+' resources',[sys.executable,REF/'audit.py',dest],marker='"passed": true');run(label+' size',['arm-none-eabi-size','-A',dest/'image.elf']);run(label+' assembly',['arm-none-eabi-objdump','-drwC',dest/'image.elf'])
 dest=w/'skip-bss'
 if run('skip bss build',target_args(dest,'arm-none-eabi-gcc','0',extra=['--poison-bss','--skip-bss']),marker='VX_TARGET_BUILD_PASS'):run('skip bss execute',emulator(dest/'image.elf'),1,'VX_STARTUP_FAIL')
 run('implicit helper rejection',target_args(w/'helper','arm-none-eabi-gcc','0',extra=['--helper',REF/'implicit_helper.c']),3,'VX_UNEXPLAINED_IMPORT __aeabi_uldivmod')
 run('unrooted LTO link rejection',target_args(w/'unrooted','arm-none-eabi-gcc','3',extra=['--lto','--unrooted-lto-control']),1,'undefined reference to')
 run('kernel frozen guard',[sys.executable,ROOT/'tools/run_kernel_runtime_followthrough.py','--check',ROOT/KERNEL],marker='12 actual profiles PASS')
 kd=json.loads((ROOT/KERNEL).read_text())
 for g in kd['guests']:
  if g['case']!=26:continue
  r=json.loads((ROOT/g['receipt']).read_text());raw=next(c['output'] for c in r['commands'] if c['label']=='guest');run('kernel '+g['compiler']+'-'+g['optimization']+' compare',[sys.executable,REF/'compare.py',ROOT/raw,'--kernel'],marker='"passed": true')
  names=['bytecode.cvm','archive-bytecode.cvm'];paths=[p for p in r['artifact_sha256'] if Path(p).name in names]
  rows.append(dict(label='kernel '+g['compiler']+'-'+g['optimization']+' bytes',passed=len(paths)==2 and all((ROOT/p).read_bytes()==bytes.fromhex('43564d31010000080000000129000000160111') for p in paths),artifacts=paths))
 for stub in [False,True]:run('public '+str(stub),[sys.executable,LAB/'run.py','--cvm',ROOT/('labs/runtime/src/cvm.c' if stub else PROVIDERS['cvm']),'--build',w/('public-'+str(stub))],int(stub),'VX_FAIL:2' if stub else MARKER)
 run('missing provider refused',[sys.executable,LAB/'run.py','--build',w/'missing'],2,'--cvm')
 # Guest archive inputs were checked above; separately capture the actual
 # fixture bytes produced by hosted.c and compare them without a codec oracle.
 exe=w/'fixture-producer';fixture=w/'fixture.cvm'
 if run('fixture producer build',['gcc','-std=c17','-O0',*inc,ROOT/'labs/kernel_followthrough/runtime/hosted.c',ROOT/PROVIDERS['probe'],ROOT/PROVIDERS['cvm'],'-o',exe]):run('fixture producer execute',[exe,fixture],marker='KF_VM_TAIL:')
 rows.append(dict(label='exact fixture bytes',passed=fixture.is_file() and fixture.read_bytes()==bytes.fromhex('43564d31010000080000000129000000160111')))
 sample=next(r for r in rows if r['label']=='gcc-debug adapter1 execute');raw=(ROOT/sample['raw']).read_text()
 for i,(before,after) in enumerate([('KF_VM:0:0:42','KF_VM:0:0:43'),('KF_VM:1:10:-99:0:0:30:0','KF_VM:1:10:-99:1:0:30:0'),('KF_VM:4:7:-99:1:0:0:0','KF_VM:4:7:-99:1:0:1:0'),('VX_BYTES:67','VX_BYTES:68'),('KF_VM_TAIL:1:6:7','KF_VM_TAIL:1:6:6')]):
  assert raw.count(before)==1;bad=w/('bad-'+str(i)+'.txt');bad.write_text(raw.replace(before,after));run('comparison control '+str(i),[sys.executable,REF/'compare.py',bad],1,'AssertionError')
 rows.append(dict(label='source stability',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=toolhash,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Original core imported unchanged; explicit callback successor changes native policy only. All same-image comparisons use independent three-instruction cost reasoning.','Frozen actual original own-kernel ring3 routes are source/receipt/artifact checked and compared, not newly executed by this consumer. GCC/Clang O0/O2 baseline case26; other freeze cases separately scoped.','M4 ROM256KiB/RAM64KiB/32KiB stack reserve, interrupts masked; bounded nonrecursive C call graph. BE ELFv2 ABI, hosted Linux and M4 do not imply board validation.','Object helper rejection and unrooted LTO failure are build/dependency controls; skip-BSS is an actual semantic startup control. None counts as arbitrary exploit or learner mastery.','Six native sanitizer/optimization profiles, two endian optimization profiles, five M4 including GCC LTO, two callback policies; external review, independent learner/delayed and physical evidence not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
