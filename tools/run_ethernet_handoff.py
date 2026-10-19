"""Source-bound cross-layer consumers; actual C providers and isolated CPU models."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
import run_network_http as network
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_handoff';PUBLIC=ROOT/'labs/ethernet_followthrough/handoff';PRIVATE=ROOT/'assessor/ethernet_followthrough/handoff';BUILD=ROOT/'build'/('ethernet-handoff-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
CASES=['route','clock','divergence','budget','stack','reassembly']
MARKERS=['OBS handoff_'+c for c in CASES[:4]]+['EH_STACK_PASS','EH_REASSEMBLY_PASS']
MUTANTS={'GATEWAY':(0,41,'EH_ASSERT:gateway_not_final','EH_MUTANT=1'),'IP_IDENTITY':(0,41,'EH_ASSERT:final_ip_preserved','EH_MUTANT=2'),'DRIFT':(1,41,'EH_ASSERT:drift_interval','EH_MUTANT=3'),'UNCERTAINTY':(1,41,'EH_ASSERT:anchor_uncertainty','ED_MUTATE_HANDOFF2'),'BUDGET':(3,41,'EH_ASSERT:padding_budget','ED_MUTATE_HANDOFF'),'HTTP':(4,1,'NH_CHECK','NH_MUTANT=1'),'HOLE':(5,1,'NI_CHECK','NI_MUTANT=7'),'OVERLAP':(5,1,'NI_CHECK','NI_MUTANT=5'),'DEADLINE':(5,1,'NI_CHECK','NI_MUTANT=6')}
PROVIDERS={k:network.PROVIDERS[k] for k in network.KEYS};PROVIDERS['application']='assessor/network_followthrough/http/http.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=set(network.inputs())|{pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'tools/run_network_driver.py',ROOT/'labs/network_followthrough/ip/specimens.h',ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/'labs/ethernet_depth/CONTRACTS.md'}|{p for d in [PUBLIC,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','checksum','providers.missing'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile',*CASES,*[c+'.oracle' for c in CASES],*['stub.'+c for c in CASES[:2]] ]}
   for m in MUTANTS:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:out|={cc+'.O'+opt+'.'+n for n in ['build','execute','oracle','closure','disassembly','stack','missing_application']}
 for m in ['DRIFT','HTTP','stub']:out|={'thumb.'+m+'.build','thumb.'+m+'.execute'}
 return out
def main():
 HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 manifest=BUILD/'providers.json';manifest.write_text(json.dumps({k:str(ROOT/v) for k,v in PROVIDERS.items()},indent=2)+'\n');checksum=BUILD/'checksum.c'
 flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-fno-pie','-no-pie','-I'+str(PUBLIC),*['-I'+str(ROOT/n) for n in network.INCLUDES]]
 def compile(tag,cc,opt,stub=False):
  b=BUILD/tag;sources=[PUBLIC/'learner.c' if stub else PRIVATE/'handoff.c',*[PUBLIC/n for n in ['tests.c','stack.c','reassembly.c','native.c']],*[checksum if k=='systems' else ROOT/v for k,v in PROVIDERS.items() if k not in ['runtime','strings']]];run(tag+'.compile',[cc,*flags,*opt,*sources,'-o',b]);return b
 def target(tag,cc='arm-none-eabi-gcc',opt='0',definition=None,stub=False):
  d=BUILD/tag;args=[sys.executable,PUBLIC/'target_build.py','--build',d,'--providers',manifest,'--root',ROOT,'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib','--cc',cc,'--opt',opt]
  if not stub:args+=['--source',PRIVATE/'handoff.c']
  if definition:args+=['--define',definition]
  run(tag+'.build',args,marker='EH_TARGET_BUILD_PASS explicit_objects=24 no_libraries');return d
 def execute(tag,d,code=0,marker='EH_GUEST_PASS'):run(tag+'.execute',network.emulator(d/'image.elf'),code,marker)
 try:
  for name,args in [('kernel',['uname','-a']),('libc',['ldd','--version']),('arm.version',['arm-none-eabi-gcc','--version']),('qemu.version',['qemu-system-arm','--version'])]:run(name,args)
  run('checksum',[sys.executable,ROOT/'labs/network_followthrough/ip/checksum_slice.py',ROOT/PROVIDERS['systems'],checksum],marker='Exactly two unchanged original checksum definitions')
  bad=BUILD/'missing.json';bad.write_text('{}');run('providers.missing',[sys.executable,PUBLIC/'run.py','--build',BUILD/'missing','--providers',bad,'--root',ROOT],2,'missing explicit providers')
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,opt in PROFILES:
    tag=cc+'.'+profile;b=compile(tag,cc,opt)
    for i,c in enumerate(CASES):run(tag+'.'+c,[b,i],marker=MARKERS[i]);run(tag+'.'+c+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+c+'.log'),c],marker='"status": "pass"')
    b=compile(tag+'.stub',cc,opt,True)
    for i,c in enumerate(CASES[:2]):run(tag+'.stub.'+c,[b,i],41,'EH_ASSERT:')
    for m,(i,code,marker,definition) in MUTANTS.items():b=compile(tag+'.mutation.'+m,cc,[*opt,'-D'+definition]);run(tag+'.mutation.'+m,[b,i],code,marker)
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    tag=cc+'.O'+opt;d=target(tag,cc,opt);execute(tag,d);run(tag+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.execute.log'),'all'],marker='"status": "pass"');assert not run(tag+'.closure',['arm-none-eabi-nm','-u',d/'image.elf']).strip();run(tag+'.disassembly',['arm-none-eabi-objdump','-d',d/'image.elf']);run(tag+'.stack',[sys.executable,PRIVATE/'stack.py',d],marker='"passed": true');run(tag+'.missing_application',['arm-none-eabi-ld','--gc-sections','-T',ROOT/'labs/network_followthrough/driver/target.ld',*[d/(str(i)+'.o') for i in range(24) if i!=22],'-o',d/'missing.elf'],1,'nh_init')
  for m in ['DRIFT','HTTP']:execute('thumb.'+m,target('thumb.'+m,definition=MUTANTS[m][3]),MUTANTS[m][1],MUTANTS[m][2])
  execute('thumb.stub',target('thumb.stub',stub=True),41,'EH_ASSERT:')
  assert {q['id'] for q in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped sources changed'
 limits=['Actual original EnRing/Na/Ni/Nc/Nb/Nh providers; CPU/register models, no physical Ethernet or capture uncertainty observation.','Routed envelope is separate from same-link NbNet/NhHttp exchange; no routed HTTP, general Internet compliance or full standard coverage.','Finite fragmentation consumer covers stated hole/overlap/deadline policy only; no options, arbitrary traffic or peer interoperability conclusion.','Independent original/delayed learner assessment and external review remain not-run.']
 r=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(p for p in BUILD.rglob('*') if p.is_file())},commands=harness.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(r,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=r['status'],commands=len(r['commands']),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
