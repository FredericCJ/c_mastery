"""Execute checked discovery, actual firmware captures and big-endian transfer."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/acpi';BASE=ROOT/'reviews/v5/expansion/legacy_smp_acpi'
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
BE_PROFILES=(('clang-O0',['-O0']),('clang-O2',['-O2']),('clang-O3',['-O3']))
VARIANTS=(0,1,2,3,4,'stub')
def load(name):
 spec=importlib.util.spec_from_file_location('smp_acpi_'+name,PUBLIC/(name+'.py'));result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
oracle=load('oracle');guest=load('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'assessor/legacy_smp/acpi.c',ROOT/'assessor/legacy_smp/acpi.md']
 for directory in (PUBLIC,ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):
  paths += [p for p in directory.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 return sorted(set(paths))

def main(report,ppc_qemu,lld):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use the SMP ACPI successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};external={str(p):sha(p) for p in (ppc_qemu,lld)}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'external_tools_sha256':external,'commands':[],'observations':[],
 'scope':'Transactional MADT and explicitly mapped RSDP/root parsing; six hosted profiles, six owned QEMU discovery captures and three actual big-endian PPC64 ELFv2 profiles.',
 'limits':['ACPI6.6 root/x2 normative passages remain unavailable; inspected ACPICA layouts do not establish full conformance.','Capture instrumentation uses the retained CMK physical-range assumptions; no general physical mapping proof, AP-startup, interrupt, physical-timing or learner claim.','Public implementations remain semantic-failing scaffolds; all successful parser implementations are assessor-only.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None,timeout=180):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=timeout);text=result.stdout+result.stderr
  path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in text:raise ValueError(label+': '+text)
  if any(s in text for s in ('ERROR: AddressSanitizer','runtime error:','Segmentation fault','FAIL SMP:','FAIL host deadline')):raise ValueError(label+': fault is not semantic evidence')
  return text,row
 try:
  value['toolchains']={name:run([tool,'--version'],name)[0].splitlines()[0] for name,tool in (('gcc','gcc'),('clang','clang'),('qemu-system-x86_64','qemu-system-x86_64'),('qemu-ppc64',ppc_qemu),('lld',lld))}
  with tempfile.TemporaryDirectory(prefix='smp-acpi-') as directory:
   work=Path(directory)
   def build_run(command,**kwargs):
    if kwargs:raise ValueError('Unexpected retained build invocation options')
    return run(command,'guest-build')[0]
   cmk,instrumentation=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',work/'capture',run_observer=build_run);value['instrumentation']=instrumentation
   captures=[]
   for machine in ('pc-i440fx-10.2','pc-q35-10.2'):
    for cpus in (1,2,4):
     capture,text=guest.capture(cmk,work/'capture',machine,cpus);stem=machine+'-'+str(cpus);path=logs/(stem+'.log');path.write_text(text,encoding='utf-8',newline='\n');captures.append(capture)
     value['observations'].append({'target':'guest','machine':machine,'cpus':cpus,'passed':True,'stage':capture['stage'],'log':path.relative_to(ROOT).as_posix()})
   capture_path=logs/'captures.json';capture_path.write_text(json.dumps(captures,indent=2)+'\n',encoding='utf-8',newline='\n');value['captures']=capture_path.relative_to(ROOT).as_posix()
   build_path=logs/'guest-build.json';build_path.write_bytes((work/'capture/guest/build.json').read_bytes());value['guest_build']=build_path.relative_to(ROOT).as_posix()
   expected=oracle.capture_header(logs/'capture_cases.h',captures);value['capture_expected']=expected
   oracle.header(logs/'oracle_cases.h');input_text='\n'.join(oracle.inputs())+'\n';(logs/'oracle.in').write_text(input_text,encoding='utf-8',newline='\n');value['oracle_input']=(logs/'oracle.in').relative_to(ROOT).as_posix()
   common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(logs)]
   for profile,compiler,flags in PROFILES:
    for variant in VARIANTS:
     source=PUBLIC/'acpi.c' if variant=='stub' else ROOT/'assessor/legacy_smp/acpi.c';binary=work/(profile+'-'+str(variant));positive=variant==0
     _,build=run([compiler,*common,*flags,'-DSA_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build')
     _,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'SA_PASS:' if positive else 'SA_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'binary_sha256':sha(binary),'passed':True})
     if positive:
      text,execution=run([binary,'stream'],profile+'-oracle',input_text=input_text);value['observations'].append({'target':'oracle','profile':profile,'execution':execution,'expected':oracle.check(text),'passed':True})
      for kind,file,marker in (('guard','guard_pages.c','SA_GUARD_PASS:'),('capture','capture_tests.c','SA_CAPTURE_PASS:')):
       target=work/(profile+'-'+kind);_,build=run([compiler,*common,*flags,source,PUBLIC/file,'-o',target],profile+'-'+kind+'-build');_,execution=run([target],profile+'-'+kind,marker=marker)
       value['observations'].append({'target':kind,'profile':profile,'build':build,'execution':execution,'passed':True})
   for profile,flags in BE_PROFILES:
    for variant in VARIANTS:
     positive=variant==0;source=PUBLIC/'acpi.c' if variant=='stub' else ROOT/'assessor/legacy_smp/acpi.c';binary=work/('be-'+profile+'-'+str(variant))
     command=['clang','--target=powerpc64-linux-gnu','-mabi=elfv2',*common,*flags,'-ffreestanding','-fno-builtin','-fno-stack-protector','-nostdlib','-static','--ld-path='+str(lld),'-Wl,-e,_start','-DSA_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'big_endian.c',PUBLIC/'big_endian.S','-o',binary]
     _,build=run(command,profile+'-'+str(variant)+'-be-build');header,elf=run(['readelf','-hW',binary],profile+'-'+str(variant)+'-be-elf');undefined,nm=run(['nm','-u',binary],profile+'-'+str(variant)+'-be-symbols');dynamic,dyn=run(['readelf','-dW',binary],profile+'-'+str(variant)+'-be-dynamic')
     if 'big endian' not in header or 'PowerPC64' not in header or 'abiv2' not in header or undefined.strip() or 'NEEDED' in dynamic:raise ValueError('Wrong endian/ABI/runtime closure')
     _,execution=run([ppc_qemu,binary],profile+'-'+str(variant)+'-be',0 if positive else 1,'SA_BE_PASS:' if positive else 'SA_BE_CHECK:')
     value['observations'].append({'target':'big-endian','profile':profile,'variant':variant,'build':build,'elf':elf,'undefined':nm,'dynamic':dyn,'execution':execution,'binary_sha256':sha(binary),'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items()) and all(sha(Path(p))==h for p,h in external.items())
 value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==78
 value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.iterdir() if p.is_file()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--ppc-qemu',type=Path,default=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64');p.add_argument('--lld',type=Path,default=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21');a=p.parse_args();raise SystemExit(main(a.report,a.ppc_qemu.absolute(),a.lld.absolute()))
