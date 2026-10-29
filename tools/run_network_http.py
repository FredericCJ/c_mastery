"""Actual T41.08 raw HTTP over actual driver campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/http';REF=ROOT/'assessor/network_followthrough/http'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
from run_network_driver import PROVIDERS as DRIVER_PROVIDERS,KEYS as DRIVER_KEYS,INCLUDES as DRIVER_INCLUDES,inputs as driver_inputs
PROVIDERS={**DRIVER_PROVIDERS,'network_driver':'assessor/network_followthrough/driver/driver.c','flow':'assessor/hosted_followthrough/http/flow.c'}
KEYS=[*DRIVER_KEYS,'network_driver','flow'];INCLUDES=[*DRIVER_INCLUDES,'labs/network_followthrough/http','labs/hosted_followthrough/http']
MUTANTS={1:'captured == z && !memcmp(capture, want, z)',2:'captured == z && !memcmp(capture, want, z)'}
MARKER='NH_PASS actual_driver_parser_encoder_raw_peer'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 return sorted(set(driver_inputs())|{Path(__file__),ROOT/PROVIDERS['flow'],ROOT/'labs/hosted_followthrough/http/flow.h'}|{p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts})
def target_args(dest,manifest,cc,opt,mutant=0,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--providers',manifest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib','--source',(LAB/'learner.c' if stub else REF/'http.c'),'--driver',REF/'target.c','--board',ROOT/PROVIDERS['board'],'--phy-port',ROOT/PROVIDERS['phy_port'],'--ring-port',ROOT/PROVIDERS['ring_port'],*[x for inc in INCLUDES for x in ['--include',ROOT/inc]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]

def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-http'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
  with log.open('w') as f:
   try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
   except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
  t=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in t) and (not empty or not t.strip()) and not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'];binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
 for t in tools:run(t+' version',[t,'--version'])
 run('environment',['uname','-a']);includes=['-I'+str(ROOT/p) for p in INCLUDES]
 run('actual checksum slice',[sys.executable,ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/PROVIDERS['systems'],w/'checksum.c'],marker='Exactly two unchanged original checksum definitions')
 manifest=w/'providers.json';manifest.write_text(json.dumps({k:str(ROOT/PROVIDERS[k]) for k in KEYS},indent=2)+'\n')
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
  for variant in [0,*MUTANTS,'stub']:
   exe=w/(tag+'-'+str(variant));source=LAB/'learner.c' if variant=='stub' else REF/'http.c';m=0 if variant=='stub' else variant
   ok,_=run(tag+' '+str(variant)+' compile',[cc,*flags,'-DNH_MUTANT='+str(m),source,LAB/'tests.c',*[w/'checksum.c' if k=='systems' else ROOT/PROVIDERS[k] for k in KEYS if k not in ['runtime','strings']],'-o',exe])
   if ok:run(tag+' '+str(variant)+' execute',[exe],int(variant!=0),MARKER if variant==0 else 'nh_init(&app, &net, \"mcu.test\", date ? date_value : 0, (void *)stamp, 0) == NH_OK' if variant=='stub' else MUTANTS[variant])
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,manifest,cc,opt),marker='NH_TARGET_BUILD_PASS explicit_objects=25 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NH_TARGET_PASS actual_ring_ARP_IP_TCP_HTTP CPU_state_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing encoder rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB.parent/'driver/target.ld',*[dest/(str(i)+'.o') for i in range(25) if i!=19],'-o',dest/'missing-encoder.elf'],1,'hf_response')
 for v in [*MUTANTS,'stub']:
  dest=w/('target-'+str(v));ok,_=run('target '+str(v)+' build',target_args(dest,manifest,'arm-none-eabi-gcc','0',v if isinstance(v,int) else 0,v=='stub'),marker='NH_TARGET_BUILD_PASS explicit_objects=25 original_runtime')
  if ok:run('target '+str(v)+' execute',emulator(dest/'image.elf'),1,'nh_init(&app, &net, \"mcu.test\", date ? date_value : 0, (void *)stamp, 0) == NH_OK' if v=='stub' else MUTANTS[v])
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),'--providers',manifest,*([] if stub else ['--source',REF/'http.c'])],int(stub),'NH_CHECK' if stub else 'NH_PASS')
 missing=w/'missing.json';missing.write_text('{}');run('missing explicit providers rejected',[sys.executable,LAB/'run.py','--build',w/'missing-provider','--providers',missing],2,'missing explicit providers')
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Actual original struct nb_net/struct en_ring/Na/Ni/Nc/http/hf_response providers; raw peer reconstructs sequence bytes and literal HTTP expectations independently.','32 finite raw cases per profile plus zero-window deadline, stale application and link/new-handshake replay controls; no full network interoperability claim.','One request per connection, one copied output up to1600bytes, parser body up to1024, receive capacity up to512; input budget64 per application step.','Date is omitted or supplied by a trusted UTC callback. Monotonic transport/deadline values are not UTC.','QEMU runs struct lr_cpu and selected register-routing models, not Ethernet electronics or physical capture.','Target generated-code/stack audit includes original helpers and actual linked closure; no library substitution.','Independent review, actual learner substitutions, delayed attempts and dedicated physical observations remain not-run.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
