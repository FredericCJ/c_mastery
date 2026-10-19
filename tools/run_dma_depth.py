"""Execute source-bound adc arithmetic, preload and actual register adapters."""
from pathlib import Path
import argparse,datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/peripheral_depth/dma';REF=ROOT/'assessor/peripheral_depth/dma'
FW=ROOT/'assessor/firmware_io/firmware_io.c';CACHE=ROOT/'assessor/firmware_lifecycle/targets/h745_cache.c';PORT=ROOT/'labs/firmware_lifecycle/targets/h745_port.c'
HELPERS=ROOT/'assessor/execution/support.c';BOOT=ROOT/'labs/smp_depth/acpi/big_endian.S';BE=ROOT/'assessor/peripheral_depth/mmio/big_endian.c'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
TARGETS=['f439','h745-m7','h745-m4']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return [Path(__file__),FW,CACHE,PORT,HELPERS,BOOT,BE,ROOT/'labs/firmware_io/include/firmware_io.h',ROOT/'labs/firmware_lifecycle/targets/h745_cache.h',*[p for base in [LAB,REF] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_units(target):
 units=[('core',REF/'dma.c'),('firmware',FW),('helpers',HELPERS)]
 if target!='h745-m4':units+=[('target',REF/'target.c')]
 if target.startswith('h745'):units+=[('notice',REF/'notice.c'),('notice_port',REF/'notice_port.c'),('cache',CACHE),('cache_port',PORT)]
 return units

def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();build=ROOT/'build/dma-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build.mkdir(parents=True);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};rows=[]
 inc=['-I'+str(p) for p in [LAB,ROOT/'labs/firmware_lifecycle/targets',ROOT/'labs/firmware_io/include']]
 def run(label,cmd,expected=0,marker=None,empty=False):
  cmd=list(map(str,cmd));log=build/(str(len(rows))+'.log')
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90);out=r.stdout+r.stderr;rc=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);rc=None
  log.write_text(out);ok=rc==expected and (not marker or marker in out) and (not empty or not out.strip()) and not any(s in out for s in ['runtime error:','ERROR: AddressSanitizer'])
  rows.append(dict(label=label,command=cmd,returncode=rc,expected=expected,marker=marker,empty=empty,passed=ok,raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,ok,flush=True);return ok,out
 qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
 for tool in ['gcc','clang','arm-none-eabi-gcc',qemu,lld]:run(str(tool)+' version',[tool,'--version'])
 run('environment',['uname','-a'])
 for cc,opt,tag in PROFILES:
  flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*inc]+(['-O1','-g'] if 'asan' in tag else [])+(['-DNDEBUG'] if 'release' in tag else [])
  def cases(kind,variants,filename,test,extras,defs,mutant,passmark,failmark):
   for v in variants:
    exe=build/(tag+'-'+kind.replace(' ','-')+'-'+str(v));src=(LAB if v=='stub' else REF)/filename
    ok,_=run(tag+' '+kind+'compile '+str(v),[cc,*flags,*defs,'-D'+mutant+'='+str(0 if v=='stub' else v),src,test,*extras,'-o',exe])
    if ok:run(tag+' '+kind+'execute '+str(v),[exe],0 if v==0 else 1,passmark if v==0 else failmark)
  cases('',[0,1,2,3,4,5,6,'stub'],'dma.c',LAB/'tests.c',[],[],'PD_MUTANT','PD_PASS','PD_CHECK:')
  for board in [0,1]:cases('target'+str(board)+' ',[0,1,2,3,*([4] if board==0 else []),'stub'],'target.c',REF/'target_test.c',[REF/'dma.c'],['-DPD_TARGET_MODEL','-DPD_BOARD='+str(board)],'PD_TARGET_MUTANT','PD_TARGET_PASS','PD_TARGET_CHECK:')
  cases('notice ',[0,1,2,3,'stub'],'notice.c',REF/'notice_test.c',[CACHE],[],'PD_NOTICE_MUTANT','PD_NOTICE_PASS','PD_NOTICE_CHECK:')
  for m7 in [0,1]:cases('port'+str(m7)+' ',[0,1,2,3,'stub'],'notice_port.c',REF/'notice_port_test.c',[REF/'notice.c'],['-DPD_PORT_MODEL','-DPD_PEER_M7='+str(m7)],'PD_PORT_MUTANT','PD_PORT_PASS','PD_PORT_CHECK:')
  cases('cache ',[0,3,6],'dma.c',REF/'cache_test.c',[CACHE],[],'PD_MUTANT','PD_CACHE_PASS','PD_CHECK:')
  for name,src,extras,marker in [('composition',REF/'composition.c',[FW],'PD_COMPOSE_PASS'),('oracle',REF/'probe.c',[],None),('notice-oracle',REF/'notice_probe.c',[],None)]:
   exe=build/(tag+'-'+name);ok,_=run(tag+' '+name+' compile',[cc,*flags,REF/('notice.c' if name=='notice-oracle' else 'dma.c'),src,*extras,'-o',exe])
   if ok:
    ok,_=run(tag+' '+name+' execute',[exe],marker=marker)
    if ok and 'oracle' in name:run(tag+' independent '+name,[sys.executable,REF/('notice_oracle.py' if name=='notice-oracle' else 'oracle.py'),ROOT/rows[-1]['raw']])
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,4,5,6,'stub']:
   label='big-endian'+opt+' '+str(v);exe=build/('be'+opt+str(v));src=LAB/'dma.c' if v=='stub' else REF/'dma.c'
   flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17',opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DPD_BIG_ENDIAN','-DPD_MUTANT='+str(0 if v=='stub' else v),*inc]
   ok,_=run(label+' compile',['clang',*flags,src,LAB/'tests.c',BE,BOOT,'-o',exe])
   if ok:run(label+' ELF',['readelf','-hW',exe],marker='big endian');run(label+' closure',['nm','-u',exe],empty=True);run(label+' execute',[qemu,exe],0 if v==0 else 1,'PD_PASS' if v==0 else 'PD_CHECK:')
 for target in TARGETS:
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['-O0','-O3']:
    tag=target+'-'+cc+opt;dest=build/tag;dest.mkdir();objs=[];m7=int(target=='h745-m7')
    flags=['-std=c17',opt,'-mcpu='+('cortex-m7' if m7 else 'cortex-m4'),'-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-DPD_BOARD='+str(int(target.startswith('h745'))),'-DPD_PEER_M7='+str(m7),'-DFW_H745_M7='+str(m7),*inc,'-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib']+(['--target=arm-none-eabi'] if cc=='clang' else [])
    for name,src in target_units(target):
     obj=dest/(name+'.o');objs.append(obj);ok,_=run(tag+' '+name+' compile',[cc,*flags,'-c',src,'-o',obj])
     if ok:run(tag+' '+name+' imports',['arm-none-eabi-nm','-u',obj])
    keep=['pd_validate','pd_submit','pd_poll','pd_cancel','pd_finalize','pd_suffix','fw_dma']
    if target!='h745-m4':keep+=['pd_target_io']
    if target.startswith('h745'):keep+=['pd_notice','pd_h745_notice','pd_h745_notice_cold_start','fw_cache_range','fw_peer_step','fw_h745_cache','fw_h745_step']
    elf=dest/'audit.elf';args=[v for name in keep for v in ['-u',name]];script=['-T',LAB/'h745_audit.ld'] if target.startswith('h745') else ['-Ttext=0x08000000']
    ok,_=run(tag+' combined audit link',['arm-none-eabi-ld','--gc-sections','-e','pd_submit',*script,*args,*objs,'-Map='+str(dest/'image.map'),'-o',elf])
    if ok:
     run(tag+' image closure',['arm-none-eabi-nm','-u',elf],empty=True);run(tag+' image symbols',['arm-none-eabi-nm',elf]);run(tag+' image sections',['arm-none-eabi-readelf','-SW',elf]);run(tag+' cost',['arm-none-eabi-size','-A',elf]);run(tag+' attributes',['arm-none-eabi-readelf','-A',elf])
     for name in ['target'] if target=='f439' else (['target','notice_port','cache_port'] if m7 else ['notice_port','cache_port']):run(tag+' '+name+' disassembly',['arm-none-eabi-objdump','-dr',dest/(name+'.o')])
 run('capture analysis',[sys.executable,REF/'capture_test.py'])
 rows.append(dict(label='sources stable',passed=all(sha(ROOT/p)==h for p,h in pins.items())))
 out=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tools_sha256={str(p):sha(p) for p in [qemu,lld]},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()},limits=['No physical DMA/cache/peer observation or interrupt latency measured.','Twelve audit images are linked, not boot firmware; H745 trusted clock/MPU/reset prerequisites remain B06 responsibilities.','Actual fw_dma/fw_cache_range/fw_peer_step providers are assessor artifacts, not learner submissions.','PPC64 exercises endian/data-model changes only.','Register/cache/HSEM models are original bounded models, not cycle-accurate silicon.','Mailbox notification is polling with payload acknowledgement; no interrupt/wakeup latency claim.'])
 report=ROOT/a.report;report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
