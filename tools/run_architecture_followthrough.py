"""Execute exact T20 architectural mechanisms, adapters and bounded analyses."""
import datetime,hashlib,json,os,pathlib,platform,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/api_depth/architecture';PRIVATE=ROOT/'assessor/api_depth/architecture';FOLDER=ROOT/'research/v5/expansion/legacy_architecture'
VENDOR=pathlib.Path('/home/frede/cmastery-architecture-tools')
UNITS={'core':['core'],'queue':['queue'],'ports':['ports'],'machine':['machine'],'config':['config'],'supervisor':['supervisor'],'indicator':['indicator'],'borrow':['borrow'],'linux':['linux','core'],'board_input':['board_input','core'],'ring':['ring','queue'],'effects':['effects','core','linux']}
MUTANTS={1:'core',2:'queue',3:'ports',4:'machine',5:'config',6:'supervisor',7:'indicator',8:'linux',9:'borrow',10:'board_input',11:'ring',12:'effects'}
PROVIDERS=['labs/foundations/include/foundation.h','assessor/foundations/foundation.c','labs/containers_depth/containers.h','assessor/containers_depth/sequence.c','labs/api_depth/stream/stream.h','assessor/api_depth/stream.c','labs/api_depth/abstractions/registry.h','assessor/api_depth/abstractions/registry.c','labs/api_depth/include/api_depth.h','assessor/api_depth/api_depth.c','assessor/firmware_lifecycle/targets/f439_port.c','labs/machine_image/freestanding_depth/free.h','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 files=[*PUBLIC.glob('*'),*PRIVATE.glob('*'),pathlib.Path(__file__),*[ROOT/x for x in PROVIDERS],* (ROOT/'labs/firmware_lifecycle').rglob('*.h')]
 if (FOLDER/'resources.json').exists():files.append(FOLDER/'resources.json')
 return {p.relative_to(ROOT).as_posix():sha(p) for p in files if p.is_file()}
def main():
 build=ROOT/'build/architecture-followthrough';build.mkdir(parents=True,exist_ok=True);FOLDER.mkdir(parents=True,exist_ok=True)
 initial=source_pins();commands=[];outputs={};observations={};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(name,cmd,negative=False,marker=None):
  t=time.monotonic();r=subprocess.run(list(map(str,cmd)),cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180)
  passed=(r.returncode!=0 if negative else r.returncode==0) and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,cmd)),exit_code=r.returncode,passed=passed,output=r.stdout,seconds=time.monotonic()-t));print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+r.stdout)
  return r.stdout
 def keep(p):outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 includes=['-I'+str(p) for p in [PUBLIC,ROOT/'labs/foundations/include',ROOT/'labs/containers_depth',ROOT/'labs/api_depth/stream',ROOT/'labs/api_depth/include',ROOT/'labs/api_depth/abstractions',ROOT/'labs/firmware_lifecycle/include',ROOT/'labs/firmware_lifecycle/targets']]
 def deps(unit):return [ROOT/'assessor/api_depth/stream.c'] if unit=='linux' else [ROOT/'assessor/firmware_lifecycle/targets/f439_port.c'] if unit=='board_input' else [ROOT/'assessor/containers_depth/sequence.c'] if unit=='ring' else []
 external={}
 try:
  run('platform',['uname','-a']);run('runtime',['ldd','--version'])
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    prefix=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-Wno-error=misleading-indentation',*flags,*includes]
    for unit,names in UNITS.items():
     binary=build/(prefix+'.'+unit);sources=[PRIVATE/(n+'.c') for n in names];run(prefix+'.'+unit+'.compile',base+[PUBLIC/(unit+'_test.c'),*sources,*deps(unit),'-pthread','-o',binary]);observations[prefix+'.'+unit]=run(prefix+'.'+unit,[binary],marker='PASS');keep(binary)
     stub=build/(prefix+'.'+unit+'.learner');learner=[PUBLIC/'board_learner.c',PRIVATE/'core.c'] if unit=='board_input' else [PUBLIC/'ring_learner.c',PRIVATE/'queue.c'] if unit=='ring' else [PUBLIC/'effects_learner.c',PRIVATE/'core.c',PRIVATE/'linux.c'] if unit=='effects' else [PUBLIC/'learner.c'];run(prefix+'.'+unit+'.learner.compile',base+[PUBLIC/(unit+'_test.c'),*learner,*deps(unit),'-pthread','-o',stub]);run(prefix+'.'+unit+'.learner',[stub],True,'AR_ASSERT');keep(stub)
    binary=build/(prefix+'.composition');run(prefix+'.composition.compile',base+[PUBLIC/'composition_test.c',PRIVATE/'core.c',PRIVATE/'ring.c',ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/containers_depth/sequence.c','-o',binary]);observations[prefix+'.composition']=run(prefix+'.composition',[binary],marker='PASS');keep(binary)
    binary=build/(prefix+'.outbox_fault');run(prefix+'.outbox_fault.compile',base+[PUBLIC/'outbox_fault_test.c',PRIVATE/'linux.c','-Wl,--wrap=pwrite','-Wl,--wrap=fsync','-o',binary]);observations[prefix+'.outbox_fault']=run(prefix+'.outbox_fault',[binary],marker='PASS');keep(binary)
    for mutant,unit in MUTANTS.items():
     binary=build/(prefix+'.mutant'+str(mutant));run(prefix+'.mutant'+str(mutant)+'.compile',base+['-DAR_MUTANT='+str(mutant),PUBLIC/(unit+'_test.c'),*[PRIVATE/(n+'.c') for n in UNITS[unit]],*deps(unit),'-pthread','-o',binary]);run(prefix+'.mutant'+str(mutant),[binary],True,'AR_ASSERT');keep(binary)
    binary=build/(prefix+'.plugin');run(prefix+'.plugin.compile',base+[PUBLIC/'plugin_test.c',ROOT/'assessor/api_depth/abstractions/registry.c',ROOT/'assessor/api_depth/api_depth.c','-o',binary]);observations[prefix+'.plugin']=run(prefix+'.plugin',[binary],marker='PASS');keep(binary)
    for mutant in [0,1]:
     binary=build/(prefix+'.property'+str(mutant));run(prefix+'.property'+str(mutant)+'.compile',base+['-DAR_MUTANT='+str(mutant),PUBLIC/'property_driver.c',PRIVATE/'core.c','-o',binary]);observations[prefix+'.property'+str(mutant)]=json.loads(run(prefix+'.property'+str(mutant),['python3',PUBLIC/'property_oracle.py',binary]+(['--expect-mutant'] if mutant else [])));keep(binary)
   ineffective=build/(compiler+'.ineffective');run(compiler+'.ineffective.compile',[compiler,'-std=c17','-O2','-DAR_MUTANT=1',*includes,PUBLIC/'ineffective_test.c',PRIVATE/'core.c','-o',ineffective]);run(compiler+'.ineffective',[ineffective]);keep(ineffective);observations[compiler+'.ineffective']=dict(mutant_linked=True,returned_success=True,acceptance=False,reason='No operation or semantic assertion was exercised')
   for policy in [1,2]:
    name=compiler+'.policy'+str(policy);binary=build/name;run(name+'.compile',[compiler,'-std=c17','-O2','-DAR_STATIC_POLICY='+str(policy),*includes,PUBLIC/'policy_test.c','-o',binary]);observations[name]=run(name,[binary],marker='PASS');run(name+'.assembly',['objdump','-dr',binary]);observations[name+'.size']=run(name+'.size',['size',binary]);keep(binary)
   for cap,feature in [(2,0),(4,0),(4,1)]:
    name=compiler+'.config.'+str(cap)+'.'+str(feature);binary=build/name;run(name+'.compile',[compiler,'-std=c17','-O2','-DAR_CAP='+str(cap),'-DAR_FEATURE='+str(feature),*includes,PUBLIC/'config_test.c',PRIVATE/'config.c','-o',binary]);observations[name]=run(name,[binary],marker='PASS');keep(binary)
   for unit in ['core','queue','ports','machine','config','supervisor','indicator','borrow']:
    obj=build/(compiler+'.'+unit+'.o');run(compiler+'.'+unit+'.object',[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage',*includes,'-c',PRIVATE/(unit+'.c'),'-o',obj]);run(compiler+'.'+unit+'.symbols',['nm','-u',obj]);keep(obj)
   closure_objects=[build/(compiler+'.'+unit+'.o') for unit in ['core','queue','ports','machine','config','supervisor','indicator','borrow']]
   for helper in ['bytes','memory_provider']:
    obj=build/(compiler+'.helper.'+helper+'.o');run(compiler+'.helper.'+helper,[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(ROOT/'labs/machine_image/freestanding_depth'),'-c',ROOT/('assessor/legacy_systems/freestanding_depth/'+helper+'.c'),'-o',obj]);keep(obj);closure_objects.append(obj)
   combined=build/(compiler+'.profileC.o');run(compiler+'.profileC.link',['ld','-r',*closure_objects,'-o',combined]);assert not run(compiler+'.profileC.closed',['nm','-u',combined]).strip();keep(combined)
   dependency=run(compiler+'.core.dependencies',[compiler,'-std=c17','-MM',*includes,PRIVATE/'core.c']);assert 'stdio' not in dependency and 'stm32' not in dependency;observations[compiler+'.core.dependencies']=dependency
   for leak in ['FILE *stream;','GPIO_TypeDef *port;']:
    file=build/'bad_boundary.c';file.write_text('#include "arch.h"\n'+leak+'\n');run(compiler+'.boundary.reject.'+leak.split()[0],[compiler,'-std=c17','-Werror',*includes,'-fsyntax-only',file],True)
   # Both binaries carry source/tool/config identities; binary hash is external.
   tool=run(compiler+'.identity.tool',[compiler,'--version']);source=sha(PUBLIC/'identity.c');config='0102000000000014';tool_id=hashlib.sha256(tool.encode()).hexdigest();headers=[]
   for label in ['original','relocated']:
    directory=build/(compiler+'.identity.'+label);directory.mkdir(exist_ok=True);h=directory/'ar_build.h';h.write_text('#define AR_SOURCE_ID "'+source+'"\n#define AR_TOOL_ID "'+tool_id+'"\n#define AR_CONFIG_ID "'+config+'"\n');binary=directory/'identity';run(compiler+'.identity.'+label+'.compile',[compiler,'-std=c17','-O2','-DAR_CAP=2','-I'+str(directory),*includes,PUBLIC/'identity.c','-o',binary]);headers.append(run(compiler+'.identity.'+label,[binary]));keep(binary);keep(h)
   assert headers[0]==headers[1];observations[compiler+'.identity.relocation']=dict(semantic_equal=True,binary_equal=sha(build/(compiler+'.identity.original/identity'))==sha(build/(compiler+'.identity.relocated/identity')))
   old=build/(compiler+'.identity.original/identity');new_config='0102010000000014';directory=old.parent;header=directory/'ar_build.h';snapshot=directory/'ar_build.initial.h';snapshot.write_bytes(header.read_bytes());keep(snapshot);header.write_text(header.read_text().replace(config,new_config));keep(header)
   stale=run(compiler+'.identity.stale-observation',[old]);assert stale.split()[2]!=new_config;observations[compiler+'.identity.stale_rejected']=dict(expected=new_config,observed=stale.split()[2],initial_header_sha256=sha(snapshot),changed_header_sha256=sha(header))
   fresh=directory/'identity_changed';run(compiler+'.identity.changed.compile',[compiler,'-std=c17','-O2','-DAR_CAP=2','-I'+str(directory),*includes,PUBLIC/'identity.c','-o',fresh]);assert run(compiler+'.identity.changed',[fresh]).split()[2]==new_config;keep(fresh)
  for path in build.glob('*.su'):keep(path);observations[path.name]=path.read_text()
  observations['models']=json.loads(run('models',['python3',PRIVATE/'models.py']))
  hal=VENDOR/'stm32f4xx-hal-driver-1.8.3';device=VENDOR/'cmsis-device-f4-2.6.10';cmsis=VENDOR/'CMSIS_5-5.9.0';vendor_source=hal/'Src/stm32f4xx_hal_gpio.c';vendor_includes=['-I'+str(p) for p in [hal/'Inc',device/'Include',cmsis/'CMSIS/Core/Include']]
  for p in [*VENDOR.glob('*.tar.gz'),vendor_source,hal/'LICENSE.md']:
   external[str(p)]=sha(p)
  for compiler in ['gcc','clang']:
   binary=build/(compiler+'.vendor');run(compiler+'.vendor.compile',[compiler,'-std=c17','-O2','-DSTM32F439xx','-Wno-pointer-to-int-cast',*includes,*vendor_includes,PUBLIC/'vendor_test.c',PRIVATE/'indicator.c',vendor_source,'-o',binary]);observations[compiler+'.vendor']=run(compiler+'.vendor',[binary],marker='PASS');keep(binary)
  arm=['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-DSTM32F439xx',*includes,*vendor_includes]
  run('arm.version',['arm-none-eabi-gcc','--version']);objects=[]
  for name,source in [('indicator',PRIVATE/'indicator.c'),('target',PRIVATE/'f439.c'),('vendor',vendor_source),('core',PRIVATE/'core.c'),('board_input',PRIVATE/'board_input.c'),('uart',ROOT/'assessor/firmware_lifecycle/targets/f439_port.c')]:
   obj=build/('arm.'+name+'.o');run('arm.'+name+'.compile',arm+['-MMD','-MF',str(obj)+'.d','-c',source,'-o',obj]);objects.append(obj);keep(obj);run('arm.'+name+'.symbols',['arm-none-eabi-nm','-u',obj]);run('arm.'+name+'.assembly',['arm-none-eabi-objdump','-dr',obj]);dep=pathlib.Path(str(obj)+'.d');keep(dep)
   # Capture every actual external header selected by the compiler.
   for token in dep.read_text().replace('\\\n',' ').split():
    p=pathlib.Path(token)
    if p.is_file() and str(p).startswith(str(VENDOR)):external[str(p)]=sha(p)
  for entry in ['ar_target_minimal','ar_target_vendor','ar_target_input']:
   elf=build/(entry+'.elf');run('arm.'+entry+'.link',arm+['-nostdlib','-Wl,--gc-sections','-Wl,-e,'+entry,'-Wl,-Ttext=0x08000000',*(['-Wl,-u,ar_target_input_init'] if entry=='ar_target_input' else []),*objects,'-o',elf]);undefined=run('arm.'+entry+'.closed',['arm-none-eabi-nm','-u',elf]);assert not undefined.strip();observations['arm.'+entry+'.size']=run('arm.'+entry+'.size',['arm-none-eabi-size',elf]);keep(elf)
 except Exception as e:failure=str(e)
 else:failure=None
 final=source_pins()
 if initial!=final:failure='source changed during campaign'
 result=dict(schema_version=1,status='pass' if failure is None else 'fail',timestamp=datetime.datetime.now(datetime.timezone.utc).isoformat(),scope='T20 author implementation, actual bounded Linux adapters, selected hardware cross-build and separate finite analysis',source_sha256=final,external_source_sha256=external,outputs_sha256=outputs,commands=commands,observations=observations,failure=failure,environment=platform.platform(),limitations=['No external review or independent learner/delayed performance.','No physical F439 execution, reset, pin waveform or measured hardware latency.','Vendor GPIO RAM-object execution is a selected register-structure model; ARM image closure is not target execution.','Scheduling/containment/decision dossiers are explicitly finite analyses.','Linux fsync plus child exit is not a power-loss durability experiment or arbitrary external exactly-once guarantee.'])
 (FOLDER/'validation.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=result['status'],commands=len(commands),failure=failure)),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
