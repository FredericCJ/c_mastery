"""Strict baseline64KiB M4, explicit original core/probe/runtime; optional GCC LTO."""
from pathlib import Path
import argparse,hashlib,json,shlex,shutil,subprocess
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True)
for key in ['cvm','probe','runtime','driver','headers']:p.add_argument('--'+key,type=Path,required=True)
p.add_argument('--poison-bss',action='store_true');p.add_argument('--skip-bss',action='store_true');p.add_argument('--unrooted-lto-control',action='store_true');p.add_argument('--helper',type=Path);p.add_argument('--cc',choices=['arm-none-eabi-gcc','clang'],default='arm-none-eabi-gcc');p.add_argument('--opt',choices=['0','3'],default='0');p.add_argument('--lto',action='store_true');a=p.parse_args();a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=True)
if a.skip_bss and not a.poison_bss:p.error('skip-bss control requires poison-bss')
if a.unrooted_lto_control and not a.lto:p.error('unrooted-lto-control requires LTO')
if a.lto and a.cc!='arm-none-eabi-gcc':p.error('this explicit LTO profile requires GCC; no fallback')
flags=['-std=c17','-O'+a.opt,'-Wall','-Wextra','-Werror','-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-ffunction-sections','-fdata-sections','-I'+str(HERE),'-I'+str(LABS/'runtime/include'),'-I'+str(LABS/'kernel_followthrough/runtime'),'-isystem',str(a.headers)]
if a.cc=='clang':flags+=['--target=arm-none-eabi']
if a.lto:flags+=['-flto']
if a.poison_bss:flags+=['-DVX_POISON_BSS']
if a.skip_bss:flags+=['-DVX_SKIP_BSS']
sources=[a.cvm,a.probe,HERE/'driver.c',a.runtime,a.driver,HERE/'startup.S']+([a.helper] if a.helper else []);objects=[];imports={};defined=set()
for i,s in enumerate(sources):
 obj=a.build/(str(i)+'.o');objects.append(obj);cmd=[a.cc,*flags,*(['-Dmain=vx_public_tests'] if i==2 else []),*([] if s.suffix=='.S' else ['-Wa,--noexecstack']),'-MD','-MF',str(a.build/(str(i)+'.d')),'-c',str(s),'-o',str(obj)];subprocess.run(cmd,check=True)
 imports[str(i)]=subprocess.check_output(['arm-none-eabi-nm','-u',str(obj)],text=True).splitlines();symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(obj)],text=True);defined|={r.split()[-1] for r in symbols.splitlines() if len(r.split())>=3}
undefined={r.split()[-1] for rows in imports.values() for r in rows};script={'__stack_top','__data_load','__data_start','__data_end','__bss_start','__bss_end'};unexplained=sorted(undefined-defined-script)
(a.build/'object-audit.json').write_text(json.dumps(dict(imports=imports,defined=sorted(defined),linker_symbols=sorted(script),unexplained=unexplained),indent=2)+'\n')
if unexplained:print('VX_UNEXPLAINED_IMPORT '+','.join(unexplained));raise SystemExit(3)
elf=a.build/'image.elf';link=['--gc-sections','-T',str(HERE/'link.ld'),*map(str,objects),'-Map='+str(a.build/'image.map'),'-o',str(elf)]
if a.lto:cmd=[a.cc,*flags,'-nostdlib','-nostartfiles','-nodefaultlibs','-save-temps','-Wl,--gc-sections',* ([] if a.unrooted_lto_control else ['-Wl,--undefined=memcpy,--undefined=memset']),'-Wl,-T,'+str(HERE/'link.ld'),*map(str,objects),'-Wl,-Map,'+str(a.build/'image.map'),'-o',str(elf)]
else:cmd=['arm-none-eabi-ld',*link]
subprocess.run(cmd,check=True);assert not subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True).strip()
deps={HERE/'link.ld'}
for f in a.build.glob('*.d'):deps.update(Path(n) for n in shlex.split(f.read_text().replace('\\\n',' ').split(':',1)[1]))
deps.update(Path(shutil.which(t)) for t in [a.cc,'arm-none-eabi-ld','arm-none-eabi-nm']);(a.build/'external-inputs.json').write_text(json.dumps({str(p.resolve()):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(deps)},indent=2)+'\n');print('VX_TARGET_BUILD_PASS explicit_objects=6 compiler_excluded lto='+str(a.lto))
