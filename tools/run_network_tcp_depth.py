"""Source-bound TCP state/ownership/precision campaign with actual providers."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/tcp';REF=ROOT/'assessor/network_followthrough/tcp'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'systems':'assessor/b045_systems/systems.c','ip':'assessor/network_followthrough/ip/ip.c','scope':'assessor/network_followthrough/scope/scope.c','stream':'assessor/network_followthrough/scope/stream.c','http':'assessor/communications/http.c','runtime':'assessor/execution/support.c','strings':'assessor/network_followthrough/scope/runtime.c'}
HEADERS=['labs/network_followthrough/ip/ip.h','labs/network_followthrough/ip/checksum_slice.py','labs/network_followthrough/scope/scope.h','labs/network_followthrough/scope/stream.h','labs/communications/include/com_http.h','labs/b045_systems/include/systems.h','curriculum/guides/network-tcp-board-protocol.md','fixtures/networking/tcp_physical_capture.json','tools/check_network_tcp_capture.py']
MUTANTS={1:('core','nc_rtt_sample(&r, 800, 1) == NC_WAIT'),2:('core','r.srtt == 85 && r.variance == 40'),3:('core','nc_u16(out.bytes + 34) == 492'),4:('core','!s->flight && s->snd_nxt == isn'),5:('core','nc_receive(s, p, n, 80) == NC_OK'),6:('core','nc_receive(&s, p, n, 81) == NC_WAIT'),7:('validation','s.peer_window == 40'),8:('core','s.rcv_nxt == 35 && s.rx.count == 4')}
MARKERS={'core':'NC_PASS wrap_overlap_capacity_acceptance_RTO_Karn_reset','validation':'NC_VALIDATION_PASS closes_timers_WL_wrap_104_rejections','composition':'NC_COMPOSITION_PASS actual_Ni_SChecksum_NwRx_HTTP two_policies_16_4'}
ORACLE='NC_ORACLE_PASS rtt_events=12288 rx_events=24576 seed=0x141067'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*HEADERS]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant=0,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib',*[x for k,v in PROVIDERS.items() for x in ['--'+k,ROOT/v]],*[x for k,v in [('source',(LAB if stub else REF)/'tcp.c'),('validation',REF/'validation.c'),('composition',REF/'composition.c'),('driver',REF/'target.c')] for x in ['--'+k,v]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-tcp-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'];binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);includes=['-I'+str(ROOT/p) for p in ['labs/network_followthrough/tcp','labs/network_followthrough/ip','labs/network_followthrough/scope','labs/b045_systems/include','labs/communications/include']]
 run('actual checksum slice',[sys.executable,ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/PROVIDERS['systems'],w/'checksum.c'],marker='Exactly two unchanged original checksum definitions')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  def build(label,test,mutant=0,stub=False):
   exe=w/(tag+'-'+label);testfile=LAB/'tests.c' if test=='core' else REF/(test+'.c');ok,_=run(tag+' '+label+' compile',[cc,*flags,'-DNC_MUTANT='+str(mutant),(LAB if stub else REF)/'tcp.c',w/'checksum.c',testfile,*[ROOT/PROVIDERS[k] for k in ['ip','scope','stream','http']],'-o',exe]);return ok,exe
  for case in MARKERS:
   ok,exe=build(case,case)
   if ok:run(tag+' '+case+' execute',[exe],marker=MARKERS[case])
  for mutant,(case,marker) in MUTANTS.items():
   ok,exe=build('mutant'+str(mutant),case,mutant)
   if ok:run(tag+' mutant'+str(mutant)+' execute',[exe],1,marker)
  ok,exe=build('stub','core',stub=True)
  if ok:run(tag+' stub execute',[exe],1,'nc_rtt_init(&r, 1, 0) == NC_OK')
  ok,exe=build('probe','probe')
  if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',exe,'--trace',w/(tag+'-oracle.trace')],marker=ORACLE)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,cc,opt),marker='NC_TARGET_BUILD_PASS explicit_objects=13 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NC_TARGET_PASS actual_TCP_queue_RTO_HTTP synthetic_frames_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing checksum rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in range(13) if i!=5],'-o',dest/'missing-checksum.elf'],1,'s_checksum_add')
 for v in [*MUTANTS,'stub']:
  dest=w/('target-'+str(v));ok,_=run('target '+str(v)+' build',target_args(dest,'arm-none-eabi-gcc','0',v if isinstance(v,int) else 0,v=='stub'),marker='NC_TARGET_BUILD_PASS explicit_objects=13 original_runtime')
  if ok:run('target '+str(v)+' execute',emulator(dest/'image.elf'),1,'nc_rtt_init(&r, 1, 0) == NC_OK' if v=='stub' else MUTANTS[v][1])
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*[x for k in ['systems','ip','scope','stream','http'] for x in ['--'+k,ROOT/PROVIDERS[k]]],*([] if stub else ['--source',REF/'tcp.c'])],int(stub),'NC_CHECK' if stub else 'NC_PASS')
 run('capture structural controls',[sys.executable,ROOT/'tools/check_network_tcp_capture.py','--self-test'],marker='"physical_claim": false');run('unobserved capture rejection',[sys.executable,ROOT/'tools/check_network_tcp_capture.py',ROOT/'fixtures/networking/tcp_physical_capture.json'],1,'physical capture not observed');rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author reference execution only; no independent learner, external review or physical observation.','Actual unchanged NiParse/struct s_checksum/struct nw_rx/NwHttpPump/http providers; explicit copied admission remains a T39 adapter obligation.','12288 rational and24576 interval transitions per profile are finite; no universal correctness or full RFC conformance inferred.','MPS2 executes CPU packet arrays, not Ethernet hardware or measured wire latency.','Fixed one-flight policy does not implement required congestion growth/fast recovery; separate mandatory investigation remains open.','The WL2 internal transition fixture is explicitly injected; scope validation and external serialization remain assumptions.','Maximum direct nonrecursive stack path and1024 allowance is an author bound, not physical high-water measurement.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
