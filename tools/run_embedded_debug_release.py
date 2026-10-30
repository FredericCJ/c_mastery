"""Run original trace/release logic, isolated M4 reset and exact ELF packaging."""
import argparse,hashlib,importlib.util,json,os,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/embedded_debug_depth/release';BASE=ROOT/'reviews/v5/expansion/legacy_embedded_debug_release';CORE=ROOT/'assessor/embedded_debug_depth/release.c'
def load(name,path):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
guest=load('debug_release_guest',PUBLIC/'guest.py');oracle=load('debug_release_oracle',PUBLIC/'oracle.py');manifest=load('debug_release_manifest',ROOT/'assessor/embedded_debug_depth/release_manifest.py')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',CORE,ROOT/'assessor/embedded_debug_depth/release_target.c',ROOT/'assessor/embedded_debug_depth/release_manifest.py',ROOT/'assessor/embedded_debug_depth/release.md',ROOT/'assessor/execution/support.c',*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def packaging(work,profile,cc,opt,run):
 folder=work/(profile+'-package');folder.mkdir();compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];obj=folder/'layout.o';flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables'];run([*compiler,*flags,'-c',PUBLIC/'layout.c','-o',obj],profile+'-layout-compile')
 images=[]
 for shifted in (False,True):
  name='changed' if shifted else 'original';ld=folder/(name+'.ld');ld.write_text((PUBLIC/'layout.ld').read_text(encoding='utf-8').replace('0x08000000','0x08000100') if shifted else (PUBLIC/'layout.ld').read_text(encoding='utf-8'),encoding='utf-8',newline='\n');image=folder/(name+'.elf');mapping=folder/(name+'.map');run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',obj,'-T',ld,'-Wl,-Map='+str(mapping),'-o',image],profile+'-'+name+'-link');deploy=folder/(name+'.stripped.elf');symbols=folder/(name+'.debug');binary=folder/(name+'.bin');run(['arm-none-eabi-objcopy','--only-keep-debug',image,symbols],profile+'-'+name+'-symbols');run(['arm-none-eabi-objcopy','--strip-all',image,deploy],profile+'-'+name+'-strip');run(['arm-none-eabi-objcopy','-O','binary','--gap-fill','0xff',image,binary],profile+'-'+name+'-binary');headers,_=run(['arm-none-eabi-readelf','-lW',image],profile+'-'+name+'-headers');names,_=run(['arm-none-eabi-nm','-n',image],profile+'-'+name+'-names');entry=next(int(row.split()[0],16) for row in names.splitlines() if row.split()[-1]=='release_entry')&~1
  args=[image.read_bytes(),deploy.read_bytes(),symbols.read_bytes(),mapping.read_bytes(),sha(PUBLIC/'layout.c'),flags,'dedicated-synthetic-F439-layout-not-programmed'];bound=manifest.build(*args);record={'manifest':bound['identity'],'pc':entry};attribution=manifest.attribute(bound,record,*args)
  parsed=[tuple(int(x,16) for x in line.split()[2:6]) for line in headers.splitlines() if line.strip().startswith('LOAD ')]
  actual=[(r['virtual'],r['load'],r['file_bytes'],r['memory_bytes']) for r in bound['segments']]
  if parsed!=actual:raise ValueError('Independent readelf segment disagreement')
  spans=[r for r in bound['segments'] if r['file_bytes']];first=0x08000100 if shifted else 0x08000000;raw=binary.read_bytes();text=next(r for r in spans if r['load']==first);data=next(r for r in spans if r['load']==0x08001000);offset=0x08001000-first
  if len(spans)!=2 or raw[:text['file_bytes']]!=bytes.fromhex(text['payload']) or raw[text['file_bytes']:offset]!=b'\xff'*(offset-text['file_bytes']) or raw[offset:]!=b'\x78\x56\x34\x12' or data['virtual']!=0x20000000 or data['file_bytes']!=4 or not any(r['memory_bytes']>r['file_bytes'] for r in bound['segments']):raise ValueError('Wrong actual hole/RAM-load/BSS packaging')
  images.append((bound,record,args));(folder/(name+'.manifest.json')).write_text(json.dumps(bound,indent=2)+'\n',encoding='utf-8',newline='\n')
 # Reproduce the discovered implicit-PHDR problem: shifted text can cause GNU
 # ld to map ELF headers. Stripping then changes actual PT_LOAD payload bytes.
 # The validator must reject this; fix the linker contract, not the comparison.
 naive=folder/'implicit.ld';naive.write_text((PUBLIC/'layout.ld').read_text(encoding='utf-8').replace('PHDRS { text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); }','').replace(' :text','').replace(' :data','').replace('0x08000000','0x08000100'),encoding='utf-8',newline='\n');bad=folder/'implicit.elf';badstrip=folder/'implicit.stripped.elf';badmap=folder/'implicit.map'
 run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',obj,'-T',naive,'-Wl,-Map='+str(badmap),'-o',bad],profile+'-implicit-link');run(['arm-none-eabi-objcopy','--strip-all',bad,badstrip],profile+'-implicit-strip')
 if manifest.segments(bad.read_bytes())==manifest.segments(badstrip.read_bytes()):raise ValueError('Missing implicit-header load counterexample')
 try:manifest.build(bad.read_bytes(),badstrip.read_bytes(),images[1][2][2],badmap.read_bytes(),*images[1][2][4:]);rejected=False
 except ValueError:rejected=True
 if not rejected:raise ValueError('Accepted changed loadable ELF headers')
 bound,record,args=images[0];controls=['implicit-loaded-headers-changed-by-strip']
 for label,changed_record,changed_args in [('wrong-layout',record,images[1][2]),('wrong-map',record,[*args[:3],images[1][2][3],*args[4:]]),('wrong-symbols',record,[*args[:2],images[1][2][2],*args[3:]]),('wrong-revision',record,[*args[:4],'0'*64,*args[5:]]),('wrong-flags',record,[*args[:5],['-Owrong'],args[6]]),('wrong-target',record,[*args[:6],'another-board']),('unbound-record',{'manifest':'0'*64,'pc':record['pc']},args),('nonexecuted-PC',{'manifest':bound['identity'],'pc':0x20000000},args)]:
  try:manifest.attribute(bound,changed_record,*changed_args);rejected=False
  except ValueError:rejected=True
  if not rejected:raise ValueError('Accepted '+label)
  controls.append(label)
 return folder,{'profile':profile,'controls':controls,'manifest_identity':bound['identity'],'segments':bound['segments'],'physical_programming':False,'source_revision':sha(PUBLIC/'layout.c')}
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use release evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Original trace/duration/startup logic; actual selected M4 IRQ/reset/timer and GNU ELF/objcopy packaging. No physical Flash, power cycle or CPU-cycle calibration.'};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(cmd,label,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=60);out=r.stdout+r.stderr;p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':cmd,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  v['toolchains']={c:run([c,'--version'],c)[0].splitlines()[0] for c in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-objcopy','arm-none-eabi-readelf','qemu-system-arm')}
  with tempfile.TemporaryDirectory(prefix='debug-release-') as d:
   work=Path(d);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)]
   for profile,cc,flags in PROFILES:
    for variant in (0,1,2,3,4,'stub'):
     name=profile+'-'+str(variant);binary=work/name;source=PUBLIC/'release.c' if variant=='stub' else CORE;_,b=run([cc,*common,*flags,'-DDT_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],name+'-build');_,e=run([binary],name,0 if variant==0 else 1,'DT_PASS:' if variant==0 else 'DT_CHECK:');v['observations'].append({'target':'core','profile':profile,'variant':variant,'build':b,'execution':e,'passed':True})
    name=profile+'-replay';binary=work/name;_,b=run([cc,*common,*flags,CORE,PUBLIC/'replay.c','-o',binary],name+'-build');out,e=run([binary],name);v['observations'].append({'target':'replay','profile':profile,'build':b,'execution':e,'expected':oracle.check(out),'passed':True})
   for profile,cc,opt in GUEST_PROFILES:
    for variant in (0,1,2,'stub'):
     name=profile+'-target-'+str(variant);folder=work/name;begin=len(v['commands']);driver=PUBLIC/'target.c' if variant=='stub' else ROOT/'assessor/embedded_debug_depth/release_target.c';image,audit=guest.build(folder,cc,opt,CORE,driver,ROOT/'assessor/execution/support.c',variant,lambda cmd:run(cmd,name)[0]);rows=v['commands'][begin:];out,e=run(guest.command(image),name,0 if variant==0 else 1,'DT_TARGET_PASS:' if variant==0 else 'DT_TARGET_CHECK:');saved=logs/name;saved.mkdir()
     for p in folder.iterdir():
      if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
     item={'target':'guest','profile':profile,'variant':variant,'build_commands':rows,'execution':e,'audit':audit,'artifacts':saved.relative_to(ROOT).as_posix(),'passed':True}
     if variant==0:item['observed']=guest.observations(out)
     v['observations'].append(item)
    begin=len(v['commands']);folder,observed=packaging(work,profile,cc,opt,run);saved=logs/(profile+'-packaging');saved.mkdir()
    for p in folder.iterdir():
     if p.is_file():(saved/p.name).write_bytes(p.read_bytes())
    v['observations'].append({'target':'packaging','profile':profile,'commands':v['commands'][begin:],'artifacts':saved.relative_to(ROOT).as_posix(),'observed':observed,'passed':True})
   v['observations'].append({'target':'contention','observed':oracle.contention(),'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());v['passed']='error' not in v and v['source_stable'] and len(v['observations'])==58;v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'observations':len(v['observations']),'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
