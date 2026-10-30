"""Execute exact map/lease models with frozen Pd and actual selected link recipe."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_bus';PUBLIC=ROOT/'labs/architecture_bus';PRIVATE=ROOT/'assessor/architecture_bus';BUILD=ROOT/'build'/('architecture-bus-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['topology','placement','lease','arbitration','bridge'];PACKETS=dict(topology=12,placement=961,lease=6,arbitration=14,bridge=15)
MUTANTS={'CCM':('topology','m.regions[MB_CCM].masters==(1u<<MB_CPU_D)','MB_MUTANT_CCM'),'F407':('topology','m.regions[MB_SRAM3].present==(part==MB_F439)','MB_MUTANT_F407'),'END':('placement','MB_EXTENT && offset==999','MB_MUTANT_END'),'RELEASE':('lease','mb_cpu_write(&s,0,99)==PD_BUSY','MB_MUTANT_RELEASE'),'EN_ONLY':('lease','mb_poll(&s,io,1)==PD_BUSY && s.transfer.phase!=3','PD_MUTANT=5'),'FAIR':('arbitration','r.master[i]==i%3','MB_MUTANT_FAIR'),'WAIT':('arbitration','r.finish[2]==13','MB_MUTANT_WAIT'),'BRIDGE':('bridge','result==0x12121212','MB_MUTANT_BRIDGE'),'WIDTH':('bridge','!mb_usart_width(8)','MB_MUTANT_WIDTH')}
IMPORTS=['tools/run_thread_lifecycle.py','labs/peripheral_depth/dma/dma.h','labs/peripheral_depth/dma/dma.c','labs/peripheral_depth/dma/CONTRACTS.md','assessor/peripheral_depth/dma/dma.c','labs/architecture_bytes/placement.c','labs/architecture_bytes/bytes.h','labs/algorithm_wrappers/wrappers.h','labs/containers_depth/containers.h','labs/containers_depth/byte_helpers.c','labs/firmware_lifecycle/targets/f439_service.ld','fixtures/algorithms/PR-T37-06.json']
INCLUDES=['labs/architecture_bus','labs/peripheral_depth/dma','labs/architecture_bytes','labs/algorithm_wrappers','labs/containers_depth']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']}|{pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')})}
def expected_commands():
 out={'kernel','libc','arm.version','qemu.version','linked.generate','linked.negative'}
 for mode in ['sram','ccm','bad']:out|={'ledger.'+mode+'.'+x for x in ['compile','symbols','undefined','headers']}
 for cc in ['gcc','clang']:
  out|={cc+'.version',cc+'.core',cc+'.core.undefined',cc+'.core.stack',cc+'.core.sizes'}
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','disassembly','stub.compile',*CASES,*[c+'.oracle' for c in CASES]]};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
 for p in ['debug','optimized']:out|={'thumb.'+p+'.'+n for n in ['compile','execute','undefined','headers','disassembly']}
 for m in [*MUTANTS,'stub']:out|={'thumb.'+m+'.compile','thumb.'+m+'.execute'}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];generated=BUILD/'generated'
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*['-I'+str(ROOT/n) for n in INCLUDES],'-I'+str(generated)]
 def sources(stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'bus.c',ROOT/('labs/peripheral_depth/dma/dma.c' if stub else 'assessor/peripheral_depth/dma/dma.c'),ROOT/'labs/architecture_bytes/placement.c',PUBLIC/'ledger_objects.c',PUBLIC/'tests.c']
 def compile(tag,cc,flags,src,binary):run(tag,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 def thumb(tag,flags=(),stub=False):
  image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections',*flags,*sources(stub),PUBLIC/'thumb_start.c',ROOT/'labs/containers_depth/byte_helpers.c','-T',PUBLIC/'model.ld','-Wl,--gc-sections,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);return image
 def execute(tag,image,bad=False):run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1 if bad else 0,'MB_THUMB_FAIL' if bad else 'MB_THUMB_PASS')
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('qemu.version',['qemu-system-arm','--version'])
  for mode in ['sram','ccm','bad']:
   tag='ledger.'+mode;image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.compile',['arm-none-eabi-gcc',*strict,'-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',ROOT/'labs/architecture_bytes/placement.c',PUBLIC/'ledger_objects.c',PUBLIC/'ledger_entry.c','-T',PUBLIC/('ledger_'+mode+'.ld'),'-Wl,--no-undefined,-Map,'+str(mapping),'-o',image]);harness.OUTPUTS.update([image,mapping]);run(tag+'.symbols',['arm-none-eabi-nm','-S',image]);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h
  run('linked.generate',[sys.executable,PRIVATE/'linked.py',BUILD/'ledger.sram.symbols.log',BUILD/'ledger.ccm.symbols.log',generated],marker='OBS actual typed linker ledger');harness.OUTPUTS.update(generated.iterdir());run('linked.negative',[sys.executable,PRIVATE/'linked.py',BUILD/'ledger.bad.symbols.log',BUILD/'ledger.ccm.symbols.log',BUILD/'bad-generated'],1,'MB_LINK_REJECT: DMA destination')
  for p,opt in [('debug','-O0'),('optimized','-O2')]:
   tag='thumb.'+p;image=thumb(tag,[opt]);execute(tag,image);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();h=run(tag+'.headers',['arm-none-eabi-readelf','-lW',image]);assert 'INTERP' not in h and 'DYNAMIC' not in h;run(tag+'.disassembly',['arm-none-eabi-objdump','-d',image])
  for m,(case,_,definition) in MUTANTS.items():tag='thumb.'+m;execute(tag,thumb(tag,['-O2','-D'+definition,'-DMB_TEST_CASE='+str(CASES.index(case))]),True)
  execute('thumb.stub',thumb('thumb.stub',['-O2'],True),True)
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for p,flags in PROFILES:
    tag=cc+'.'+p;binary=BUILD/tag;compile(tag+'.compile',cc,flags,[*sources(),PUBLIC/'native.c'],binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case);run(tag+'.'+case+'.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.'+case+'.log'),case],marker='"status": "pass"')
    run(tag+'.disassembly',['objdump','-d',binary]);stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[*sources(True),PUBLIC/'native.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'MB_ASSERT:')
    for m,(case,marker,definition) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-D'+definition],[*sources(),PUBLIC/'native.c'],mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    observations.append(dict(profile=tag,independent_packets=PACKETS,ordinary_sram_bytes=[196608,131072],physical=False,actual_provider='PdTransfer',ready_retains_lease=True,cpu_temporary_only_relocated=True))
   obj=BUILD/(cc+'.core.o');stack=obj.with_suffix('.su');run(cc+'.core',[cc,*strict,'-O2','-fstack-usage','-ffreestanding','-fno-builtin','-c',PRIVATE/'bus.c','-o',obj]);harness.OUTPUTS.update([obj,stack]);u=run(cc+'.core.undefined',['nm','-u',obj]);symbols=set(x.split()[-1] for x in u.splitlines());assert {'pd_validate','pd_submit','pd_poll','pd_finalize'}<=symbols and symbols<={'pd_validate','pd_submit','pd_poll','pd_finalize','memcpy','memset','_GLOBAL_OFFSET_TABLE_'};run(cc+'.core.stack',['cat',stack]);run(cc+'.core.sizes',['nm','-S','--size-sort',obj])
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped inputs changed during campaign'
 limits=['Exact three PR-T37-06 criterion/V1 mechanisms; PR07/08 remain separately required.','Actual frozen Pd state machine, B06 selected linker and prior typed objects are imported unchanged; this does not revalidate unrelated predecessor functions or complete all T29 work.','The six-port main-flash/no-remap data subgraph is a scoped selection, not the whole chip or permission for reserved APB addresses.','Real F439 silicon/board/clock/errata qualification and initialization remain unobserved; documentary family/manual selection is not a hardware identity check.','F439 ledger ELFs are audit-only, not bootable firmware; MPS2 executes the original model with tagged real backing storage and no F439 DMA/peripheral execution.','Round-robin burst waits and bridge words are explicit bounded models; no physical throughput, WCET, external review or independent delayed learner claim.']
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
