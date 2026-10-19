"""Cross-build original checkpoint firmware; execute only isolated Cortex-M QEMU guests."""
import argparse
from datetime import datetime,timezone
import hashlib,json,re,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def inspect(image,board):
    data=image.read_bytes();header=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
    if header[0][:6]!=b'\x7fELF\x01\x01' or header[2]!=40:raise ValueError('Expected ARM little-endian ELF32')
    symbols=subprocess.check_output(['arm-none-eabi-nm','-n',str(image)],text=True)
    values={name:int(address,16) for address,name in re.findall(r'^([0-9a-fA-F]+)\s+\S\s+(\S+)$',symbols,re.M)}
    required=('cp_reset','cp_vectors','cp_pendsv','__data_load','__data_start','__data_end','__bss_start','__bss_end','__stack_top','__dma_start','__dma_end')
    if any(name not in values for name in required):raise ValueError('Missing reset/layout boundary: '+repr([name for name in required if name not in values]))
    if values['cp_vectors']%512 or values['__stack_top']%8:raise ValueError('Vector/stack alignment')
    if header[4]!=(values['cp_reset']|1):raise ValueError('Thumb entry does not name original reset')
    undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(image)],text=True)
    if undefined.strip():raise ValueError('Unresolved target helper: '+undefined)
    if re.search(r'\b(?:malloc|free|printf|__atomic_\w+|__aeabi_(?!memclr4$)\w+|__stack_chk_fail)$',symbols,re.M):raise ValueError('Undeclared runtime/helper')
    if board and not (0x20000000<=values['__dma_start']<values['__dma_end']<=0x20020000):raise ValueError('DMA object outside selected F439 AHB SRAM')
    if values['__data_load']==values['__data_start'] or values['__bss_end']+8192>=values['__stack_top']:raise ValueError('Bad startup/stack separation')
    return {'symbols':values,'undefined_symbols':[], 'entry':hex(header[4]),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}

