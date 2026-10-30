"""Actual original checksum/IP/fragment implementations with explicit target closure."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/ip';REF=ROOT/'assessor/network_followthrough/ip'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'systems':'assessor/b045_systems/systems.c','http':'assessor/communications/http.c','net':'assessor/communications/net.c','runtime':'assessor/execution/support.c','strings':'assessor/network_followthrough/scope/runtime.c'}
MUTANTS={1:('core','ni_parse(&config, input, 32'),2:('core','ni_parse(&config, input, 100'),3:('core','out.bytes[24] == 0x12'),4:('core','index == 1'),5:('core','== NI_CONFLICT'),6:('core','ni_tick(&s, 60100)'),7:('core','ni_receive(&s, p, 28, 101'),8:('composition','count == 1')}
MARKERS={'core':'NI_PASS','validation':'NI_VALIDATION_PASS rejected=314 unaligned=520 checksum_collision_observed','composition':'NI_COMPOSITION_PASS actual_net_checksum_Echo_reassembly_timeout'}
HEADERS=['labs/communications/include/com_net.h','labs/communications/include/com_http.h','labs/b045_systems/include/systems.h','labs/network_followthrough/include/systems.h','labs/network_followthrough/src/checksum.c','labs/network_followthrough/src/ip.c','labs/network_followthrough/CMakeLists.txt','labs/network_followthrough/family.json','tools/check_network_ip_capture.py','fixtures/networking/ip_physical_capture.json','curriculum/guides/network-ip-board-protocol.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*HEADERS]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant=0,stub=False,checksum=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib',*[x for k,v in PROVIDERS.items() for x in ['--'+k,ROOT/v]],*[x for k,v in [('source',(LAB if stub else REF)/'ip.c'),('validation',REF/'validation.c'),('composition',REF/'composition.c'),('driver',REF/'target.c')] for x in ['--'+k,v]],*(['--checksum-mutant'] if checksum else [])]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-ip-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or ''.join(marker.split()) in ''.join(t.split())) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'];binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);includes=['-I'+str(ROOT/p) for p in ['labs/network_followthrough/ip','labs/b045_systems/include','labs/communications/include']]
 run('actual checksum slice',[sys.executable,LAB/'checksum_slice.py',ROOT/PROVIDERS['systems'],w/'checksum.c'],marker='Exactly two unchanged original checksum definitions')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  def build(label,test,mutant=0,stub=False,checksum=False):
   exe=w/(tag+'-'+label);extra=[ROOT/PROVIDERS[k] for k in ['http','net']] if test=='composition' else []
   testfile=LAB/'tests.c' if test=='core' else REF/(test+'.c')
   ok,_=run(tag+' '+label+' compile',[cc,*flags,'-DNI_MUTANT='+str(mutant),*(['-DS_MUTATE_CHECKSUM'] if checksum else []),(LAB if stub else REF)/'ip.c',w/'checksum.c',testfile,*extra,'-o',exe]);return ok,exe
  for case in MARKERS:
   ok,exe=build(case,case)
   if ok:run(tag+' '+case+' execute',[exe],marker=MARKERS[case])
  for mutant,(case,marker) in MUTANTS.items():
   ok,exe=build('mutant'+str(mutant),case,mutant)
   if ok:run(tag+' mutant'+str(mutant)+' execute',[exe],1,marker)
  for name,stub,checksum in [('stub',True,False),('checksum-mutant',False,True)]:
   ok,exe=build(name,'core',stub=stub,checksum=checksum)
   if ok:run(tag+' '+name+' execute',[exe],1,'ni_parse' if stub else 'actual == ni_test_sum')
  ok,exe=build('probe','probe')
  if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',exe,'--trace',w/(tag+'-oracle.trace')],marker='NI_ORACLE_PASS events=22751 seed=0x14105 permutations=304 random_histories=600')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,cc,opt),marker='NI_TARGET_BUILD_PASS explicit_objects=11 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NI_TARGET_PASS actual_IPv4_checksum_reassembly_ICMP synthetic_frames_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing checksum rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in range(11) if i!=5],'-o',dest/'missing-checksum.elf'],1,'s_checksum_add')
 for v in [*MUTANTS,'stub','checksum']:
  tag='target '+str(v);dest=w/('target-'+str(v));ok,_=run(tag+' build',target_args(dest,'arm-none-eabi-gcc','0',v if isinstance(v,int) else 0,v=='stub',v=='checksum'),marker='NI_TARGET_BUILD_PASS explicit_objects=11 original_runtime')
  if ok:run(tag+' execute',emulator(dest/'image.elf'),1,'ni_parse' if v=='stub' else 'actual == ni_test_sum' if v=='checksum' else MUTANTS[v][1])
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--systems',ROOT/PROVIDERS['systems'],'--checksum-slice','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'ip.c'])],int(stub),'NI_CHECK' if stub else 'NI_PASS')
 run('capture structural controls',[sys.executable,ROOT/'tools/check_network_ip_capture.py','--self-test'],marker='"physical_claim": false');run('unobserved capture rejection',[sys.executable,ROOT/'tools/check_network_ip_capture.py',ROOT/'fixtures/networking/ip_physical_capture.json'],1,'physical capture not observed');rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author reference execution only; no independent learner, external review or physical observation.','Actual unchanged struct s_checksum/net_checksum/net_receive/http providers, with real original ARP admission before differential Echo.','314 rejected packets,33411split checks,22751oracle events are finite; no universal correctness or full RFC compliance inferred.','MPS2 CPU packet arrays do not implement Ethernet hardware; actual T39 copied-port binding and physical capture remain required.','Strict/identical overlap and tombstone lifetime are explicit policies; original historical parser remains fragment-rejecting.','Declared unicast-only outer link admission and expire-before-receive required for feedback; original runtime closure depends on selected objects.','Source access, target memory and nonrecursive stack assumptions remain bounded and separately inspectable.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
