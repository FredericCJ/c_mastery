"""Concrete newlib-nano source/object dependency experiment, separately profiled."""
from pathlib import Path
import hashlib,json,re
def run(c,root,args):
 c.group='freestanding-library';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';prefix=Path('/home/frede/cmastery-freestanding-tools');headers=prefix/'root/usr/include/newlib';library=prefix/'root/usr/lib/arm-none-eabi/newlib/thumb/v7e-m/nofp/libc_nano.a'
 c.require(library.is_file(),'selected concrete embedded library exists',str(library))
 flags=['-std=c17','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-I'+str(pub),'-isystem',headers,'-T',pub/'m4.ld'];sources=[pub/'m4_start.S',private/'newlib_target.c']
 missing=c.run(['arm-none-eabi-gcc','-O2',*flags,'-DOMIT_HOOKS',*sources,library,'-o',c.root/'missing-hooks'],expected='nonzero',contains='undefined reference')
 hooks=sorted(set(re.findall(r"undefined reference to [`']([^'`]+)",missing)));c.require({'_write','_sbrk','_read','_lseek','_close','_fstat','_isatty'}<=set(hooks),'actual formatted-output environment dependencies are visible',hooks)
 manifests=[]
 for opt in ('O0','O2'):
  image=c.root/('newlib-'+opt);mapfile=c.root/(image.name+'.map');c.run(['arm-none-eabi-gcc','-'+opt,*flags,*sources,library,'-Wl,-Map='+str(mapfile),'-o',image]);undefined=c.run(['arm-none-eabi-nm','-u',image]);c.require(not undefined.strip(),'permitted-library image closes every observed hook',undefined);c.run(['arm-none-eabi-size','-A',image]);c.run(['arm-none-eabi-objdump','-dr',image]);symbols=c.run(['arm-none-eabi-nm','-a',image]);mapping=mapfile.read_text();c.require('libc_nano.a' in mapping and 'libgcc' not in mapping,'newlib profile records explicit library and excludes undeclared compiler runtime')
  c.run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','null','-semihosting-config','enable=on,target=native','-kernel',image],contains='NEWLIB-PASS:',timeout=20)
  manifests.append(dict(profile=opt,binary_sha256=hashlib.sha256(image.read_bytes()).hexdigest(),map_sha256=hashlib.sha256(mapfile.read_bytes()).hexdigest(),library_sha256=hashlib.sha256(library.read_bytes()).hexdigest(),observed_missing_hooks=hooks,bytes=image.stat().st_size,explicit_runtime='newlib-nano; semihosting diagnostics and exit; original refusing hooks; not strict profile'))
 c.write('library/manifests.json',json.dumps(manifests,indent=2))
 c.observe('T11.07.a','Pinned newlib-nano4.6.0.20260123 tiny printf requires seven concrete environment hooks; partial-write and refusal behavior executes in M4 guest',['freestanding-library'])
 c.observe('T11.07.a','Caller-owned raw24-byte service uses no stdio/reentrancy/heap hooks; its explicit output and readiness obligations remain',['freestanding-library','freestanding-services'],True)