def run(report):
    base=ROOT/'reviews/v5/expansion/legacy_completion';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use new completion evidence')
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ');logs=base/'firmware_runs'/stamp;logs.mkdir(parents=True)
    sources=[Path(__file__).resolve(),ROOT/'labs/checkpoint_depth/include/checkpoint_depth.h',ROOT/'labs/checkpoint_depth/include/checkpoint_firmware.h',
             ROOT/'assessor/checkpoint_depth/checkpoint_depth.c',ROOT/'assessor/checkpoint_depth/checkpoint_firmware.c',
             *[ROOT/'assessor/checkpoint_depth/targets'/x for x in ('cortex_start.S','cortex_guest.c','f439.c','byte_helpers.c','arm_helpers.c')]]
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    result={'schema_version':1,'created_utc':stamp,'inspected_paths':pins,'profiles':[],'negative_controls':[],
            'scope':'Original reset/PendSV acquisition and P3 framing under QEMU MPS2-AN386. F439 images cross-linked/audited only. No host security changes, flashing, physical UART/DMA/timing or learner mastery claim.',
            'physical':'not-run','learner_mastery':'not-assessed','independent_review':'not-performed'}
    def command(argv,timeout=40,allow_failure=False):
        p=subprocess.run(list(map(str,argv)),capture_output=True,text=True,timeout=timeout)
        if p.returncode and not allow_failure:raise RuntimeError('Command failed '+repr(argv)+'\n'+p.stdout+p.stderr)
        return p
    try:
        result['tools']={tool:command([tool,'--version']).stdout.splitlines()[0] for tool in ('arm-none-eabi-gcc','clang','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm')}
        with tempfile.TemporaryDirectory(prefix='checkpoint-firmware-') as temporary:
            tmp=Path(temporary)
            def build(compiler,opt,offset,board=False,mutant=0,bad_dma=False):
                name=f'{compiler}-{opt}-{offset:x}-'+('f439' if board else 'mps2')+f'-m{mutant}'+('-ccm' if bad_dma else '')
                folder=tmp/name;folder.mkdir();flash=0x08000000 if board else 0;ram=0x20000000+offset
                link=folder/'image.ld';link.write_text(f'''ENTRY(cp_reset)
MEMORY {{ FLASH (rx) : ORIGIN = {flash:#x}, LENGTH = 512K
 RAM (rwx) : ORIGIN = {ram:#x}, LENGTH = 64K
 CCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K }}
__stack_top = ORIGIN(RAM)+LENGTH(RAM);
SECTIONS {{
 .text : {{ KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); }} > FLASH
 .data : {{ __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; }} > RAM AT> FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : {{ . = ALIGN(4); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; }} > RAM
 .dma (NOLOAD) : {{ . = ALIGN(4); __dma_start = .; *(.dma) . = ALIGN(4); __dma_end = .; }} > {'CCM' if bad_dma else 'RAM'}
 /DISCARD/ : {{ *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }}
 ASSERT(__bss_end+8192<__stack_top,"stack reserve")
}}
''',encoding='utf-8',newline='\n')
                cc=['arm-none-eabi-gcc'] if compiler=='gcc' else ['clang','--target=arm-none-eabi']
                flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-Wall','-Wextra','-Werror','-DCP_MUTANT='+str(mutant),'-I'+str(ROOT/'labs/checkpoint_depth/include')]
                files=[sources[3],sources[4],ROOT/'assessor/checkpoint_depth/targets/cortex_start.S',ROOT/'assessor/checkpoint_depth/targets'/('f439.c' if board else 'cortex_guest.c'),ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c',ROOT/'assessor/checkpoint_depth/targets/arm_helpers.c']
                objects=[];commands=[]
                for source in files:
                    obj=folder/(source.name+'.o');argv=[*cc,*flags,'-c',source,'-o',obj];command(argv);commands.append(list(map(str,argv)));objects.append(obj)
                image=folder/'image.elf';argv=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objects,'-T',link,'-Wl,-Map='+str(folder/'image.map'),'-o',image];command(argv);commands.append(list(map(str,argv)))
                mapping=(folder/'image.map').read_text(encoding='utf-8')
                if re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping):raise ValueError('Forbidden target runtime archive')
                for suffix,text in [('map',mapping),('dis',command(['arm-none-eabi-objdump','-d',image]).stdout),('symbols',command(['arm-none-eabi-nm','-n',image]).stdout)]:
                    (logs/(name+'.'+suffix)).write_text(text,encoding='utf-8',newline='\n')
                return name,image,commands
            def execute(name,image,audit,expected):
                values=audit['symbols'];argv=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image]
                # Poison VMA data too: ELF loading must not masquerade as reset copying.
                for section in ('data','bss'):
                    start,end=values[f'__{section}_start'],values[f'__{section}_end']
                    if end>start:
                        poison=image.parent/(section+'.bin');poison.write_bytes(b'\xa5'*(end-start));argv+=['-device',f'loader,file={poison},addr={start:#x},force-raw=on']
                observed=command(argv,allow_failure=True);output=observed.stdout+observed.stderr;(logs/(name+'.log')).write_text(output,encoding='utf-8',newline='\n')
                passed=observed.returncode==(0 if expected else 1) and ('CP_CORTEX_PASS' if expected else 'CP_CORTEX_FAIL') in output
                if not passed:raise RuntimeError('Unexpected guest outcome '+name+': '+output)
                return {'command':list(map(str,argv)),'exit_code':observed.returncode,'output':output,'passed':passed}
            for compiler,opt in [('gcc','O0'),('gcc','O2'),('clang','O2')]:
                for offset in (0,0x4000):
                    name,image,commands=build(compiler,opt,offset);audit=inspect(image,False)
                    result['profiles'].append({'name':name,'commands':commands,'audit':audit,'execution':execute(name,image,audit,True),'passed':True})
                for mutant in (33,34,41,42):
                    name,image,commands=build(compiler,opt,0,mutant=mutant);audit=inspect(image,False)
                    result['negative_controls'].append({'name':name,'commands':commands,'execution':execute(name,image,audit,False),'passed':True})
            for compiler in ('gcc','clang'):
                for offset in (0,0x4000):
                    name,image,commands=build(compiler,'O2',offset,board=True)
                    result['profiles'].append({'name':name,'commands':commands,'audit':inspect(image,True),'execution':'not-run: F439 hardware required','passed':True})
            name,image,commands=build('gcc','O2',0,board=True,bad_dma=True)
            try:inspect(image,True)
            except ValueError as error:
                if 'DMA object outside' not in str(error):raise
                result['negative_controls'].append({'name':name,'commands':commands,'reason':str(error),'passed':True})
            else:raise RuntimeError('CCM relocation accepted')
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:result['error']=str(error)
    result['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==v for p,v in pins.items())
    result['passed']='error' not in result and result['source_stable'] and len(result['profiles'])==10 and len(result['negative_controls'])==13
    result['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    text=json.dumps(result,indent=2)+'\n';(logs/'report.json').write_text(text,encoding='utf-8',newline='\n');destination.write_text(text,encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'profiles':len(result['profiles']),'negative_controls':len(result['negative_controls']),'error':result.get('error')}));return 0 if result['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);args=parser.parse_args();raise SystemExit(run(args.report))
