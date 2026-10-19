"""Run original bounded initializer models and strict G474 image audits."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,struct,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/g474_uart_followthrough';REF=ROOT/'assessor/g474_uart_followthrough';BASE=ROOT/'labs/peripheral_depth/layers';TRANSFER=ROOT/'assessor/peripheral_depth/layers/compare.c';RUNTIME=ROOT/'assessor/execution/support.c'
REPORT='reviews/v5/expansion/legacy_g474_uart/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('g4_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():
 files={Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',TRANSFER,RUNTIME}
 for base in [LAB,REF]:files.update(p for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 files.update(BASE/n for n in ['compare.h','service.h','registers.h','registers.json','generate_registers.py'])
 return sorted(files)
def audit_elf(data):
 if data[:7]!=b'\x7fELF\x01\x01\x01':return False
 h=struct.unpack_from('<16sHHIIIIIHHHHHH',data);sections=[struct.unpack_from('<10I',data,h[6]+i*h[11]) for i in range(h[12])];names=sections[h[13]];strings=data[names[4]:names[4]+names[5]];v=next(s for s in sections if strings[s[0]:].split(b'\0',1)[0]==b'.vectors');words=struct.unpack_from('<16I',data,v[4]);return v[3]==0x08000000 and v[5]==64 and words[0]==0x20004000 and words[1]&1 and words[1]==h[4] and all(words[i]&1 for i in [2,3,4,5,6,11,12,14,15]) and all(words[i]==0 for i in [7,8,9,10,13])
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/g474-uart-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;deps={};images=[]
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-size']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  c.run(['uname','-a'])
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB),'-I'+str(BASE)]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for name,test,mut,code,marker in [('public',LAB/'tests.c',0,0,'G4_PASS'),('model',REF/'model.c',0,0,'G4_MODEL_PASS cases=212 transfer=3 probe=4')]+[(f'mutant-{n}',REF/'model.c',n,1,'G4_MODEL_CHECK:') for n in range(1,5)]+[('stub',LAB/'tests.c',-1,1,'G4_CHECK:')]:
    c.group=tag+':'+name;exe=w/(tag+'-'+name);source=LAB/'learner.c' if mut==-1 else REF/'port.c';c.run([cc,*flags,'-DG4_MUTANT='+str(max(mut,0)),source,LAB/'clock_probe.c',TRANSFER,test,'-o',exe]);text=c.run([exe],expected=code,contains=marker,env=env);c.require('runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,'defined model/control execution')
  for name,source in [('baud',REF/'baud_oracle.py'),('capture',LAB/'check_capture.py')]:
   c.group=name;text=c.run([sys.executable,source]+(['--self-test'] if name=='capture' else []),contains='"passed": true');d=json.loads(text);c.require(d['passed'],'independent '+name);c.write(name+'.json',json.dumps(d,indent=2)+'\n')
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['0','3']:
    for baud in [9600,115200,1000000]:
     name=f'{cc}-O{opt}-{baud}';dest=w/name;c.group='board:'+name;c.run([sys.executable,LAB/'target_build.py','--build',dest,'--initializer',REF/'port.c','--driver',REF/'target.c','--transfer',TRANSFER,'--transfer-include',BASE,'--runtime',RUNTIME,'--cc',cc,'--opt',opt,'--baud',str(baud)],contains='G4_BUILD_PASS objects=7 archives=0 physical=not-run')
     u=c.run(['arm-none-eabi-nm','-u',dest/'image.elf']);c.require(not u.strip(),'no unresolved helpers');listing=c.run(['arm-none-eabi-objdump','-dr',dest/'image.elf']);c.write(name+'/disassembly.txt',listing);c.run(['arm-none-eabi-size','-A',dest/'image.elf']);symbols=c.run(['arm-none-eabi-nm',dest/'image.elf']);c.require(all(s in symbols for s in ['g4_initialize','g4_physical_io','pl_byte','g4_capture','g4_release_tx']),'actual integrated original symbols')
     objects=sorted(dest.glob('*.o'));c.write(name+'/object-unresolved.txt',c.run(['arm-none-eabi-nm','-u',*objects]));c.write(name+'/object-disassembly.txt',c.run(['arm-none-eabi-objdump','-dr',*objects]));c.require(len(objects)==7,'actual seven input objects audited')
     d=json.loads((dest/'build.json').read_text());deps.update(d['input_sha256']);mapping=(dest/'image.map').read_text();c.require(d['objects']==7 and d['archives']==0 and '.a(' not in mapping and 'libgcc' not in mapping and 'libc.' not in mapping,'explicit object runtime closure');c.require(audit_elf((dest/'image.elf').read_bytes()),'actual reset/vector/RAM layout');c.require(len(list(dest.glob('*.su')))>=5,'stack usage produced');images.append(dict(name=name,path=str(dest),sha256=sha(dest/'image.elf'),baud=baud,execution='not-run'))
  c.group='bad-vector';bad=bytearray((Path(images[0]['path'])/'image.elf').read_bytes());h=struct.unpack_from('<16sHHIIIIIHHHHHH',bad);sections=[struct.unpack_from('<10I',bad,h[6]+i*h[11]) for i in range(h[12])];names=sections[h[13]];strings=bad[names[4]:names[4]+names[5]];v=next(s for s in sections if strings[s[0]:].split(b'\0',1)[0]==b'.vectors');struct.pack_into('<I',bad,v[4]+4,0);(w/'bad-vector.elf').write_bytes(bad);c.require(not audit_elf(bad),'wrong actual entry rejected')
  provision=json.loads((ROOT/'research/v5/expansion/legacy_advanced_layers/g4_provisioning.json').read_text());device=Path(provision[1]['path']);cmsis=Path('/home/frede/cmastery-architecture-tools/CMSIS_5-5.9.0/CMSIS/Core/Include')
  for cc in ['arm-none-eabi-gcc','clang']:
   for bad in [0,1]:
    c.group=cc+':layout-'+str(bad);dep=w/(cc+str(bad)+'.d');cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-DSTM32G474xx','-DG4_OLD_MAP='+str(bad),'-I'+str(device/'Include'),'-I'+str(cmsis),'-MD','-MF',dep,'-c',REF/'layout.c','-o',w/(cc+str(bad)+'.o')]+(['--target=arm-none-eabi'] if cc=='clang' else []);c.run(cmd,expected=1 if bad else 0,contains='status mapping' if bad else None)
    if not bad:
     import shlex
     for name in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):deps[name]=sha(Path(name))
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(board_images=images,dependency_sha256=deps,external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(native_profiles=6,models_per_profile=212,transfer_cases=3,probe_cases=4,compiling_mutants=4,compiling_stubs=1,board_images=12,layout_compilers=2,negative_translation=2),limits=d['limits']+['Register model is not G474 peripheral emulation; board images were not executed.','Vendor headers compile only for independent layout assertions; no vendor code linked into target images.','Original runtime helpers are explicit objects; no libc/libgcc/compiler-rt/HAL/libatomic archives.','Capture controls are synthetic consistency checks, not authentic physical records.','Poll counts and synthetic transfer ticks are bounds, not elapsed-time measurements.','RM0440 full text remains inaccessible; selected claims use pinned primary device/LL sources, datasheet, board manual and errata.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
