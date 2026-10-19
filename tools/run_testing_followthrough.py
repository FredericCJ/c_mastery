"""Source-bound local author campaign for exact remaining T32 mechanisms."""
import argparse,hashlib,json,os,re,subprocess,sys,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/testing_followthrough';REF=ROOT/'assessor/testing_followthrough';BASE=ROOT/'reviews/v5/expansion/legacy_testing_followthrough'
PROFILES=[(cc+'-'+p,cc,f) for cc in ('gcc','clang') for p,f in [('debug',['-O0','-g']),('release',['-O3','-g']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return [Path(__file__).resolve(),ROOT/'labs/testing_depth/models/schedule.c',*[p for folder in (LAB,REF) for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main(report):
 BASE.mkdir(parents=True,exist_ok=True);dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve())
 work=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
 v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'commands':[],'checks':[],'open_gates':[]}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:exitcode=86:allow_addr2line=1:external_symbolizer_path=/usr/bin/addr2line',UBSAN_OPTIONS='halt_on_error=1:exitcode=86',TSAN_OPTIONS='halt_on_error=1:exitcode=86:external_symbolizer_path=/usr/bin/addr2line')
 v['runtime_environment']={k:env[k] for k in ('ASAN_OPTIONS','UBSAN_OPTIONS','TSAN_OPTIONS')}
 def run(cmd,label,code=0,marker=None,cwd=None,timeout=60):
  cmd=list(map(str,cmd));start=time.perf_counter();r=subprocess.run(cmd,capture_output=True,text=True,env=env,cwd=cwd or ROOT,timeout=timeout);out=r.stdout+r.stderr
  p=work/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n')
  row={'label':label,'command':cmd,'cwd':str(cwd or ROOT),'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix(),'elapsed_seconds':time.perf_counter()-start,'expected_exit':code,'required_marker':marker};v['commands'].append(row)
  if code is not None and r.returncode!=code:raise ValueError(label+': unexpected exit '+str(r.returncode)+'\n'+out[-4000:])
  if marker and marker not in out:raise ValueError(label+': missing '+marker+'\n'+out[-4000:])
  return out,row
 def check(name,condition,detail=None):
  v['checks'].append({'name':name,'passed':bool(condition),'detail':detail})
  if not condition:raise ValueError(name)
 def compile(cc,name,files,flags=(),strict=True):
  exe=work/name;cmd=[cc,'-std=c17','-Wall','-Wextra',*(['-Werror','-pedantic'] if strict else []),*flags,'-I'+str(LAB),*files,'-o',exe];run(cmd,name+'-build');return exe
 try:
  for tool in ('gcc','clang','gcov','valgrind','gdb','qemu-system-arm','/usr/bin/time'):run([tool,'--version'],tool.split('/')[-1]+'-version')
  replay={'schema_version':1,'generator':'literal-endian-v1','seed':0,'environment':{'length':2,'live_owner':1,'wire_order':'big-endian'},'cases':[]}
  for name,data in [('original',bytes.fromhex('1234')),('minimal',bytes.fromhex('0001')),('equal-control',bytes.fromhex('0000')),('packet',bytes.fromhex('01021234abcd'))]:
   p=work/(name+'.bin');p.write_bytes(data);replay['cases'].append({'name':name,'path':p.relative_to(ROOT).as_posix(),'hex':data.hex(),'sha256':sha(p)})
  (work/'replay.json').write_text(json.dumps(replay,indent=2)+'\n');v['replay']=replay
  # Exact semantic controls: invalid compilation/sanitizer failure cannot kill mutants.
  for profile,cc,flags in PROFILES:
   for kind,count,prefix in [('transaction',3,'TX'),('coverage',4,'CV')]:
    for variant in [0,*range(1,count+1),'stub']:
     name=profile+'-'+kind+'-'+str(variant);source=(LAB if variant=='stub' else REF)/(kind+'.c')
     exe=compile(cc,name,[source,LAB/(kind+'_tests.c')],[*flags,'-D'+prefix+'_MUTANT='+str(0 if variant=='stub' else variant)])
     out,_=run([exe],name,0 if variant==0 else 1,prefix+('_PASS:' if variant==0 else '_CHECK:'))
     check(name+'-semantic',not any(s in out for s in ('AddressSanitizer','runtime error:','Segmentation fault')))
     if kind=='coverage' and variant in (0,1,3,4):run([exe,'weak'],name+'-weak',0,'CV_WEAK_PASS:')
   exe=compile(cc,profile+'-ipc',[LAB/'process_adapter.c'],flags);run([exe],profile+'-ipc',0,'IPC_PASS:')
  for cc in ('gcc','clang'):
   for kind in ('transaction','coverage'):
    obj=work/(cc+'-'+kind+'-core.o');run([cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-I'+str(LAB),'-c',REF/(kind+'.c'),'-o',obj],cc+'-'+kind+'-core-build')
    out,_=run(['nm','-u',obj],cc+'-'+kind+'-undefined');check(cc+'-'+kind+'-dependency-profile',not out.strip())
  # Individual native memory mechanisms and repaired controls, never ASan under Memcheck.
  for cc in ('gcc','clang'):
   for repaired in (0,1):
    for instrument,flags in [('plain',['-O0','-g']),('asan',['-O1','-g','-fsanitize=address','-fno-omit-frame-pointer','-no-pie']),('ubsan',['-O1','-g','-fsanitize=undefined','-fno-omit-frame-pointer'])]:
     name=f'{cc}-{instrument}-diag-{repaired}';exe=compile(cc,name,[LAB/'diagnostic.c'],[*flags,'-DDIAG_REPAIRED='+str(repaired)],False)
     modes=(0,1,2,3) if repaired else (0,1,2) if instrument in ('plain','asan') else (3,)
     for mode in modes:
      label=name+'-'+str(mode)
      if instrument=='plain':
       out,row=run(['valgrind','--tool=memcheck','--track-origins=yes','--leak-check=full','--error-exitcode=89',exe,str(mode)],label,0 if repaired else 89,'ERROR SUMMARY: 0 errors' if repaired else ['Invalid write','Invalid read','Conditional jump or move depends on uninitialised'][mode])
      elif instrument=='asan':
       expected=86 if not repaired and mode in (0,1) else 0;marker=['heap-buffer-overflow','heap-use-after-free'][mode] if expected else 'DIAG_TERMINATED';out,row=run([exe,str(mode)],label,expected,marker)
      else:out,row=run([exe,str(mode)],label,0 if repaired else 86,'DIAG_TERMINATED' if repaired else 'signed integer overflow')
      check(label+'-mechanism',True,{'profile':instrument,'repaired':repaired,'mode':mode,'diagnostic_scope':'selected observed access only'})
    name=f'{cc}-tsan-{repaired}';exe=compile(cc,name,[LAB/'race.c'],['-O1','-g','-fsanitize=thread','-pthread','-DRACE_REPAIRED='+str(repaired)])
    out,row=run([exe],name,None)
    check(name+'-runtime',row['exit_code']==(0 if repaired else 86) and ('RACE_COMPLETED:20000' in out if repaired else 'WARNING: ThreadSanitizer: data race' in out),out[-1500:])
   # Fixed actual workload with process peak RSS and monotonic inner timing.
   for instrument,flags in [('plain',[]),('asan',['-fsanitize=address','-no-pie']),('ubsan',['-fsanitize=undefined'])]:
    name=cc+'-workload-'+instrument;exe=compile(cc,name,[LAB/'workload.c'],['-O2','-g',*flags]);run(['size',exe],name+'-size')
    samples=[]
    for k in range(3):
     out,row=run(['/usr/bin/time','-f','TIME_RSS:%e:%M',exe],name+'-'+str(k),0,'WORKLOAD:');m=re.search(r'WORKLOAD:(\d+):ns=(\d+):resolution_ns=(\d+)',out);rss=re.search(r'TIME_RSS:([\d.]+):(\d+)',out)
     expected=sum((i*17+r)%65521 for r in range(512) for i in range(4096));check(name+'-checksum-'+str(k),m is not None and int(m[1])==expected and rss is not None);samples.append({'inner_ns':int(m[2]),'resolution_ns':int(m[3]),'outer_seconds':float(rss[1]),'peak_rss_kib':int(rss[2])})
    v.setdefault('overhead',[]).append({'compiler':cc,'profile':instrument,'binary':exe.relative_to(ROOT).as_posix(),'samples':samples,'scope':'process RSS; outer time resolution0.01s, setup included; inner fixed workload monotonic time'})
  # Public packet runtime admission versus static contexts including invalid callers.
  for cc in ('gcc','clang'):
   obj=work/(cc+'-packet.o');run([cc,'-std=c17','-Wall','-Wextra','-Werror','-O1','-g','-Dmain=analysis_entry','-c',LAB/'packet_analysis.c','-o',obj],cc+'-packet-object')
   exe=compile(cc,cc+'-packet-runtime',[obj,LAB/'packet_runtime.c'],['-O1','-g','-fsanitize=address,undefined','-no-pie']);run([exe],cc+'-packet-runtime',0,'PACKET_PASS:')
  framac=os.environ.get('FRAMAC','/home/frede/cmastery-formal-tools/bundle/bin/frama-c');run([framac,'-version'],'framac-version')
  for case in (0,1,2,3):
   out,row=run([framac,'-eva','-eva-slevel','100','-cpp-extra-args=-DANALYSIS_CASE='+str(case),LAB/'packet_analysis.c'],'eva-'+str(case))
   check('eva-context-'+str(case),('0 alarms generated' in out) if case in (0,3) else ('out of bounds' in out or 'out-of-bounds' in out or 'accessing out of bounds' in out or 'invalid memory access' in out),out[-1500:])
  run(['gcc','-std=c17','-O0','-Wall','-Wextra','-fanalyzer','-DANALYSIS_CASE=1','-c',LAB/'packet_analysis.c','-o',work/'warning.o'],'warnings-packet')
  # Targeted MC/DC and distinct weak/strong runs with raw condition and branch maps.
  for variant in (0,4):
   folder=work/('coverage-'+str(variant));folder.mkdir();exe=folder/'cv';run(['gcc','-std=c17','-O0','-g','--coverage','-fcondition-coverage','-fprofile-abs-path','-DCV_MUTANT='+str(variant),'-I'+str(LAB),REF/'coverage.c',LAB/'coverage_tests.c','-o',exe],'condition-build-'+str(variant))
   run([exe,'weak'],'condition-weak-'+str(variant),0,'CV_WEAK_PASS:');run(['gcov','--conditions','--branch-probabilities','--branch-counts','cv-coverage.gcno'],'condition-weak-report-'+str(variant),cwd=folder)
   weak=(folder/'coverage.c.gcov').read_text();(folder/'weak.gcov').write_text(weak)
   run([exe],'condition-strong-'+str(variant),0 if not variant else 1,'CV_PASS:' if not variant else 'CV_CHECK:')
   out,_=run(['gcov','--conditions','--branch-probabilities','--branch-counts','cv-coverage.gcno'],'condition-strong-report-'+str(variant),cwd=folder)
   v.setdefault('coverage',[]).append({'mutant':variant,'weak':(folder/'weak.gcov').relative_to(ROOT).as_posix(),'strong':(folder/'coverage.c.gcov').relative_to(ROOT).as_posix()})
  out,_=run(['gcc','-std=c17','-O0','-fpath-coverage','-fpath-coverage-limit=1','-I'+str(LAB),'-c',REF/'coverage.c','-o',work/'pathlimit.o'],'prime-path-limit')
  check('prime-path-limit-diagnostic','paths' in out and ('limit' in out or 'exceeded' in out),out)
  # Exact debug symbols and semantic hypothesis discrimination, optimized registers too.
  for cc in ('gcc','clang'):
   for level in ('-O0','-O3'):
    for repair in (0,1):
     name=f'{cc}-debug-{level[1:]}-{repair}';exe=compile(cc,name,[LAB/'debug_component.c',LAB/'debug_driver.c'],[level,'-g','-DDEBUG_REPAIRED='+str(repair)])
     run([exe],name,0 if repair else 1,'DEBUG_VALUE:4660' if repair else 'DEBUG_CHECK:wire-big-endian')
     run([exe,'00','01'],name+'-minimal',0 if repair else 1,'DEBUG_VALUE:1' if repair else 'DEBUG_VALUE:256')
     run([exe,'00','00'],name+'-equal-control',0,'DEBUG_VALUE:0')
     for label,args in [('delete-first',['01','00','1']),('delete-second',['00','01','1']),('delete-both',['00','00','0'])]:run([exe,*args],name+'-'+label,0,'DEBUG_REJECT:extent')
     if not repair:
      run(['gdb','-q','-batch','-ex','set disable-randomization off','-ex','set pagination off','-ex','break debug_decode','-ex','run','-ex','info args','-ex','x/2bx p','-ex','print value','-ex','info registers rdi rsi rdx','-ex','disassemble debug_decode','-ex','continue',exe],name+'-gdb',0,'DEBUG_CHECK:wire-big-endian')
   for release in (0,1):
    for repair in (0,1):
     name=f'{cc}-releasecase-{release}-{repair}';exe=compile(cc,name,[LAB/'release_case.c'],['-O3' if release else '-O0','-g',*(['-DNDEBUG'] if release else []),'-DRELEASE_REPAIRED='+str(repair),'-Wno-unused-function'])
     run([exe],name,1 if release and not repair else 0,'RELEASE_CHECK:' if release and not repair else 'RELEASE_PASS:')
     run(['gdb','-q','-batch','-ex','set disable-randomization off','-ex','run',exe],name+'-gdb',0,'RELEASE_CHECK:' if release and not repair else 'RELEASE_PASS:')
     if release:
      instrumented=compile(cc,name+'-asan',[LAB/'release_case.c'],['-O3','-g','-DNDEBUG','-DRELEASE_REPAIRED='+str(repair),'-Wno-unused-function','-fsanitize=address,undefined','-no-pie'])
      run([instrumented],name+'-asan',1 if not repair else 0,'RELEASE_CHECK:' if not repair else 'RELEASE_PASS:')
  # Actual owned pthread schedule: logical lost update is defined atomic behavior.
  for repair in (0,1):
   exe=compile('gcc','schedule-'+str(repair),[ROOT/'labs/testing_depth/models/schedule.c'],['-O2','-g','-pthread','-DTM_SCHEDULE_REPAIR='+str(repair)])
   for schedule in ('aAbB','abAB','abBA','baAB','baBA','bBaA'):
    run([exe,schedule],'schedule-'+str(repair)+'-'+schedule,0,'SCHEDULE none 2' if repair or schedule in ('aAbB','bBaA') else 'SCHEDULE lost-update 1')
  # Same wire vector and architectural SVC on two identified emulator machines.
  exe=work/'guest.elf';run(['clang','--target=arm-none-eabi','-mcpu=cortex-m3','-mthumb','-std=c17','-O2','-g','-ffreestanding','-fno-builtin','-nostdlib','-fuse-ld=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21','-Wl,-T,'+str(LAB/'guest.ld'),LAB/'guest.c',LAB/'guest_startup.S','-o',exe],'guest-build')
  for machine in ('mps2-an385','lm3s6965evb'):run(['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',exe],machine,0,'GUEST_PASS:BE_SUM=48641:SVC=11',timeout=15)
  v['open_gates']=['Dedicated physical board transport, routing, baud/timebase calibration and ADC accuracy remain unobserved.','Author/reference execution does not establish independent review or delayed learner performance.']
  run([sys.executable,LAB/'models.py'],'independent-models',0,'MODELS_PASS:')
 except (OSError,ValueError,subprocess.SubprocessError,AssertionError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in v['inspected_paths'].items());v['passed']='error' not in v and v['source_stable']
 v['artifact_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()};dest.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n');(work/'campaign.json').write_bytes(dest.read_bytes());print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'checks':len(v['checks']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
