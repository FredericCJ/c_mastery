"""Current ARP field/lifetime/source/target evidence with exact compiled controls."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/arp';REF=ROOT/'assessor/network_followthrough/arp'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'scope':'assessor/network_followthrough/scope/scope.c','http':'assessor/communications/http.c','net':'assessor/communications/net.c','runtime':'assessor/execution/support.c','strings':'assessor/network_followthrough/scope/runtime.c'}
MUTANTS={1:('validation','rc == NA_INVALID'),2:('validation','rc == NA_INVALID'),3:('core','frame.bytes[5] == 7'),4:('core','ticket.kind == NA_REQUEST && !s.known'),5:('core','na_prepare(&s, 1000'),6:('core','s.requests == 0')}
MARKERS={'core':'NA_PASS retry_ms','validation':'NA_VALIDATION_PASS rejected=4888 accepted_controls=2','composition':'NA_COMPOSITION_PASS actual_HTTP_TCP_IP'}
HEADERS=['labs/communications/include/com_net.h','labs/communications/include/com_http.h','labs/network_followthrough/scope/scope.h','tools/check_network_arp_capture.py','fixtures/networking/arp_physical_capture.json','curriculum/guides/network-arp-board-protocol.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*HEADERS]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant=0,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib',*[x for k,v in PROVIDERS.items() for x in ['--'+k,ROOT/v]],*[x for k,v in [('source',(LAB if stub else REF)/'arp.c'),('validation',REF/'validation.c'),('composition',REF/'composition.c'),('driver',REF/'target.c')] for x in ['--'+k,v]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-arp-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'];binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);includes=['-I'+str(ROOT/p) for p in ['labs/network_followthrough/arp','labs/network_followthrough/scope','labs/communications/include']]
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  for case in ['core','validation','composition']:
   test=LAB/'tests.c' if case=='core' else REF/(case+'.c');extra=[ROOT/PROVIDERS[k] for k in ['scope','http','net']] if case=='composition' else [];exe=w/(tag+'-'+case);ok,_=run(tag+' '+case+' compile',[cc,*flags,REF/'arp.c',test,*extra,'-o',exe])
   if ok:run(tag+' '+case+' execute',[exe],marker=MARKERS[case])
  for mutant,(case,marker) in MUTANTS.items():
   test=LAB/'tests.c' if case=='core' else REF/'validation.c';exe=w/(tag+'-mutant'+str(mutant));ok,_=run(tag+' mutant'+str(mutant)+' compile',[cc,*flags,'-DNA_MUTANT='+str(mutant),REF/'arp.c',test,'-o',exe])
   if ok:run(tag+' mutant'+str(mutant)+' execute',[exe],1,marker)
  exe=w/(tag+'-stub');ok,_=run(tag+' stub compile',[cc,*flags,LAB/'arp.c',LAB/'tests.c','-o',exe])
  if ok:run(tag+' stub execute',[exe],1,'NA_CHECK na_init')
  exe=w/(tag+'-probe');ok,_=run(tag+' probe compile',[cc,*flags,REF/'arp.c',REF/'probe.c','-o',exe])
  if ok:
   ok,log=run(tag+' probe execute',[exe])
   if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='NA_ORACLE_PASS groups=72 events=51024')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,cc,opt),marker='NA_TARGET_BUILD_PASS explicit_objects=11 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NA_TARGET_PASS actual_arp_fields_retry_HTTP_TCP_IP synthetic_frames_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing strings rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in [*range(9),10]],'-o',dest/'missing-strings.elf'],1,'strlen')
 for v in [1,2,3,4,5,6,'stub']:
  tag='target '+str(v);dest=w/('target-'+str(v));ok,_=run(tag+' build',target_args(dest,'arm-none-eabi-gcc','0',0 if v=='stub' else v,v=='stub'),marker='NA_TARGET_BUILD_PASS explicit_objects=11 original_runtime')
  if ok:run(tag+' execute',emulator(dest/'image.elf'),1,'NA_CHECK na_init' if v=='stub' else MUTANTS[v][1])
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'arp.c'])],int(stub),'NA_CHECK' if stub else 'NA_PASS')
 run('capture structural controls',[sys.executable,ROOT/'tools/check_network_arp_capture.py','--self-test'],marker='"physical_claim": false');run('unobserved capture rejection',[sys.executable,ROOT/'tools/check_network_arp_capture.py',ROOT/'fixtures/networking/arp_physical_capture.json'],1,'physical capture not observed');rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author reference execution only; no independent learner, external review or physical observation.','C17 with actual historical HTTP/TCP/IP and current struct nw_lengths; historical transport lower-admission gap remains separate.','4888 malformed/local-boundary rejects and72timer groups51024events are finite; no whole-input proof inferred.','MPS2 struct lr_cpu packet arrays do not implement Ethernet hardware; actual T39 copy adapter and physical capture remain distinct.','3000ms total lower-admission lifetime and30000ms cache expiry are explicit curriculum policy, not universal RFC826 timers.','Consistent forged local association demonstrates unauthenticated ARP; no network interaction or code-execution claim.','Original runtime closure and nonrecursive stack bound depend on actual selected objects and masked interrupts.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
