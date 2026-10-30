"""Actual T39/T41 composed ownership and bounded board-port campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/driver';REF=ROOT/'assessor/network_followthrough/driver'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'tcp': 'assessor/network_followthrough/tcp/tcp.c', 'arp': 'assessor/network_followthrough/arp/arp.c', 'ip': 'assessor/network_followthrough/ip/ip.c', 'scope': 'assessor/network_followthrough/scope/scope.c', 'stream': 'assessor/network_followthrough/scope/stream.c', 'http': 'assessor/communications/http.c', 'systems': 'assessor/b045_systems/systems.c', 'ring': 'assessor/ethernet_followthrough/ring/ring.c', 'ring_model': 'labs/ethernet_followthrough/ring/model.c', 'phy': 'assessor/ethernet_followthrough/phy/phy.c', 'phy_model': 'labs/ethernet_followthrough/phy/model.c', 'ethernet': 'assessor/ethernet_depth/ethernet_depth.c', 'bringup': 'assessor/bringup_depth/bringup.c', 'runtime': 'assessor/execution/support.c', 'strings': 'assessor/network_followthrough/scope/runtime.c', 'board': 'assessor/network_followthrough/driver/board.c', 'phy_port': 'assessor/ethernet_followthrough/phy/target.c', 'ring_port': 'assessor/ethernet_followthrough/ring/target.c', 'variants': 'assessor/ethernet_followthrough/ring/variants.c', 'variant_tests': 'labs/ethernet_followthrough/ring/variants_tests.c', 'phy_tests':'labs/ethernet_followthrough/phy/tests.c'}
INCLUDES=['labs', 'labs/bringup_depth', 'labs/network_followthrough/driver', 'labs/network_followthrough/tcp', 'labs/network_followthrough/arp', 'labs/network_followthrough/ip', 'labs/network_followthrough/scope', 'labs/b045_systems/include', 'labs/communications/include', 'labs/ethernet_followthrough/ring', 'labs/ethernet_followthrough/phy', 'labs/ethernet_depth/include']
MUTANTS={1: 'net.pending_tcp && !net.tcp.flight && !net.tcp.dma', 2: 'net.tcp.flight && !net.tcp.dma', 3: 'net.tcp.state == NC_CLOSED && net.tcp.dma', 4: 'net.rx_frames == net.rx_released', 5: 'net.pending_tcp && !net.tcp.flight && net.tcp.snd_nxt == 1000', 6: 'model.wire_length == 70', 7: 'net.paused_tcp == 1 && net.pending_tcp'}
KEYS=['tcp','arp','ip','scope','stream','http','systems','ring','ring_model','phy','phy_model','ethernet','bringup','runtime','strings']
MARKER='NB_PASS actual_EnRing_ARP_TCP_copy_pressure_ACK_DMA_link_quiescence'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
 files={Path(__file__),*[ROOT/p for p in PROVIDERS.values()],ROOT/'labs/ethernet_followthrough/ring/target.h',ROOT/'labs/ethernet_followthrough/ring/variants.h',ROOT/'labs/network_followthrough/tcp/test_wire.h',ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/'assessor/ethernet_followthrough/ring/port_audit.py'}
 files.update(p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 files.update(ROOT/p for p in ['labs/network_followthrough/tcp/tcp.h','labs/network_followthrough/arp/arp.h','labs/network_followthrough/ip/ip.h','labs/network_followthrough/scope/scope.h','labs/network_followthrough/scope/stream.h','labs/communications/include/com_http.h','labs/communications/include/com_net.h','labs/ethernet_followthrough/phy/phy.h','labs/ethernet_followthrough/phy/model.h','labs/ethernet_followthrough/ring/ring.h','labs/ethernet_followthrough/ring/model.h','labs/ethernet_depth/include/ethernet_depth.h','labs/bringup_depth/bringup.h','labs/b045_systems/include/systems.h'])
 return sorted(files)
def target_args(dest,manifest,cc,opt,mutant=0,stub=False):
 return [sys.executable,LAB/'target_build.py','--build',dest,'--providers',manifest,'--cc',cc,'--opt',opt,'--mutant',str(mutant),'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib','--source',(LAB/'learner.c' if stub else REF/'driver.c'),'--driver',REF/'target.c','--board',REF/'board.c','--phy-port',ROOT/PROVIDERS['phy_port'],'--ring-port',ROOT/PROVIDERS['ring_port'],*[x for inc in INCLUDES for x in ['--include',ROOT/inc]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-driver'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
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
   exe=w/(tag+'-'+str(variant));source=LAB/'learner.c' if variant=='stub' else REF/'driver.c';m=0 if variant=='stub' else variant
   ok,_=run(tag+' '+str(variant)+' compile',[cc,*flags,'-DNB_MUTANT='+str(m),source,LAB/'tests.c',*[w/'checksum.c' if k=='systems' else ROOT/PROVIDERS[k] for k in KEYS[:-2]],'-o',exe])
   if ok:run(tag+' '+str(variant)+' execute',[exe],int(variant!=0),MARKER if variant==0 else 'nb_init(&net, &ring, config, 1, 1000, 16, 0) == NB_OK' if variant=='stub' else MUTANTS[variant])
  exe=w/(tag+'-variants');ok,_=run(tag+' changed contracts compile',[cc,*flags,REF/'predecessor_variants.c',ROOT/PROVIDERS['variants'],ROOT/PROVIDERS['variant_tests'],ROOT/PROVIDERS['phy_tests'],ROOT/PROVIDERS['phy'],ROOT/PROVIDERS['phy_model'],ROOT/PROVIDERS['ethernet'],'-o',exe])
  if ok:run(tag+' changed contracts execute',[exe],marker='NB_PREDECESSOR_VARIANTS_PASS FCS_policy_cache_lines_publication')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;dest=w/tag;ok,_=run(tag+' build',target_args(dest,manifest,cc,opt),marker='NB_TARGET_BUILD_PASS explicit_objects=23 original_runtime')
   if ok:
    elf=dest/'image.elf';run(tag+' execute',emulator(elf),marker='NB_TARGET_PASS actual_ring_ARP_IP_TCP CPU_state_only');run(tag+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' size',['arm-none-eabi-size','-A',elf]);run(tag+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(tag+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(tag+' missing ring rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in range(23) if i!=10],'-o',dest/'missing-ring.elf'],1,'en_tx_copy')
    # Retain non-model selected MMIO functions in a separately linked image.
    flags=['-std=c17','-O'+opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections',*includes,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
    ports=[]
    for k in ['phy_port','ring_port']:
     obj=dest/(k+'.o');ports.append(obj);run(tag+' '+k+' non-model compile',[cc,*flags,'-c',ROOT/PROVIDERS[k],'-o',obj])
    run(tag+' selected MMIO audit',[sys.executable,ROOT/'assessor/ethernet_followthrough/ring/port_audit.py',ports[1]],marker='"passed": true')
    run(tag+' selected port link',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in range(23) if i not in [20,21]],*ports,'-Map='+str(dest/'f439.map'),'-o',dest/'f439-not-executed.elf'])
    run(tag+' selected port closure',['arm-none-eabi-nm','-u',dest/'f439-not-executed.elf'],empty=True)
 for v in [*MUTANTS,'stub']:
  dest=w/('target-'+str(v));ok,_=run('target '+str(v)+' build',target_args(dest,manifest,'arm-none-eabi-gcc','0',v if isinstance(v,int) else 0,v=='stub'),marker='NB_TARGET_BUILD_PASS explicit_objects=23 original_runtime')
  if ok:run('target '+str(v)+' execute',emulator(dest/'image.elf'),1,'nb_init(&net, &ring, config, 1, 1000, 16, 0) == NB_OK' if v=='stub' else MUTANTS[v])
 run('CCM link rejected',['arm-none-eabi-ld','--gc-sections','--defsym','ETH_BASE=0x10001000','-T',LAB/'target.ld',*[w/'arm-none-eabi-gcc-O0'/(str(i)+'.o') for i in range(23)],'-o',w/'ccm.elf'],1,'EN_DMA_REACHABILITY')
 for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),'--providers',manifest,*([] if stub else ['--source',REF/'driver.c'])],int(stub),'NB_CHECK' if stub else 'NB_PASS')
 missing=w/'missing.json';missing.write_text('{}');run('missing explicit providers rejected',[sys.executable,LAB/'run.py','--build',w/'missing-provider','--providers',missing],2,'missing explicit providers')
 rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
 d=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Actual author struct en_ring/struct ep_phy/struct na_arp/struct ni_reassembly/struct nc_stack providers; subsequent learner substitution and independent assessment remain separate.','MPS2 executes CPU state and selected register-routing models; it does not emulate Ethernet wire, PHY electronics or clock waveforms.','F439 non-model image is linked/audited only and never executed/flashed by this campaign.','One serialized owner, two copied TX descriptors, four RX buffers, one TCP connection; no concurrent ISR entry or complete RFC host conformance.','Raw register failures may leave resources live: stop timeout and partial cold preparation retain ownership and require diagnosis.','Source-defined finite variants and7 semantic controls do not prove absolute mastery or all schedules.','No physical timing, independent review, independent learner or delayed attempt is fabricated.'])
 out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
