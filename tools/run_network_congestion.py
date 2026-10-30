"""Source-bound Reno, independent byte oracles, packet schedules and target closure."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/congestion';REF=ROOT/'assessor/network_followthrough/congestion'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'ip':'assessor/network_followthrough/ip/ip.c','systems':'assessor/b045_systems/systems.c','runtime':'assessor/execution/support.c'}
MUTANTS={1:'nr_usable(&s) == 50',2:'s.ssthresh == 200 && s.cwnd == 500 && s.recovery',3:'nr_ack(&s, s.una, 401, 0, 16, 33) == NR_WAIT && s.duplicates == 0',4:'s.cwnd == 150 && s.una == 34',5:'s.cwnd == 600',6:'nr_timeout(&s, 3000) == NR_OK && s.ssthresh == 400',7:'nr_restart(&s, 1062, 1000) == NR_OK && s.cwnd == 100',8:'s.ssthresh == 400 && s.cwnd == 700',9:'s.cwnd == 600 && !s.quota'}
CORE='NR_PASS flight_window_ACK_division_fast_recovery_timeout_idle_limited'
ORACLE='NR_ORACLE_PASS cases=8255 ACK_partitions=4095 thresholds=2048 CA_partitions=672 limited_transmit=1440'
SIM='NR_SIM_ANALYSIS_PASS twelve_schedules seventeen_flows label_swap delayed_ACK loss_recovery'
STUB='nr_init(&s, 100, 0xfffffff0, 50) == NR_OK'
EXTRA=['labs/network_followthrough/src/congestion.c','labs/network_followthrough/ip/ip.h','labs/network_followthrough/ip/checksum_slice.py','labs/b045_systems/include/systems.h','labs/network_followthrough/family.json','labs/network_followthrough/CMakeLists.txt','curriculum/v5/exercises/X-T41-003.md','research/v5/expansion/legacy_network_congestion/resources.json','research/v5/expansion/legacy_network_congestion/taxonomy_extensions.json','build/network-tcp-sources/rfc5681.txt']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*EXTRA]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant=0,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib','--source',ROOT/'labs/network_followthrough/src/congestion.c' if stub else REF/'congestion.c','--driver',REF/'target.c','--runtime',ROOT/PROVIDERS['runtime']]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-congestion'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'];binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);includes=['-I'+str(ROOT/p) for p in ['labs/network_followthrough/congestion','labs/network_followthrough/ip','labs/b045_systems/include']]
 run('actual checksum slice',[sys.executable,ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/PROVIDERS['systems'],w/'checksum.c'],marker='Exactly two unchanged original checksum definitions')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  def build(name,test,mutant=0,stub=False):
   exe=w/(tag+'-'+name);src=ROOT/'labs/network_followthrough/src/congestion.c' if stub else REF/'congestion.c';testfile=LAB/'tests.c' if test=='core' else REF/(test+'.c');extra=[ROOT/PROVIDERS['ip'],w/'checksum.c'] if test=='simulator' else [];ok,_=run(tag+' '+name+' compile',[cc,*flags,'-DNR_MUTANT='+str(mutant),src,testfile,*extra,'-o',exe]);return ok,exe
  ok,exe=build('core','core')
  if ok:run(tag+' core execute',[exe],marker=CORE)
  ok,exe=build('probe','probe')
  if ok:
   run(tag+' independent oracle',[sys.executable,REF/'oracle.py',exe,'--trace',w/(tag+'-oracle.json')],marker=ORACLE)
   run(tag+' state layout',[exe,'--layout'],marker='NR_LAYOUT state=')
  ok,exe=build('simulator','simulator')
  if ok:run(tag+' packet schedules',[sys.executable,REF/'simulation_analysis.py',exe,'--trace',w/(tag+'-schedules.json')],marker=SIM)
  for v,marker in MUTANTS.items():
   ok,exe=build('mutant'+str(v),'core',v)
   if ok:run(tag+' mutant'+str(v)+' execute',[exe],1,marker)
  ok,exe=build('stub','core',stub=True)
  if ok:run(tag+' stub execute',[exe],1,STUB)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,cc,opt),marker='NR_TARGET_BUILD_PASS explicit_objects=5 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NR_TARGET_PASS actual_Reno_core CPU_state_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing controller rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in range(1,5)],'-o',dest/'missing-controller.elf'],1,'nr_init')
 for v in [*MUTANTS,'stub']:
  dest=w/('target-'+str(v));ok,_=run('target '+str(v)+' build',target_args(dest,'arm-none-eabi-gcc','0',v if isinstance(v,int) else 0,v=='stub'),marker='NR_TARGET_BUILD_PASS explicit_objects=5 original_runtime')
  if ok:run('target '+str(v)+' execute',emulator(dest/'image.elf'),1,STUB if v=='stub' else MUTANTS[v])
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author reference execution; external review and independent/delayed learner evidence absent.','Actual NiParse/struct s_checksum used by finite original packet model; no full bidirectional TCP or driver claim.','8255 aggregate oracle cases/profile and12 finite schedules are bounded, not universal correctness.','Strict Cortex-M4 core CPU execution is not physical Ethernet or latency observation.','No NewReno/SACK/ECN; conservative1MSS IW and fixed simulator RTO/backoff are explicit model assumptions.','Finite100ms service fairness cannot establish Internet fairness; completed equal workloads trivially give1.','Stack bound uses emitted direct-call frames plus1024 allowance, not measured physical high water.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
