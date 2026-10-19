"""Build explicit selected DMA port link closures; never flash or execute MMIO."""
from pathlib import Path
import argparse,json,re,subprocess
def cache_stores(assembly):
    """Recover local constant-address stores in the selected emit function.
    Covers the observed literal, MOVW/MOVT and base+offset lowerings. This is
    an address audit, not a general path-sensitive machine-code verifier.
    """
    m=re.search(r'<emit>:\n(.*?)(?=\n[0-9a-f]+ <|\Z)',assembly,re.S)
    if not m:return []
    pool={int(a,16):int(v,16) for a,v in re.findall(r'^\s*([0-9a-f]+):.*?\.word\s+0x([0-9a-f]+)',assembly,re.M)}
    regs={};stores=[]
    for line in m[1].splitlines():
        move=re.search(r'\b(movw|movt|movs|mov\.w|mov)\s+(r\d+), #(\d+)',line)
        if move:
            op,reg,n=move.groups();n=int(n)
            if op=='movt':
                if reg in regs:regs[reg]=(regs[reg]&65535)|(n<<16)
            else:regs[reg]=n
        load=re.search(r'\bldr\s+(r\d+), \[pc,.*?\(\s*([0-9a-f]+)',line)
        if load:
            reg,address=load.groups();regs.pop(reg,None)
            if int(address,16) in pool:regs[reg]=pool[int(address,16)]
        otherload=re.search(r'\bldr\s+(r\d+), \[(?!pc)',line)
        if otherload:regs.pop(otherload[1],None)
        store=re.search(r'\bstr(?:\.w)?\s+r\d+, \[(r\d+)(?:, #(\d+))?\]',line)
        if store and store[1] in regs:
            address=regs[store[1]]+int(store[2] or 0)
            stores.append(dict(address=hex(address),instruction=line.strip()))
    return stores
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--board',choices=['F439','H745'],required=True);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0')
for key in ['port','dma','cache','runtime']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--cache-port',type=Path,default=LABS/'firmware_lifecycle/targets/h745_port.c')
a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);h=a.board=='H745'
flags=['-std=c17','-O'+a.opt,'-g','-mcpu='+('cortex-m7' if h else 'cortex-m4'),'-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-DPD_BOARD='+str(int(h)),'-I'+str(LABS/'peripheral_depth/dma'),'-I'+str(LABS/'firmware_lifecycle/targets')]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
sources=[a.port,a.dma,a.runtime]+([a.cache,a.cache_port] if h else [])
objects=[]
for i,s in enumerate(sources):
    obj=a.build/(str(i)+'.o');objects.append(obj);subprocess.run([a.cc,*flags,'-c',str(s),'-o',str(obj)],check=True);subprocess.run(['arm-none-eabi-nm','-u',str(obj)],check=True)
elf=a.build/'port.elf'
subprocess.run(['arm-none-eabi-ld','--gc-sections','-e','pd_target_io','-Ttext=0x08000000',*map(str,objects),'-Map='+str(a.build/'port.map'),'-o',str(elf)],check=True)
assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
asm=subprocess.check_output(['arm-none-eabi-objdump','-drwC',str(elf)],text=True);(a.build/'port.asm').write_text(asm)
assert 'dsb\tsy' in asm
assert ('fw_h745_cache' in asm)==h
stores=cache_stores(asm);addresses={int(s['address'],16) for s in stores}
assert addresses==({0xe000ef68,0xe000ef5c} if h else set()),stores
(a.build/'store_audit.json').write_text(json.dumps(dict(board=a.board,stores=stores,limit='Local constant-address audit in selected cache emitter; not general path-sensitive proof.'),indent=2)+'\n')
print('XD_BOARD_BUILD_PASS '+a.board+' object/link only; no vector image or physical execution')
