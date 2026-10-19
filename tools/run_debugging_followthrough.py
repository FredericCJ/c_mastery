"""Source-bound T44 author execution. Owned bounded processes; no host settings."""
import argparse,hashlib,json,os,re,subprocess,sys,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/debugging_followthrough';REF=ROOT/'assessor/debugging_followthrough';BASE=ROOT/'reviews/v5/expansion/legacy_debugging_followthrough'
RETAINED=['assessor/architecture_frame/ledger.c','labs/architecture_frame/ledger.h','labs/architecture_frame/learner.c','assessor/interrupt_depth/frame/frame.c','labs/interrupt_depth/frame/frame.h','labs/interrupt_depth/frame/frame.c','assessor/firmware_io/firmware_io.c','labs/firmware_io/include/firmware_io.h','labs/firmware_io/src/firmware_io.c','assessor/thread_depth/waiting/queue.c','labs/thread_depth/waiting/waiting.h','labs/thread_depth/waiting/test_support.h','labs/thread_depth/waiting/queue_tests.c','labs/testing_followthrough/diagnostic.c','labs/testing_followthrough/race.c','labs/algorithms/include/algo.h','labs/algorithms/tests/abi_x86_64.S','labs/algorithms/tests/abi_thumb.S','assessor/algorithms/x86_64.S','assessor/algorithms/thumb.S','labs/algorithms/thumb/link.ld']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted({p for p in [*LAB.rglob('*'),*REF.rglob('*'),*[ROOT/p for p in RETAINED],*(ROOT/'fixtures/debugging_lab').glob('PR-T44-*.json'),Path(__file__)] if p.is_file() and '__pycache__' not in p.parts})
def main(report):
 dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve()) and not dest.exists();out=dest.with_suffix('');out.mkdir(parents=True,exist_ok=False);before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};commands=[];checks=[];obs={}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:exitcode=86:allow_addr2line=1:external_symbolizer_path=/usr/bin/addr2line',UBSAN_OPTIONS='halt_on_error=1:exitcode=86',TSAN_OPTIONS='halt_on_error=1:exitcode=86:external_symbolizer_path=/usr/bin/addr2line')
 def run(label,command,code=0,timeout=60,extra=None):
  command=list(map(str,command));start=time.monotonic();r=subprocess.run(command,cwd=ROOT,env=dict(env,**(extra or {})),capture_output=True,text=True,timeout=timeout);log=out/(label+'.txt');log.write_text(r.stdout+r.stderr);commands.append({'label':label,'command':command,'cwd':str(ROOT),'exit_code':r.returncode,'expected_exit_code':code,'seconds':time.monotonic()-start,'environment_overrides':extra or {},'output':log.relative_to(ROOT).as_posix()});assert code is None or r.returncode==code,(label,r.returncode,r.stdout,r.stderr);return r.stdout+r.stderr
 def check(name,value):checks.append({'name':name,'passed':bool(value)});assert value,name
 def compile(cc,name,files,flags=(),strict=True):
  exe=out/name;run(name+'-build',[cc,'-std=c17','-Wall','-Wextra',*(['-Werror','-pedantic'] if strict else []),*includes,*flags,*files,'-o',exe]);return exe
 def gdb(name,binary,mode):
  text=run(name,['gdb','-q','-batch','-x',LAB/'gdb_observe.py','--args',binary],extra={'DX_GDB_MODE':mode});value=json.loads(text.split('DX_GDB_JSON:',1)[1].splitlines()[0]);value.update(binary_sha256=sha(binary),symbols_sha256=sha(binary),native_core=False);obs[name]=value;return text
 includes=['-I'+str(ROOT/p) for p in ('labs/debugging_followthrough','labs/architecture_frame','labs/interrupt_depth/frame','labs/firmware_io/include','labs/thread_depth/waiting','labs/algorithms/include')]
 core=[LAB/'tests.c',REF/'diagnosis.c',ROOT/'assessor/architecture_frame/ledger.c',ROOT/'assessor/interrupt_depth/frame/frame.c',ROOT/'assessor/firmware_io/firmware_io.c']
 try:
  for tool in ('gcc','clang','gdb','valgrind','qemu-system-arm'):run(tool+'-version',[tool,'--version'])
  run('kernel',['uname','-a'])
  obs['certificates']=json.loads(run('certificates',[sys.executable,LAB/'certificates.py']));check('distinct-premise-certificates',obs['certificates']['passed'] and len(obs['certificates']['records'])==12)
  for bad in (0,1):
   hidden=compile('gcc','hidden-'+str(bad),[REF/'hidden.c'],['-O2','-DDX_HIDDEN_BAD='+str(bad)]);check('author-hidden-oracle-'+str(bad),('stale-view-accepted' if bad else 'expired-borrow-rejected') in run('hidden-'+str(bad),[hidden]))
  for cc in ('gcc','clang'):
   obj=out/(cc+'-core.o');helper=out/(cc+'-byte.o');closed=out/(cc+'-closed.o');run(cc+'-core-object',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-Ilabs/debugging_followthrough','-c',REF/'diagnosis.c','-o',obj]);run(cc+'-core-undefined',['nm','-u',obj]);run(cc+'-byte-object',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-c',REF/'byte_support.c','-o',helper]);run(cc+'-core-close',[cc,'-r','-nostdlib',obj,helper,'-o',closed]);check(cc+'-runtime-closure',not run(cc+'-closed-undefined',['nm','-u',closed]).strip())
   for profile,flags in [('debug',['-O0','-g']),('release',['-O2','-DNDEBUG'])]:
    prefix=cc+'-'+profile;exe=compile(cc,prefix+'-core',core,flags);check(prefix+'-core','DX_PASS:core' in run(prefix+'-core',[exe]));allocation=compile(cc,prefix+'-allocation',[LAB/'allocations.c'],flags);check(prefix+'-allocation','forced-relocation:no-stale-pointer-read' in run(prefix+'-allocation',[allocation]))
    for repaired in (0,1):
     for logging in (0,1):
      name=prefix+f'-specimen-{repaired}-{logging}';binary=compile(cc,name,[LAB/'specimen.c'],[*flags,'-DDX_FIXED='+str(repaired),'-DDX_LOGGING='+str(logging)],False);assignment=run(name+'-assignment',[binary,'assignment']);check(name+'-assignment',assignment==('ADMIT:0:0\nADMIT:1:1\nADMIT:4:1\nADMIT:5:0\n' if repaired else 'ADMIT:0:1\nADMIT:1:1\nADMIT:4:1\nADMIT:5:1\n'));check(name+'-macros',f'MACROS:1:1:{logging}:{logging+1}' in run(name+'-macros',[binary,'macros']));check(name+'-first-transition',('COUNTER:3:0:0' if repaired else 'COUNTER:3:1:5') in run(name+'-counter',[binary,'counter']))
   raw=compile(cc,cc+'-raw-counter',[LAB/'specimen.c'],['-O0','-g','-DDX_CLAMP=0'],False);check(cc+'-raw-history','HISTORY:1,2,3,4,5,4' in run(cc+'-raw-history',[raw,'history']));check(cc+'-fixed-history','HISTORY:1,2,3,4,4,3' in run(cc+'-fixed-history',[out/(cc+'-debug-specimen-1-1'),'history']))
   parenthesized=compile(cc,cc+'-parenthesized',[LAB/'specimen.c'],['-O2','-DDX_PAREN=1'],False);check(cc+'-warning-suppression-not-repair','ADMIT:0:1' in run(cc+'-parenthesized',[parenthesized,'assignment']))
   fixed=out/(cc+'-debug-specimen-1-1');check(cc+'-stale-config','ADMIT:5:1' in run(cc+'-stale-config',[fixed,'assignment'],extra={'DX_CAP':'8'}))
   host=compile(cc,cc+'-host',[LAB/'host.c'],['-O2','-g','-pthread']);check(cc+'-host','ignored-EPIPE' in run(cc+'-host',[host]))
   queue=compile(cc,cc+'-queue',[LAB/'queue.c',ROOT/'labs/thread_depth/waiting/queue_tests.c',ROOT/'assessor/thread_depth/waiting/queue.c'],['-O2','-g','-pthread']);check(cc+'-queue','cancel-near-wake-ownership-checked' in run(cc+'-queue',[queue]))
   queue_tsan=compile(cc,cc+'-queue-tsan',[LAB/'queue.c',ROOT/'labs/thread_depth/waiting/queue_tests.c',ROOT/'assessor/thread_depth/waiting/queue.c'],['-O1','-g','-fsanitize=thread','-pthread']);check(cc+'-queue-tsan','DX_PASS:queue' in run(cc+'-queue-tsan',[queue_tsan]))
   schedules=compile(cc,cc+'-schedules',[LAB/'schedules.c'],['-O0','-g','-pthread']);check(cc+'-lost-wake','predicate1' in run(cc+'-lost-wake',[schedules]));check(cc+'-timeout-cycle','timeout-breaks-cycle' in run(cc+'-cycle',[schedules,'cycle']))
   for bad in (0,1):
    name=cc+'-abi-'+str(bad);exe=compile(cc,name,[LAB/'abi.c',ROOT/'assessor/algorithms/x86_64.S',ROOT/'labs/algorithms/tests/abi_x86_64.S'],['-O0','-g','-DDX_ABI_BAD='+str(bad),*(['-DALGO_ASM_ABI'] if bad else [])]);check(name,'result=1' in run(name,[exe]))
   for repaired in (0,1):
    for instrument,flags in [('plain',['-O0','-g']),('asan',['-O1','-g','-fsanitize=address','-fno-omit-frame-pointer','-no-pie']),('ubsan',['-O1','-g','-fsanitize=undefined'])]:
     name=f'{cc}-{instrument}-diagnostic-{repaired}';exe=compile(cc,name,[ROOT/'labs/testing_followthrough/diagnostic.c'],[*flags,'-DDIAG_REPAIRED='+str(repaired)],False)
     for mode in ((0,1,2) if instrument in ('plain','asan') else (3,)):
      label=name+'-'+str(mode)
      if instrument=='plain':text=run(label,['valgrind','--tool=memcheck','--track-origins=yes','--error-exitcode=89',exe,str(mode)],0 if repaired else 89);marker='ERROR SUMMARY: 0 errors' if repaired else ['Invalid write','Invalid read','Conditional jump or move depends on uninitialised'][mode]
      elif instrument=='asan':expected=86 if not repaired and mode in (0,1) else 0;text=run(label,[exe,str(mode)],expected);marker=['heap-buffer-overflow','heap-use-after-free'][mode] if expected else 'DIAG_TERMINATED'
      else:text=run(label,[exe,str(mode)],0 if repaired else 86);marker='DIAG_TERMINATED' if repaired else 'signed integer overflow'
      check(label,marker in text)
    name=f'{cc}-tsan-{repaired}';exe=compile(cc,name,[ROOT/'labs/testing_followthrough/race.c'],['-O1','-g','-fsanitize=thread','-pthread','-DRACE_REPAIRED='+str(repaired)]);check(name,('RACE_COMPLETED:20000' if repaired else 'WARNING: ThreadSanitizer: data race') in run(name,[exe],0 if repaired else 86))
    name=f'{cc}-copied-init-{repaired}';exe=compile(cc,name,[LAB/'uninitialized.c'],['-O0','-g','-DDX_FIXED='+str(repaired)],False);check(name,('ERROR SUMMARY: 0 errors' if repaired else 'Conditional jump or move depends on uninitialised') in run(name,['valgrind','--tool=memcheck','--track-origins=yes','--error-exitcode=89',exe],0 if repaired else 89))
   for weak in (0,1):
    exe=compile(cc,cc+'-static-runtime-'+str(weak),[LAB/'static.c'],['-O0','-g','-DDX_WEAK='+str(weak)]);check(cc+'-unreached-'+str(weak),run(cc+'-unreached-'+str(weak),[exe])=='')
   allocation=compile(cc,cc+'-allocation-sanitized',[LAB/'allocations.c'],['-O2','-g','-fsanitize=address,undefined','-no-pie']);check(cc+'-allocation-sanitized','forced-relocation' in run(cc+'-allocation-sanitized',[allocation]))
   weak=compile(cc,cc+'-static-reached',[LAB/'static.c'],['-O0','-g','-fsanitize=address','-no-pie','-DDX_WEAK=1'],False);check(cc+'-same-static-dynamic-transition','stack-buffer-overflow' in run(cc+'-static-reached',[weak,'4'],86))
   diagnostic_core=compile(cc,cc+'-core-sanitized',core,['-O1','-g','-fsanitize=address,undefined','-no-pie']);check(cc+'-semantic-under-tools','65536:failures32640' in run(cc+'-core-sanitized',[diagnostic_core]))
  lib=out/'diagnosis.so';run('reducer-library',['gcc','-std=c17','-fPIC','-shared','-Ilabs/debugging_followthrough',REF/'diagnosis.c','-o',lib]);binary=out/'gcc-debug-specimen-0-1';symbols=out/'specimen.debug';run('separate-symbols',['objcopy','--only-keep-debug',binary,symbols]);new_source=out/'new-fixed-source.c';new_source.write_text('#define DX_FIXED 1\n'+(LAB/'specimen.c').read_text());new_binary=compile('gcc','new-fixed-source',[new_source],['-O0','-g'],False);new_symbols=out/'new-fixed.debug';run('new-separate-symbols',['objcopy','--only-keep-debug',new_binary,new_symbols]);obs['replay']=json.loads(run('replay',[sys.executable,LAB/'replay.py',binary,LAB/'specimen.c',symbols,lib,new_source,new_binary,new_symbols]));check('replay-tristate-native-C-reduction',obs['replay']['passed'] and obs['replay']['native_C_reducer']['selected']=='[A]')
  for profile,flags in [('Og',['-Og']),('O2',['-O2']),('LTO',['-O2','-flto'])]:
   exe=compile('gcc','gdb-'+profile,[LAB/'specimen.c'],[*flags,'-g'],False);run('mapping-'+profile,['objdump','-d','-S','--disassemble=dx_acquire',exe]);gdb('scope-'+profile,exe,'scope')
  for mode in ('counter','candidate','relocate'):gdb('debugger-'+mode,out/'gcc-raw-counter' if mode=='counter' else binary,mode)
  for mode in ('lost','cycle','thread-watch'):gdb('debugger-'+mode,out/'gcc-schedules',mode)
  check('actual-first-invalid-watch',' = 5' in json.dumps(obs['debugger-counter']) and 'hw watchpoint' in json.dumps(obs['debugger-counter']))
  check('actual-caller-callee',' = 7' in json.dumps(obs['scope-Og']) and ' = 2' in json.dumps(obs['scope-Og']))
  check('actual-optimized-unavailable','optimized out' in json.dumps(obs['scope-O2']))
  for bad in (0,1):
   elf=out/('thumb-'+str(bad)+'.elf');run('thumb-build-'+str(bad),['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-Os','-ffreestanding','-fno-builtin','-nostdlib','-fuse-ld=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21','-DDX_ABI_BAD='+str(bad),*(['-DALGO_ASM_ABI'] if bad else []),LAB/'thumb.c',ROOT/'assessor/algorithms/thumb.S',ROOT/'labs/algorithms/tests/abi_thumb.S','-T'+str(LAB/'thumb.ld'),'-o',elf]);run('thumb-undefined-'+str(bad),['nm','-u',elf]);check('thumb-ABI-'+str(bad),('clobber-detected-result1' if bad else 'preserved-result1') in run('thumb-'+str(bad),['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',elf]))
  framac='/home/frede/cmastery-formal-tools/bundle/bin/frama-c';run('framac-version',[framac,'-version'])
  for weak in (0,1):
   text=run('eva-'+str(weak),[framac,'-eva','-eva-slevel','100','-cpp-extra-args=-DDX_WEAK='+str(weak),LAB/'static.c']);check('eva-premise-'+str(weak),('0 alarms generated' in text) if not weak else ('out of bounds' in text or 'out-of-bounds' in text or 'invalid memory access' in text))
  stub=compile('gcc','stub',[LAB/'tests.c',LAB/'diagnosis.c',*core[2:]],['-O2']);check('stub-incomplete','DX_CHECK:' in run('stub',[stub],1))
  check('source-stable',before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()})
 except Exception as e:passed=False;error=repr(e)
 else:passed=all(c['passed'] for c in checks);error=None
 artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(out.rglob('*')) if p.is_file()};result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':passed,'error':error,'commands':commands,'checks':checks,'observations':obs,'runtime_environment':{k:env[k] for k in ('ASAN_OPTIONS','UBSAN_OPTIONS','TSAN_OPTIONS')},'inspected_paths':before,'artifact_sha256':artifacts,'open_gates':['Independent review not run.','Minimum-seven-day independent unaided hidden diagnosis not observed.','Physical MCU/MMIO/cache/DMA observations not run;captured models and QEMU are separate.']};dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'commands':len(commands),'checks':len(checks),'error':error}));return 0 if passed else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
