"""Execute exact capstone firmware models and original Cortex-M4 guest controls."""
import datetime,hashlib,json,pathlib,re,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_capstone_firmware';PUBLIC=ROOT/'labs/capstone_followthrough/firmware';PRIVATE=ROOT/'assessor/capstone_followthrough/firmware';BUILD=ROOT/'build'/('capstone-firmware-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
INCLUDES=['labs/capstone_followthrough/firmware','labs/capstone_followthrough/core','labs/checkpoint_depth/include','labs/peripheral_depth/dma','labs/interrupt_depth/frame']
SOURCES=['assessor/capstone_followthrough/firmware/firmware.c','assessor/capstone_followthrough/core/service.c','assessor/checkpoint_depth/checkpoint_depth.c','assessor/checkpoint_depth/checkpoint_firmware.c','assessor/peripheral_depth/dma/dma.c','assessor/interrupt_depth/frame/frame.c']
PROFILES=[(cc,opt,f) for cc in ['gcc','clang'] for opt,f in [('O0',['-O0']),('O3',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]]
NEWLIB=pathlib.Path('/home/frede/cmastery-freestanding-tools/root/usr/include/newlib')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
    files={pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in SOURCES],ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c',ROOT/'assessor/checkpoint_depth/targets/arm_helpers.c',ROOT/'assessor/checkpoint_depth/targets/f439.c',ROOT/'assessor/checkpoint_depth/targets/cortex_start.S',ROOT/'assessor/peripheral_depth/dma/target.c',ROOT/'assessor/errors/errors.c',ROOT/'labs/errors/include/errors.h'}
    for tree in [PUBLIC,PRIVATE,*[ROOT/n for n in INCLUDES[1:]]]:files|={p for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-size','arm-none-eabi-objdump','arm-none-eabi-objcopy','qemu-system-arm']}|{p for p in NEWLIB.rglob('*') if p.is_file()})}
def release(run,folder,other):
    t=folder.name;image=folder/'image.elf';binary=folder/'image.bin';raw=binary.read_bytes()
    manifest={k:dict(path=str(p),sha256=sha(p)) for k,p in [('source',ROOT/SOURCES[1]),('elf',image),('binary',binary),('map',folder/'image.map')]};manifest.update(vector=int.from_bytes(raw[4:8],'little'),flags=['c17','cortex-m4','thumb','soft-float','nostdlib','nodefaultlibs','nostartfiles'],physical_observation='not-run');(folder/'release.json').write_text(json.dumps(manifest,indent=2)+'\n')
    symbols=folder/'symbols.elf';stripped=folder/'stripped.elf';run(t+'.debug.keep',['arm-none-eabi-objcopy','--only-keep-debug',image,symbols]);run(t+'.debug.strip',['arm-none-eabi-objcopy','--strip-all',image,stripped]);run(t+'.debug.link',['arm-none-eabi-objcopy','--add-gnu-debuglink='+str(symbols),stripped]);run(t+'.debug.binary',['arm-none-eabi-objcopy','-O','binary',stripped,folder/'stripped.bin']);assert (folder/'stripped.bin').read_bytes()==raw;(folder/'debug.json').write_text(json.dumps(dict(stripped_sha256=sha(stripped),symbols_sha256=sha(symbols)),indent=2)+'\n');run(t+'.release',[sys.executable,PRIVATE/'evidence_tests.py',folder,other/'image.elf'],marker='CAP_MCU_EVIDENCE_PASS')

def expected_commands():
    out=set()
    for cc,opt,_ in PROFILES:
        t=cc+'.'+opt
        for name,cases in [('reference',range(8)),('authority',[0]),('lease',[1]),('watchdog',[2]),('trust',[3]),('status',[7]),('stub',[0])]:
            out.add(t+'.'+name+'.build');out.update(t+'.'+name+'.'+str(c) for c in cases)
        out.update(t+'.'+n for n in ['transfer.build','transfer','representation.build','representation'])
    for cc in ['gcc','clang']:
        out.update(cc+'.'+n for n in ['result-default-reject','result-header','result-full.build','result-full.undefined'])
        for opt in ['O0','O3']:
            for v in ['normal','reorder','copy','bss','vector','mpu','f439']:
                t=cc+'.'+opt+'.'+v;out.update(t+'.compile.'+str(i) for i in range(12));out.update(t+'.'+n for n in ['link','symbols','undefined','size','assembly','binary'])
                if v not in ['vector','f439']:out.add(t+'.run')
    for t in ['gcc.O0.normal','gcc.O0.f439']:out.update(t+'.'+n for n in ['debug.keep','debug.strip','debug.link','debug.binary','release'])
    return out
def target(run,cc,opt,variant):
    t=cc+'.'+opt+'.'+variant;folder=BUILD/t;folder.mkdir();link=folder/'image.ld'
    order='*(.data.special) *(.data) *(.data.user*)' if variant!='reorder' else '*(.data) *(.data.user*) . = ALIGN(32); *(.data.special)'
    script='''ENTRY(cf_reset)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 512K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : { __data_start = .; ORDER KEEP(*(.data.tail)) . = ALIGN(4); __data_end = .; } > RAM AT> FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(32); __bss_start = .; *(.bss*) *(.dma*) *(COMMON) . = ALIGN(4); __bss_end = .; } > RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__bss_end + 16384 < __stack_top,"verification-stack reserve")
}
'''.replace('ORDER',order)
    if variant=='f439':script=script.replace('ENTRY(cf_reset)','ENTRY(cp_reset)').replace('ORIGIN = 0, LENGTH','ORIGIN = 0x08000000, LENGTH').replace(order,'*(.data*)')
    link.write_text(script)
    macros={'copy':['-DCF_BAD_COPY'],'bss':['-DCF_NO_BSS'],'vector':['-DCF_WRONG_VECTOR'],'mpu':['-DCF_NO_MPU']}.get(variant,[])
    compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-fstack-usage',*macros,*['-I'+str(ROOT/n) for n in INCLUDES]]
    sources=[*[ROOT/n for n in SOURCES],PUBLIC/'checks.c',PRIVATE/'target.c',PUBLIC/'startup.S',ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c',ROOT/'assessor/checkpoint_depth/targets/arm_helpers.c',ROOT/'labs/capstone_followthrough/core/target_helpers.c'];objects=[]
    if variant=='f439':sources=[*[ROOT/n for n in SOURCES],PRIVATE/'f439_adapter.c',ROOT/'assessor/peripheral_depth/dma/target.c',ROOT/'assessor/checkpoint_depth/targets/cortex_start.S',*sources[-3:]]
    for i,source in enumerate(sources):obj=folder/(str(i)+'.o');run(t+'.compile.'+str(i),[*compiler,*flags,'-c',source,'-o',obj]);objects.append(obj)
    frames=[]
    for p in folder.glob('*.su'):
        for line in p.read_text().splitlines():
            fields=line.split('\t');assert len(fields)>=3 and fields[-1]=='static',('unbounded stack',line);frames.append(dict(function=fields[0],bytes=int(fields[-2])))
    assert frames
    # Original selected functions have no recursion; fixed callback targets are
    # bound above. Three complete copies deliberately overbound foreground plus
    # two distinct nested handlers, including repeated functions across contexts.
    required_stack=3*sum(q['bytes'] for q in frames)+2*36+128
    link.write_text(link.read_text().replace('16384',str(required_stack)))
    image=folder/'image.elf';run(t+'.link',['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',link,'-Wl,--gc-sections,-Map='+str(folder/'image.map'),'-o',image]);symbols=run(t+'.symbols',['arm-none-eabi-nm','-n',image],marker=' cp_target_main' if variant=='f439' else ' cf_target_main');undefined=run(t+'.undefined',['arm-none-eabi-nm','-u',image]);assert not undefined.strip();mapping=(folder/'image.map').read_text();assert not re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping)
    run(t+'.size',['arm-none-eabi-size','-A',image],marker='.text');run(t+'.assembly',['arm-none-eabi-objdump','-d',image],marker='<cp_reset>:' if variant=='f439' else '<cf_reset>:');binary=folder/'image.bin';run(t+'.binary',['arm-none-eabi-objcopy','-O','binary',image,binary]);raw=binary.read_bytes();stack=int.from_bytes(raw[:4],'little');vector=int.from_bytes(raw[4:8],'little');assert stack==0x20010000 and stack%8==0
    values={name:int(address,16) for address,name in re.findall(r'^([0-9a-fA-F]+)\s+\S\s+(\S+)$',symbols,re.M)};static=values['__bss_end']-0x20000000;actual=static+required_stack
    (folder/'budget.json').write_text(json.dumps(dict(static_ram_including_padding=static,static_frames=frames,conservative_stack=required_stack,total=actual,admit4096=actual<=4096,admit2048=actual<=2048,admit_selected64KiB=actual<65536,fp_two_extended_delta=144,limitations=['Acyclic selected callbacks and at most two distinct nested handlers; no recursive reentry.','Sum of every compiled frame, including discarded functions, is a deliberately loose upper account.','No physical stack watermark or timing bound.']),indent=2)+'\n');assert actual<65536
    if variant=='f439':assert vector&1 and 0x08000000<=(vector&~1)<0x08000000+len(raw) and ' cap_board' in symbols and ' pd_target_io' in symbols;return
    if variant=='vector':assert vector==0;return
    assert vector&1 and (vector&~1)<len(raw)
    command=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image]
    for section in ['data','bss']:
        start,end=values['__'+section+'_start'],values['__'+section+'_end'];poison=folder/(section+'.bin');poison.write_bytes(b'\xa5'*(end-start));command+=['-device',f'loader,file={poison},addr={start:#x},force-raw=on']
    run(t+'.run',command,41 if variant in ['copy','bss','mpu'] else 0,'CAP_MCU_MPU_REJECT' if variant=='mpu' else 'CAP_MCU_STARTUP_REJECT' if variant in ['copy','bss'] else 'CAP_MCU_PASS')
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();before=pins();ext=external();failure=None;run=h.run
    try:
        for cc,opt,flags in PROFILES:
            t=cc+'.'+opt;common=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,*['-I'+str(ROOT/n) for n in INCLUDES]]
            for name,macro,source,cases in [('reference',[],ROOT/SOURCES[0],range(8)),('authority',['-DCF_MUTANT=1'],ROOT/SOURCES[0],[0]),('lease',['-DCF_MUTANT=2'],ROOT/SOURCES[0],[1]),('watchdog',['-DCF_MUTANT=3'],ROOT/SOURCES[0],[2]),('trust',['-DCF_MUTANT=4'],ROOT/SOURCES[0],[3]),('status',['-DCF_MUTANT=5'],ROOT/SOURCES[0],[7]),('stub',[],PUBLIC/'learner.c',[0])]:
                exe=BUILD/(t+'.'+name);run(t+'.'+name+'.build',[*common,*macro,source,*[ROOT/n for n in SOURCES[1:]],PUBLIC/'checks.c',PUBLIC/'tests.c','-o',exe])
                for case in cases:run(t+'.'+name+'.'+str(case),[exe,str(case)],0 if name=='reference' else 41,'CAP_FIRMWARE_PASS' if name=='reference' else 'CAP_FIRMWARE_ASSERT')
            exe=BUILD/(t+'.transfer');run(t+'.transfer.build',[*common,*[ROOT/n for n in SOURCES],PRIVATE/'transfer.c','-o',exe]);run(t+'.transfer',[exe],marker='CAP_MCU_TRANSFER_PASS')
            exe=BUILD/(t+'.representation');run(t+'.representation.build',[*common,'-Ilabs/errors/include','-ffunction-sections','-fdata-sections',ROOT/'assessor/errors/errors.c',PUBLIC/'representation.c','-Wl,--gc-sections','-o',exe]);run(t+'.representation',[exe],marker='CAP_MCU_RESULT')
        for cc in ['gcc','clang']:
            compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];probe=BUILD/(cc+'.result-header.c');probe.write_text('#include "errors.h"\nunsigned result_size(void){return sizeof(er_result);}\n');flags=['-std=c17','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-I'+str(ROOT/'labs/errors/include')]
            run(cc+'.result-default-reject',[*compiler,*flags,'-M',probe],1,'setjmp.h');flags+=['-isystem',NEWLIB];run(cc+'.result-header',[*compiler,*flags,'-M',probe],marker='setjmp.h');obj=BUILD/(cc+'.result-full.o');run(cc+'.result-full.build',[*compiler,*flags,'-O0','-c',ROOT/'assessor/errors/errors.c','-o',obj]);run(cc+'.result-full.undefined',['arm-none-eabi-nm','-u',obj],marker='longjmp')
            for opt in ['O0','O3']:
                for variant in ['normal','reorder','copy','bss','vector','mpu','f439']:target(run,cc,opt,variant)
        release(run,BUILD/'gcc.O0.normal',BUILD/'gcc.O0.reorder');release(run,BUILD/'gcc.O0.f439',BUILD/'gcc.O3.f439')
        assert {q['id'] for q in h.COMMANDS}==expected_commands() and len(h.COMMANDS)==len(expected_commands())
    except Exception as ex:failure=str(ex)
    if before!=pins() or ext!=external():failure='Scoped source/tool identity changed'
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,commands=h.COMMANDS,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},limits=['Original models and MPS2 struct lr_cpu execution; no physical F439 or DMA bus execution is claimed.','A model signer verdict is trusted input, not an implemented signature verifier.','Actual MPU experiment covers struct lr_cpu access only; DMA/debug/NMI authority remains separate.','Finite tests and author reference variants are not independent learner or external acceptance.']);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
