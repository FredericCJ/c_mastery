"""Actual staged facilities, compiler change, resource rebuild and target closure."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys,os
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/facilities_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/facilities';private=ROOT/'assessor/language_portability/facilities/facilities.c';oldroot=ROOT/'build/portability-compiler20/root';old=oldroot/'usr/bin/clang-20'
 paths=['tools/run_portability_facilities_companion.py','tools/verify_portability_facilities.py','tools/common.py','fixtures/language/PR-T05-04.json',private.relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]+[p.relative_to(ROOT).as_posix() for p in oldroot.parent.glob('*.deb')]+['build/portability-compiler20/download.log']
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[];embeds=[];targets=[];upgrades=[];observations=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('facilities-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 oldenv=dict(os.environ);oldenv['LD_LIBRARY_PATH']=str(oldroot/'usr/lib/x86_64-linux-gnu')
 def bind(q):
  q=Path(q).resolve()
  if q.is_relative_to(runs):return
  if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
  else:external.setdefault(str(q),sha(q))
 def execute(cmd,label,expect=0,markers=(),env=None):
  cmd=list(map(str,cmd));dep=None
  if any(x in Path(cmd[0]).name for x in ['gcc','clang']) and '-c' in cmd and expect==0:
   dep=runs/(f'{len(records):04d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);out=p.stdout+p.stderr;log=runs/(f'{len(records):04d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expect,markers=list(markers),environment_overrides={'LD_LIBRARY_PATH':oldenv['LD_LIBRARY_PATH']} if env else {},log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==expect if expect is not None else p.returncode!=0) and all(m in out for m in markers),out
  if expect is None:assert 'error:' in out or 'undefined reference' in out
  if dep:
   deps=[Path(x).resolve() for x in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in deps]));[bind(q) for q in deps]
  return out
 def linked(exe,label,env=None):
  out=execute(['ldd',exe],label,env=env)
  for token in out.split():
   q=Path(token)
   if q.is_absolute() and q.is_file():bind(q)
 def native(cc,opt,home,enhanced):
  flags=[opt,'-std='+('c23' if enhanced else 'c17'),'-pedantic-errors','-Wall','-Wextra','-Werror','-I'+str(public)]+(['-DLF_ENHANCED'] if enhanced else [])
  objs=[]
  for i,src in enumerate([private,public/'tests.c']):
   obj=home/(str(i)+'.o');execute([cc,*flags,'-c',src,'-o',obj],'native-compile');objs.append(obj)
  exe=home/'test';execute([cc,*objs,'-o',exe],'native-link')
  for anchor in ['add','mixed','bits','divide','select']:execute([exe,anchor],'native-'+anchor,markers=['LP '+anchor+' validated'])
  out=execute([sys.executable,ROOT/'tools/verify_portability_facilities.py','--driver',exe],'native-oracle');observations.append(dict(compiler=cc,optimization=opt,enhanced=enhanced,result=json.loads(out)))
  execute(['nm','-u',objs[0]],'native-undefined');execute(['objdump','-dr',objs[0]],'native-assembly');execute(['size',exe],'native-size');linked(exe,'native-runtime')
 for cc in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','arm-none-eabi-ld','qemu-system-arm','readelf','nm','objdump','size','ld']:
  tool=Path(shutil.which(cc)).resolve();bind(tool);execute([cc,'--version'],cc+'-version')
 linker=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21';bind(linker);execute([linker,'--version'],'riscv-linker-version');bind(Path(sys.executable).resolve());bind(old);execute([old,'--version'],'old-compiler-version',markers=['20.1.8'],env=oldenv);linked(old,'old-compiler-runtime',oldenv)
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records);flags=[opt,'-std=c23','-Wall','-Wextra','-pedantic-errors']
   execute([cc,*flags,'-DLF_PROBE=4','-E','-dM',public/'probes.c'],'macro-observation',markers=['__STDC_VERSION__ 202311L'])
   for probe in [1,2,3,4,5,6,7,9]:
    f=[*flags,'-DLF_PROBE='+str(probe)];execute([cc,*f,'-E',public/'probes.c'],'header-preprocess');execute([cc,*f,'-fsyntax-only',public/'probes.c'],'language-only');obj=home/(str(probe)+'.o');exe=home/str(probe);execute([cc,*f,'-c',public/'probes.c','-o',obj],'feature-object');execute([cc,obj,'-o',exe],'feature-link');execute([exe],'feature-run')
   for probe in [4,5,9]:
    obj=home/('c17-'+str(probe)+'.o');exe=home/('c17-'+str(probe));execute([cc,opt,'-std=c17','-pedantic-errors','-DLF_PROBE='+str(probe),'-c',public/'probes.c','-o',obj],'c17-spelling');execute([cc,obj,'-o',exe],'c17-link');execute([exe],'c17-run')
   execute([cc,'-std=c17','-pedantic-errors','-DLF_PROBE=3','-fsyntax-only',public/'probes.c'],'missing-compatibility-headers',None)
   for mode in ['c17','c23']:execute([cc,opt,'-std='+mode,'-pedantic-errors','-DLF_PROBE=8','-fsyntax-only',public/'probes.c'],'strict-extension-reject',None)
   for mode in ['gnu17','gnu23']:
    obj=home/(mode+'.o');exe=home/mode;execute([cc,opt,'-std='+mode,'-Wall','-Wextra','-DLF_PROBE=8','-c',public/'probes.c','-o',obj],'extension-object');execute([cc,obj,'-o',exe],'extension-link');execute([exe],'extension-run')
   execute([cc,'-std=c23','-nostdinc','-E',public/'missing_header.c'],'advertised-mode-withheld-header',None,markers=['stdckdint.h'])
   for enhanced in [False,True]:
    d=home/('enhanced' if enhanced else 'baseline');d.mkdir();native(cc,opt,d,enhanced)
   d=home/'embedded';d.mkdir();shutil.copyfile(public/'embed.c',d/'embed.c');shutil.copyfile(public/'resource.bin',d/'resource.bin');source_sha=sha(d/'embed.c');compiler=Path(shutil.which(cc)).resolve();events=[]
   for step in range(3):
    if step==2:(d/'resource.bin').write_bytes(bytes([0,127,254]))
    out=execute([sys.executable,public/'embed_build.py',compiler,d],'embed-rebuild');row=json.loads(out);assert row['rebuilt']==(step!=1) and row['output']==('007ffe' if step==2 else '007fff');events.append(row)
    manifest=read_json(d/'manifest.json');snap=d/('manifest'+str(step)+'.json');dump(snap,manifest)
    for k in manifest['inputs']:bind(k)
    assert sha(d/'embed.c')==source_sha and any(k.endswith('/resource.bin') for k in manifest['inputs'])
   assert read_json(d/'manifest0.json')['binary']!=read_json(d/'manifest2.json')['binary'];embeds.append(dict(compiler=cc,optimization=opt,events=events,source_unchanged=True))
   profiles.append(dict(compiler=cc,optimization=opt,executions=list(range(start,len(records))),staged_features=[1,2,3,4,5,6,7,9],strict_extension_modes=['c17','c23'],extension_modes=['gnu17','gnu23'],missing_header_rejected=True))
 # Actual compiler version change; only affected specimen claims rerun.
 for label,cc,env in [('before20',old,oldenv),('after21',Path(shutil.which('clang')).resolve(),None)]:
  home=runs/label;home.mkdir();start=len(records)
  for probe in [1,2,3,4,5,6,7,9]:
   obj=home/(str(probe)+'.o');exe=home/str(probe);execute([cc,'-O2','-std=c23','-pedantic-errors','-DLF_PROBE='+str(probe),'-c',public/'probes.c','-o',obj],'upgrade-object',env=env);execute([cc,obj,'-o',exe],'upgrade-link',env=env);execute([exe],'upgrade-run')
  execute([cc,'-std=c17','-pedantic-errors','-DLF_PROBE=8','-fsyntax-only',public/'probes.c'],'upgrade-strict',None,env=env)
  obj=home/'gnu.o';exe=home/'gnu';execute([cc,'-std=gnu17','-DLF_PROBE=8','-c',public/'probes.c','-o',obj],'upgrade-extension',env=env);execute([cc,obj,'-o',exe],'upgrade-link',env=env);execute([exe],'upgrade-run');linked(exe,'upgrade-runtime')
  upgrades.append(dict(label=label,compiler=str(cc),executable_sha256=sha(Path(cc).resolve()),executions=list(range(start,len(records))),claims='Eight C23 specimens and strict/GNU statement expression only',all_selected_observations_unchanged=True))
 for label,cc,opt in [('gcc-O0','arm-none-eabi-gcc','-O0'),('gcc-O3','arm-none-eabi-gcc','-O3'),('clang-O3','clang','-O3')]:
  home=runs/('m4-'+label);home.mkdir();start=len(records);flags=[opt,'-std=c17','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-I'+str(public)]+(['--target=arm-none-eabi'] if cc=='clang' else [])
  objs=[]
  for src in [public/'entry.S',public/'target.c',private]:
   obj=home/(src.name+'.o');execute([cc,*flags,'-c',src,'-o',obj],'target-object');out=execute(['arm-none-eabi-nm','-u',obj],'target-undefined');assert set(out.split())<={'U','_sdata','_edata','_sidata','_sbss','_ebss','lp_target','lf_div','lf_add','lf_mixed'},out;execute(['arm-none-eabi-objdump','-dr',obj],'target-assembly');objs.append(obj)
  image=home/'image.elf';execute(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,--gc-sections','-Wl,-Map='+str(home/'image.map'),'-T',public/'target.ld',*objs,'-o',image],'target-link');assert not execute(['arm-none-eabi-nm','-u',image],'image-undefined').strip();out=execute(['arm-none-eabi-readelf','-h','-d',image],'image-ELF');assert 'ELF32' in out and 'ARM' in out and 'NEEDED' not in out
  execute(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',image],'target-run',markers=['LP_TARGET_PASS facilities manual64 no implicit helper'])
  obj=home/'direct.o';execute([cc,*flags,'-c',public/'division_direct.c','-o',obj],'direct-divide');out=execute(['arm-none-eabi-nm','-u',obj],'direct-undefined');assert '__aeabi_uldivmod' in out
  execute(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,-e,lf_direct',obj,'-o',home/'unsupported.elf'],'direct-no-provider',None,markers=['__aeabi_uldivmod'])
  targets.append(dict(profile=label,actual_execution=True,zero_undefined=True,direct_helper_rejected='__aeabi_uldivmod',executions=list(range(start,len(records)))))
 home=runs/'cross-only';home.mkdir();start=len(records);obj=home/'entry.o';image=home/'image.elf';execute(['clang','--target=riscv32-none-elf','-std=c23','-ffreestanding','-fno-builtin','-c',public/'cross.c','-o',obj],'cross-object');execute(['clang','--target=riscv32-none-elf','--ld-path='+str(linker),'-nostdlib','-Wl,-e,entry',obj,'-o',image],'cross-link');out=execute(['readelf','-h',image],'cross-ELF');assert 'ELF32' in out and 'RISC-V' in out;assert not execute(['nm','-u',image],'cross-undefined').strip();cross=dict(target='riscv32-none-elf',compiled=True,linked=True,executed=False,reason='This declared compile-only profile has no execution adapter attached;no emulator or physical run is inferred.',observation_mask=7,executions=list(range(start,len(records))))
 fixture=read_json(ROOT/'fixtures/language/PR-T05-04.json');actual=[all(f['observed'].get(k,False) for k in f['required']) for f in fixture['input']['features']];assert actual==fixture['expected']['usable']
 for p in runs.rglob('*'):
  if p.is_file() and p.suffix!='.log':artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_facilities_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,compiler_dependencies=dependencies,observations=observations,embed_rebuilds=embeds,compiler_upgrade=upgrades,target_profiles=targets,cross_only=cross,synthetic_fixture_usable=actual,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Exact published C17/C23 remain unavailable;draft requirements remain labeled.','Actual QEMU M4 is not a board run;RISC-V has compilation/link evidence only.','Compiler20to21 observation scope is explicitly bounded,not a whole-release compatibility claim.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles),targets=len(targets))))
if __name__=='__main__':main()
