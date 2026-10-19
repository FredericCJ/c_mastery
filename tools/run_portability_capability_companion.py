"""Observe separate language/header/object/link/run layers and strict M4 closure."""
from datetime import datetime,timezone
from pathlib import Path
import json,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough'
REPORT=BASE+'/capability_companion_validation.json'

def main():
 public=ROOT/'labs/language_portability/capability';private=ROOT/'assessor/language_portability/capability/capability.c'
 paths=['tools/run_portability_capability_companion.py','tools/common.py','fixtures/language/PR-T05-02.json',private.relative_to(ROOT).as_posix()]+[p.relative_to(ROOT).as_posix() for p in public.iterdir() if p.is_file()]
 inputs={p:sha(ROOT/p) for p in paths};external={};records=[];profiles=[];artifacts=[];dependencies=[];observations=[]
 runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('capability-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 def execute(cmd,label,negative=False,markers=(),timeout=60):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang','arm-none-eabi-gcc'] and '-c' in cmd and any(x.endswith(('.c','.S')) for x in cmd):
   dep=runs/(f'{len(records):04d}-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=timeout);out=p.stdout+p.stderr;log=runs/(f'{len(records):04d}-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_rejection=negative,markers=list(markers),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
  assert (p.returncode!=0 if negative else p.returncode==0) and all(m in out for m in markers),out
  if dep and dep.is_file():
   raw=dep.read_text().replace('\\\n',' ');paths=[Path(x).resolve() for x in shlex.split(raw.split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if q.is_relative_to(ROOT):inputs.setdefault(q.relative_to(ROOT).as_posix(),sha(q))
    else:external.setdefault(str(q),sha(q))
   artifacts.append(dict(path=dep.relative_to(ROOT).as_posix(),sha256=sha(dep)))
  return len(records)-1,out
 def object_file(cc,flags,source,dest,label):
  execute([cc,*flags,'-c',source,'-o',dest],label+'-compile');execute(['nm','-u',dest],label+'-undefined');execute(['nm','--defined-only',dest],label+'-defined');return dest
 tools=['gcc','clang','nm','objdump','ldd','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-readelf','qemu-system-arm']
 for tool in tools:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],'version-'+tool)
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 for cc in ['gcc','clang']:
  for opt in ['-O0','-O3']:
   home=runs/(cc+opt);home.mkdir();start=len(records);flags=[opt,'-Wall','-Wextra','-pedantic-errors'];feature_rows=[];inline_rows=[]
   # Each feature has its own preprocessing, semantic translation, object, link and run.
   for mode in ['c11','c17']:
    for feature in range(1,7):
     f=[*flags,'-std='+mode,'-DLP_FEATURE='+str(feature)];stem=mode+'-feature'+str(feature);obj=home/(stem+'.o');exe=home/stem;steps=[]
     steps.append(execute([cc,*f,'-E',public/'features.c'],stem+'-header')[0]);steps.append(execute([cc,*f,'-fsyntax-only',public/'features.c'],stem+'-language')[0])
     steps.append(execute([cc,*f,'-c',public/'features.c','-o',obj],stem+'-object')[0]);steps.append(execute([cc,obj,'-pthread','-o',exe],stem+'-link')[0]);steps.append(execute([exe],stem+'-run')[0])
     feature_rows.append(dict(mode=mode,feature=feature,steps=steps))
    execute([cc,'-std='+mode,'-DLP_FEATURE=5','-E','-dM',public/'features.c'],mode+'-optional-macros')
   # An exact-width facility/format C99 probe is separate from current C11/C17 rows.
   exe=home/'c99-format';obj=object_file(cc,[*flags,'-std=c99','-DLP_FEATURE=6'],public/'features.c',home/'c99-format.o','c99-format');execute([cc,obj,'-o',exe],'c99-format-link');execute([exe],'c99-format-run')
   if cc=='clang':execute([cc,*flags,'-std=gnu89','-c',public/'inline_a.c','-o',home/'strict-gnu89.o'],'strict-gnu89-inline',True,['extension used'])
   for mode in ['c99','c17','gnu89']:
    f=[*([opt,'-Wall','-Wextra','-pedantic'] if mode=='gnu89' else flags),'-std='+mode];stem=mode+'-static';objs=[object_file(cc,f,public/('inline_'+x+'.c'),home/(stem+x+'.o'),stem+x) for x in ['a','b','main']];execute([cc,*objs,'-o',home/stem],stem+'-link');execute([home/stem],stem+'-run',markers=['LP_INLINE values5,7']);inline_rows.append(dict(mode=mode,kind='static',passed=True))
    # GNU89 VLA is an explicitly allowed extension: preserve warnings, no ISO claim.
    vf=f if mode!='gnu89' else [opt,'-Wall','-Wextra','-pedantic','-std=gnu89']
    vo=object_file(cc,vf,public/'vla.c',home/(mode+'-vla.o'),mode+'-vla');execute([cc,vo,'-o',home/(mode+'-vla')],mode+'-vla-link');execute([home/(mode+'-vla')],mode+'-vla-run',markers=['LP_VLA positive_bounds'])
   for mode,extra,negative in [('c99',[],False),('c17',[],False),('gnu89',[],True),('gnu89',['-DLP_GNU_EXTERN'],False)]:
    stem=mode+'-external'+('-extern' if extra else '');f=[*([opt,'-Wall','-Wextra','-pedantic'] if mode=='gnu89' else flags),'-std='+mode,'-DLP_EXTERNAL',*extra];objs=[object_file(cc,f,public/('inline_'+x+'.c'),home/(stem+x+'.o'),stem+x) for x in ['a','b','main','provider']]
    index,_=execute([cc,*objs,'-o',home/stem],stem+'-link',negative,['lp_inline'] if negative else [])
    if not negative:execute([home/stem],stem+'-run',markers=['LP_INLINE values5,7'])
    inline_rows.append(dict(mode=mode,kind='external-extern' if extra else 'external',intended_duplicate_rejection=negative,link=index))
   old=object_file(cc,[*flags,'-std=c90'],public/'old_record.c',home/'old.o','old-record');execute([cc,old,'-o',home/'old'],'old-link');_,oldout=execute([home/'old'],'old-run')
   for layout in [1,2]:
    f=[*flags,'-std=c17','-I'+str(public),*(['-DLP_LAYOUT2'] if layout==2 else [])];objs=[object_file(cc,f,s,home/(f'new{layout}-{i}.o'),f'new{layout}-{i}') for i,s in enumerate([public/'new_record.c',private])];exe=home/f'new{layout}';execute([cc,*objs,'-o',exe],'new-link');_,out=execute([exe],'new-run');assert out==oldout;observations.append(dict(kind='record-port',compiler=cc,optimization=opt,layout=layout,output=out,identical=True))
   # Actual oversized atomic helper: object compiles; default link is unavailable.
   obj=object_file(cc,[*flags,'-std=c17','-DLP_FEATURE=7'],public/'features.c',home/'wide.o','wide-atomic');_,symbols=execute(['nm','-u',obj],'wide-helper-audit');assert '__atomic_' in symbols
   execute([cc,obj,'-o',home/'wide-missing'],'wide-missing',True,['__atomic_'])
   execute([cc,obj,'-latomic','-o',home/'wide'],'wide-explicit-provider');execute([home/'wide'],'wide-run');_,linked=execute(['ldd',home/'wide'],'wide-libraries');assert 'libatomic' in linked
   _,library=execute([cc,'-print-file-name=libatomic.so'],'wide-provider-path');library=Path(library.strip()).resolve();assert library.is_file();external[str(library)]=sha(library)
   for token in linked.split():
    q=Path(token)
    if q.is_absolute() and q.is_file():external[str(q.resolve())]=sha(q.resolve())
   f=[*flags,'-std=c17'];execute([cc,*f,'-E',public/'optional.c'],'optional-header');execute([cc,*f,'-fsyntax-only',public/'optional.c'],'optional-language');obj=object_file(cc,f,public/'optional.c',home/'optional.o','optional-object');execute([cc,obj,'-o',home/'missing'],'optional-missing',True,['lp_optional_runtime']);helper=object_file(cc,f,public/'helper.c',home/'helper.o','optional-helper');execute([cc,obj,helper,'-o',home/'provided'],'optional-linked');execute([home/'provided'],'optional-run')
   profiles.append(dict(profile=cc+opt,compiler=cc,optimization=opt,executions=list(range(start,len(records))),features=feature_rows,inline=inline_rows,changed_layouts=2,wide_atomic_helper=True,explicit_original_helper=True))
 # Separate selected freestanding target: optional headers do not grant runtime.
 targets=[]
 for label,cc,opt in [('gcc-O0','arm-none-eabi-gcc','-O0'),('gcc-O3','arm-none-eabi-gcc','-O3'),('clang-O3','clang','-O3')]:
  home=runs/('m4-'+label);home.mkdir();start=len(records);flags=[opt,'-std=c17','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections','-fdata-sections','-I'+str(public)]
  if cc=='clang':flags.insert(0,'--target=arm-none-eabi')
  execute([cc,*flags,'-E','-dM',public/'target.c'],label+'-macros');execute([cc,*flags,'-fsyntax-only',public/'target.c'],label+'-language')
  objs=[]
  for s in ['entry.S','target.c']:
   obj=home/(s+'.o');execute([cc,*flags,'-c',public/s,'-o',obj],label+'-compile');_,out=execute(['arm-none-eabi-nm','-u',obj],label+'-object-undefined');assert set(out.split())<={'U','_sdata','_edata','_sidata','_sbss','_ebss','lp_target'},out;execute(['arm-none-eabi-objdump','-dr',obj],label+'-disassembly');objs.append(obj)
  image=home/'image.elf';execute(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,--gc-sections','-Wl,-Map='+str(home/'image.map'),'-T',public/'target.ld',*objs,'-o',image],label+'-link');_,out=execute(['arm-none-eabi-nm','-u',image],label+'-image-undefined');assert not out.strip();_,out=execute(['arm-none-eabi-readelf','-h','-d',image],label+'-ELF');assert 'ELF32' in out and 'ARM' in out and 'NEEDED' not in out
  execute(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',image],label+'-execute',markers=['LP_TARGET_PASS selected32 no-library helpers audited'])
  wide=home/'wide.o';execute([cc,*flags,'-c',public/'wide_target.c','-o',wide],label+'-wide');_,out=execute(['arm-none-eabi-nm','-u',wide],label+'-wide-audit');assert '__atomic_fetch_add_8' in out
  execute(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,-e,lp_wide',wide,'-o',home/'unsupported.elf'],label+'-wide-link',True,['__atomic_fetch_add_8'])
  targets.append(dict(profile=label,actual_execution=True,zero_undefined=True,wide_runtime_rejected=True,executions=list(range(start,len(records)))))
 # Explicitly probe the selected Arm sysroot's missing thread header (not universal).
 execute(['arm-none-eabi-gcc','-std=c17','-ffreestanding','-DLP_FEATURE=5','-E',public/'features.c'],'target-threads-unavailable',True,['threads.h'])
 fixture=read_json(ROOT/'fixtures/language/PR-T05-02.json');actual=[all(x['observed'].get(k,False) for k in x['required']) for x in fixture['input']['features']];assert actual==fixture['expected']['usable']==[False,True]
 for p in runs.rglob('*'):
  if p.is_file() and p.suffix!='.log' and not any(r['path']==p.relative_to(ROOT).as_posix() for r in artifacts):artifacts.append(dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_capability_companion.py',input_sha256=inputs,source_stable=stable,external_input_sha256=external,external_inputs_stable=extstable,generated_artifacts=artifacts,executions=records,profiles=profiles,target_profiles=targets,compiler_dependencies=dependencies,observations=observations,synthetic_fixture_usable=actual,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C17 core; C90/C99/C11/GNU89 are separately labeled comparison probes.','Actual selected QEMU M4 execution is not a physical board run.','Optional macro/header observations are not universal capability; wide atomic runtime explicitly rejected in strict target.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles),targets=len(targets))))
if __name__=='__main__':main()
