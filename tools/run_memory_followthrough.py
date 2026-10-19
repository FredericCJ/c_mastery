"""Distinct original T45 mechanisms, bounded author observations, immutable attempts."""
import argparse,hashlib,json,os,subprocess,sys,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/memory_followthrough';REF=ROOT/'assessor/memory_followthrough';BASE=ROOT/'reviews/v5/expansion/legacy_memory_followthrough'
RETAINED=['labs/foundations/include/foundation.h','labs/foundations/src/foundation.c','assessor/foundations/foundation.c','labs/performance_followthrough/stack.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted({p for p in [*LAB.rglob('*'),*REF.rglob('*'),*[ROOT/p for p in RETAINED],*(ROOT/'fixtures/memory_lab').glob('PR-T45-*.json'),Path(__file__)] if p.is_file() and '__pycache__' not in p.parts})
def main(report):
 dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve()) and not dest.exists();out=dest.with_suffix('');out.mkdir(parents=True,exist_ok=False);before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};commands=[];checks=[];obs={}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:exitcode=86:allow_addr2line=1:external_symbolizer_path=/usr/bin/addr2line',UBSAN_OPTIONS='halt_on_error=1:exitcode=86')
 def run(label,cmd,code=0,timeout=60):
  cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True,timeout=timeout);log=out/(label+'.txt');log.write_text(p.stdout+p.stderr);commands.append({'label':label,'command':cmd,'cwd':str(ROOT),'exit_code':p.returncode,'expected_exit_code':code,'seconds':time.monotonic()-start,'output':log.relative_to(ROOT).as_posix()});assert code is None or p.returncode==code,(label,p.returncode,p.stdout,p.stderr);return p.stdout+p.stderr
 def check(name,value):checks.append({'name':name,'passed':bool(value)});assert value,name
 includes=['-I'+str(LAB),'-I'+str(ROOT/'labs/foundations/include')];core=[LAB/'tests.c',REF/'memory.c',ROOT/'assessor/foundations/foundation.c']
 def compile(cc,name,files,flags=(),strict=True,standard='c17'):
  exe=out/name;run(name+'-build',[cc,'-std='+standard,'-Wall','-Wextra',*(['-Werror','-pedantic'] if strict else []),*includes,*flags,*files,'-o',exe]);return exe
 try:
  for tool in ('gcc','clang','gdb','valgrind','qemu-system-arm'):run(tool+'-version',[tool,'--version'])
  run('kernel',['uname','-a']);obs['certificates']=json.loads(run('certificates',[sys.executable,LAB/'certificates.py']));check('ten-premise-certificates',obs['certificates']['passed'] and len(obs['certificates']['records'])==10)
  for cc in ('gcc','clang'):
   for profile,flags in [('O0',['-O0','-g','-fno-omit-frame-pointer']),('O2',['-O2','-g','-DNDEBUG','-fomit-frame-pointer'])]:
    prefix=cc+'-'+profile;binary=compile(cc,prefix+'-core',core,flags);check(prefix+'-core','MW_PASS:core' in run(prefix+'-core',[binary]));resources=compile(cc,prefix+'-resources',[LAB/'resources.c',ROOT/'assessor/foundations/foundation.c'],flags);check(prefix+'-resources','800-prefix-cycles' in run(prefix+'-resources',[resources]));stack=compile(cc,prefix+'-stack',[LAB/'stack.c',LAB/'stack.S'],flags,False);check(prefix+'-pattern','changed24:reserved64' in run(prefix+'-pattern',[stack]));obs[prefix+'-bounded']=json.loads(run(prefix+'-bounded',[sys.executable,LAB/'bounded.py',stack]));check(prefix+'-bounded',obs[prefix+'-bounded']['passed'])
    for specimen in ('provider','work','original'):
     obj=out/(prefix+'-'+specimen+'.o');source=ROOT/'labs/performance_followthrough/stack.c' if specimen=='provider' else LAB/'stack.c' if specimen=='original' else REF/'memory.c';run(prefix+'-'+specimen+'-stack-usage',[cc,'-std=c17',*flags,*includes,'-fstack-usage','-c',source,'-o',obj]);su=obj.with_suffix('.su');check(prefix+'-'+specimen+'-stack-record',su.exists());run(prefix+'-'+specimen+'-assembly',['objdump','-dr',obj]);obs[prefix+'-'+specimen+'-su']=su.read_text()
   sanitized=compile(cc,cc+'-core-sanitized',core,['-O1','-g','-fsanitize=address,undefined','-no-pie']);check(cc+'-core-sanitized','MW_PASS:core' in run(cc+'-core-sanitized',[sanitized]))
   obj=out/(cc+'-memory.o');foundation=out/(cc+'-foundation.o');helper=out/(cc+'-bytes.o');closed=out/(cc+'-closed.o')
   for name,source,target in [('memory',REF/'memory.c',obj),('foundation',ROOT/'assessor/foundations/foundation.c',foundation),('bytes',REF/'byte_support.c',helper)]:run(cc+'-'+name+'-object',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin',*includes,'-c',source,'-o',target])
   run(cc+'-core-close',[cc,'-nostdlib','-r',obj,foundation,helper,'-o',closed]);check(cc+'-runtime-closure',not run(cc+'-undefined',['nm','-u',closed]).strip())
   header=run(cc+'-ckd-header-path',[cc,'-print-file-name=include/stdckdint.h']).strip();run(cc+'-ckd-header-read',[sys.executable,'-c','from pathlib import Path;import sys;print(Path(sys.argv[1]).read_text())',header]);ckd=compile(cc,cc+'-checked23',[LAB/'checked23.c',*core[1:]],['-O2'],standard='c23');check(cc+'-C23-comparison','cases98:temporary-preserves' in run(cc+'-checked23',[ckd]))
   for fixed in (0,1):
    for profile in ('O0','O2'):
     for instrument in ('plain','asan','ubsan'):
      name=f'{cc}-{profile}-{instrument}-{fixed}';flags=['-'+profile,'-g','-DMW_FIXED='+str(fixed),*(['-fsanitize=address','-no-pie'] if instrument=='asan' else ['-fsanitize=undefined'] if instrument=='ubsan' else [])];binary=compile(cc,name,[LAB/'observer.c',LAB/'dependency.c'],flags,False)
      modes=('extent','lower','nul','pool','init','free','lifetime','canary') if instrument=='asan' else ('signed',) if instrument=='ubsan' else ('init','leak')
      for mode in modes:
       label=name+'-'+mode
       if instrument=='plain':code=0 if fixed else 89;command=['valgrind','--tool=memcheck','--track-origins=yes','--leak-check=full','--show-leak-kinds=all','--errors-for-leak-kinds=all','--error-exitcode=89',binary,mode]
       else:code=86 if not fixed and mode not in ('pool','init','canary') else 0;command=[binary,mode]
       raw=run(label,command,code);check(label+'-entered','ENTRY' in raw and 'SETUP:allocation8' in raw)
       if code==86:check(label+'-observer',('signed integer overflow' if instrument=='ubsan' else 'AddressSanitizer') in raw)
       elif instrument=='plain':check(label+'-observer',('ERROR SUMMARY: 0 errors' if fixed else 'Conditional jump or move depends on uninitialised' if mode=='init' else 'LEAK SUMMARY') in raw)
       elif mode=='pool':check(label+'-semantic',('DENIED:logical-extent' if fixed else 'SEMANTIC:logical-extent-violated') in raw)
       else:check(label+'-completed','EXIT:bounded-trial' in raw)
   # Identical instrumented caller, actual dependency coverage control.
   for coverage in ('plain','asan'):
    dep=out/(cc+'-dependency-'+coverage+'.o');run(cc+'-dependency-'+coverage+'-build',[cc,'-std=c17','-O0',*(['-g','-fsanitize=address'] if coverage=='asan' else []),'-c',LAB/'dependency.c','-o',dep]);exe=compile(cc,cc+'-coverage-'+coverage,[LAB/'observer.c',dep],['-O0','-g','-fsanitize=address','-no-pie'],False);raw=run(cc+'-coverage-'+coverage,[exe,'extent'],86 if coverage=='asan' else 0);check(cc+'-coverage-'+coverage,('heap-buffer-overflow' if coverage=='asan' else 'EXIT:bounded-trial') in raw and 'ENTRY' in raw)
   plain_dep=out/(cc+'-dependency-plain.o');uninstrumented=compile(cc,cc+'-memcheck-uninstrumented',[LAB/'observer.c',plain_dep],['-O0','-g'],False);check(cc+'-memcheck-binary-coverage','Conditional jump or move depends on uninitialised' in run(cc+'-memcheck-uninstrumented',['valgrind','--track-origins=yes','--error-exitcode=89',uninstrumented,'init'],89))
  check('gcc-qualifiers','dynamic,bounded' in obs['gcc-O0-provider-su'] and 'dynamic' in obs['gcc-O2-provider-su'] and 'static' in obs['gcc-O2-provider-su'])
  for bad in (0,1):
   exe=compile('gcc','hidden-'+str(bad),[REF/'hidden.c',*core[1:]],['-O2','-DMW_BAD='+str(bad)]);check('hidden-'+str(bad),('bad-caller-reclaims' if bad else 'fixed-context-held') in run('hidden-'+str(bad),[exe]))
  watch=compile('gcc','watch',[LAB/'watch.c'],['-O0','-g'],False);run('watch-map-before',['nm','-S',watch]);raw=run('watch-gdb',['gdb','-q','-batch','-x',LAB/'gdb.py','--args',watch]);obs['gdb']=json.loads(raw.split('MW_GDB:',1)[1].splitlines()[0]);run('watch-map-after',['nm','-S',watch]);check('live-watch-rebound',' = 99' in raw and ' = 82' in raw and 'hw watchpoint' in raw)
  for kind,extra in [('good',[]),('ram-overflow',['-DMW_BSS=900']),('flash-overflow',[])]:
   script=LAB/'image.ld'
   if kind=='flash-overflow':script=out/'tiny-flash.ld';script.write_text((LAB/'image.ld').read_text().replace('LENGTH = 4096','LENGTH = 64'))
   elf=out/(kind+'.elf');raw=run('image-'+kind+'-link',['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-Os','-g','-ffreestanding','-fno-builtin','-nostdlib','-fuse-ld=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21',*extra,LAB/'image.c','-T'+str(script),'-Wl,-Map='+str(out/(kind+'.map')),'-o',elf],0 if kind=='good' else None)
   if kind=='good':check('image-closure',not run('image-undefined',['nm','-u',elf]).strip());run('image-sections',['objdump','-h',elf]);check('QEMU-startup','MW:QEMU:copy96:zero400:retained64:two-initializations' in run('image-qemu',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',elf]))
   else:check('image-'+kind+'-rejected',commands[-1]['exit_code']!=0 and ('overflow' in raw or 'will not fit' in raw))
  stub=compile('gcc','stub',[LAB/'tests.c',LAB/'memory.c',ROOT/'labs/foundations/src/foundation.c'],['-O2']);check('stub-incomplete','MW_CHECK:' in run('stub',[stub],1));check('source-stable',before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()})
 except Exception as e:passed=False;error=repr(e)
 else:passed=all(r['passed'] for r in checks);error=None
 artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(out.rglob('*')) if p.is_file()};result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':passed,'error':error,'commands':commands,'checks':checks,'observations':obs,'environment':{k:env[k] for k in ('ASAN_OPTIONS','UBSAN_OPTIONS')},'inspected_paths':before,'artifact_sha256':artifacts,'open_gates':['Independent external review not run.','Minimum-seven-day independent unseen learner repair not observed.','Physical reset/DMA/cache/IRQ/stack observations not run;QEMU and models separate.','Exact published C17/C23 text not obtained;optional C23 comparison is explicitly N3096 draft plus installed headers.']};dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'commands':len(commands),'checks':len(checks),'error':error}));return 0 if passed else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
