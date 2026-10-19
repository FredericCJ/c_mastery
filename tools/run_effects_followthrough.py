#!/usr/bin/env python3
"""Source-bound T13 original programs; no host configuration or target writes."""
from __future__ import annotations
import argparse,datetime,hashlib,json,os,pathlib,platform,pty,re,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default='research/v5/expansion/legacy_effects/validation.json');p.add_argument('--prefix',default='/home/frede/cmastery-effects-tools');args=p.parse_args()
 private=pathlib.Path(args.prefix);build=ROOT/'build/effects-followthrough';build.mkdir(parents=True,exist_ok=True)
 public=ROOT/'labs/effects_depth';assessor=ROOT/'assessor/effects_depth'
 rows=[];observations={};outputs={}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1',LD_LIBRARY_PATH=str(private/'usr-root/usr/lib/x86_64-linux-gnu'))
 def source_pins():
  files=[*public.rglob('*'),*assessor.rglob('*'),pathlib.Path(__file__),*sorted((ROOT/'research/v5/expansion/legacy_effects').glob('*resources.json'))]
  return {str(x.relative_to(ROOT)):digest(x) for x in files if x.is_file()}
 initial_pins=source_pins()
 def run(name,command,*,negative=False,marker=None,timeout=120):
  start=time.monotonic();result=subprocess.run([str(x) for x in command],cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=timeout)
  text=result.stdout;passed=(result.returncode!=0 if negative else result.returncode==0) and (marker is None or marker in text)
  row={'id':name,'command':[str(x) for x in command],'exit_code':result.returncode,'seconds':time.monotonic()-start,'expected':'nonzero with required evidence marker' if negative else 'zero','passed':passed,'output':text};rows.append(row)
  print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+text)
  return text
 def keep(path):outputs[str(path.relative_to(ROOT))]=digest(path)
 try:
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    name=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(public)]
    core=build/(name+'.core');run(name+'.core.compile',base+[public/'core_test.c',assessor/'effects.c',assessor/'logger.c','-o',core]);run(name+'.core',[core],marker='EFFECTS_CORE_PASS');keep(core)
    state=build/(name+'.state');run(name+'.state.compile',base+[public/'state_test.c',assessor/'effects.c','-o',state]);observations[name+'.state']=json.loads(run(name+'.state',[state]));keep(state)
    locks=build/(name+'.locks');run(name+'.locks.compile',base+[public/'locks_test.c',assessor/'logger.c','-pthread','-o',locks]);observations[name+'.locks']=json.loads(run(name+'.locks',[locks]));keep(locks)
    for mutant in [1,2,3]:
     binary=build/(name+'.mutant'+str(mutant));run(name+'.mutant'+str(mutant)+'.compile',base+['-DEF_MUTANT='+str(mutant),public/'core_test.c',assessor/'effects.c',assessor/'logger.c','-o',binary]);run(name+'.mutant'+str(mutant),[binary],negative=True,marker='EFFECTS_ASSERT')
    binary=build/(name+'.learner');run(name+'.learner.compile',base+[public/'core_test.c',public/'effects.c',public/'logger.c','-o',binary]);run(name+'.learner',[binary],negative=True,marker='EFFECTS_ASSERT')
  base=['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-fno-builtin','-I'+str(public)]
  dso=build/'libeffects_hidden.so';run('interception.dso',base+['-shared','-fPIC',public/'hidden.c','-o',dso]);keep(dso)
  process=build/'process';run('interception.executable',base+[public/'process_test.c',assessor/'effects.c','-L'+str(build),'-leffects_hidden','-Wl,-rpath,'+str(build),'-Wl,--wrap=malloc','-Wl,--wrap=free','-o',process]);keep(process)
  for mode in ['cache','cache','cache','startup','bypass','refill','resources','callbacks']:
   key='process.'+mode+'.'+str(len(observations));observations[key]=json.loads(run(key,[process,mode]))
  run('process.stdio',[process,'stdio'])
  hidden=build/'hook-scope';run('interception.hidden.compile',base+[public/'hook_scope.c','-Wl,--wrap=ef_local_alloc','-o',hidden]);observations['interception.hidden']=json.loads(run('interception.hidden',[hidden]));keep(hidden)
  cpu=build/'cpu-controls';run('cpu.compile',base+[public/'cpu_test.c','-pthread','-o',cpu]);keep(cpu)
  for mode in ['observer','cpu','wait','threads']:observations['cpu.'+mode]=json.loads(run('cpu.'+mode,[cpu,mode]))
  tracer=private/'usr-root/usr/bin/strace';run('strace.version',[tracer,'--version'])
  for mode in ['stdio','resources','bypass']:
   trace=build/(mode+'.strace');run('syscall.'+mode,[tracer,'-f','-tt','-T','-o',trace,process,mode]);keep(trace);observations['syscall.'+mode]={'path':str(trace.relative_to(ROOT)),'sha256':digest(trace),'syscall_counts':{name:len(re.findall(r'\b'+name+r'\(',trace.read_text())) for name in ['openat','write','read','lseek','mmap','munmap','brk','clock_nanosleep','futex']},'scope':'complete child includes loader/startup; counts not solely attributed to API'}
  for terminal in [False,True]:
   trace=build/('console-'+str(terminal)+'.strace');command=[str(tracer),'-o',str(trace),str(process),'console'];output=build/('console-'+str(terminal)+'.txt')
   if terminal:
    master,slave=pty.openpty();result=subprocess.run(command,cwd=build,env=env,stdout=slave,stderr=subprocess.PIPE,text=True,timeout=30);os.close(slave);data=b''
    while True:
     try:chunk=os.read(master,4096)
     except OSError:break
     if not chunk:break
     data+=chunk
    os.close(master);output.write_bytes(data)
   else:
    with output.open('wb') as stream:result=subprocess.run(command,cwd=build,env=env,stdout=stream,stderr=subprocess.PIPE,text=True,timeout=30)
   passed=result.returncode==0 and ('terminal='+str(int(terminal))) in output.read_text();rows.append({'id':'console.'+str(terminal),'command':command,'exit_code':result.returncode,'passed':passed,'output':output.read_text(),'stderr':result.stderr});assert passed
   keep(trace);keep(output);observations['console.'+str(terminal)]={'writes':len(re.findall(r'\bwrite\(',trace.read_text())),'output':output.read_text()}
  highwater=build/'highwater';run('stack.highwater.compile',base+[public/'highwater_test.c','-pthread','-o',highwater]);observations['stack.highwater']=json.loads(run('stack.highwater',[highwater]));keep(highwater)
  for name,flags in [('O0',['-O0']),('O3',['-O3']),('LTO',['-O3','-flto'])]:
   binary=build/('stack-'+name);run('stack.'+name+'.compile',['gcc','-std=c17',*flags,'-fstack-usage','-fcallgraph-info=su',public/'stack_probe.c','-o',binary]);run('stack.'+name,[binary]);assembly=run('stack.'+name+'.disassembly',['objdump','-dr',binary]);(build/('stack-'+name+'.asm')).write_text(assembly);keep(binary);keep(build/('stack-'+name+'.asm'))
  for path in build.glob('*.su'):keep(path);observations[path.name]=path.read_text()
  for path in build.glob('*.ci'):keep(path)
  experiment=build/'sort-experiment';run('sort.experiment.compile',base+[public/'sort_experiment.c',assessor/'effects.c','-o',experiment]);observations['sort.experiment']=json.loads(run('sort.experiment',[experiment]));run('sort.experiment.size',['size',experiment]);keep(experiment)
  for compiler in ['gcc','clang']:
   obj=build/(compiler+'-strict.o');run(compiler+'.strict.compile',[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-I'+str(public),'-c',assessor/'effects.c','-o',obj]);undefined=run(compiler+'.strict.undefined',['nm','-u',obj]);assert not undefined.strip(),undefined;keep(obj)
   binary=build/(compiler+'-strict.elf');run(compiler+'.strict.link',[compiler,'-nostdlib','-no-pie','-Wl,--no-undefined,-e,ef_sort',obj,'-o',binary]);assert not run(compiler+'.strict.final-undefined',['nm','-u',binary]).strip();keep(binary)
  for cpu in ['cortex-m0','cortex-m4']:
   obj=build/(cpu+'-divide.o');run(cpu+'.helper.compile',['clang','--target=arm-none-eabi','-mcpu='+cpu,'-mthumb','-std=c17','-O2','-ffreestanding','-c',public/'helper_probe.c','-o',obj]);undefined=run(cpu+'.helper.undefined',['nm','-u',obj]);assert '__aeabi_uldivmod' in undefined;keep(obj)
   obj=build/(cpu+'-logger.o');run(cpu+'.logger.compile',['clang','--target=arm-none-eabi','-mcpu='+cpu,'-mthumb','-std=c17','-O2','-ffreestanding','-fstack-usage','-I'+str(public),'-c',assessor/'logger.c','-o',obj]);observations[cpu+'.logger.undefined']=run(cpu+'.logger.undefined',['nm','-u',obj]);keep(obj)
   assembly=build/(cpu+'-logger.s');run(cpu+'.logger.assembly',['clang','--target=arm-none-eabi','-mcpu='+cpu,'-mthumb','-std=c17','-O2','-ffreestanding','-I'+str(public),'-S',assessor/'logger.c','-o',assembly]);keep(assembly)
   if cpu=='cortex-m4':assert 'ldrexb' in assembly.read_text() and 'strexb' in assembly.read_text()
  for path in build.glob('*logger.su'):keep(path);observations[path.name]=path.read_text()
  musl=private/'install-musl-1.2.5/bin/musl-gcc';binary=build/'musl-sort';run('musl.sort.compile',[musl,'-std=c17','-O2','-static','-Wl,--wrap=malloc',public/'musl_sort_probe.c','-o',binary]);observations['musl.sort']=json.loads(run('musl.sort',[binary]));keep(binary)
  glibc=private/'install-glibc-2.41/lib';binary=build/'glibc-sort';run('glibc.sort.compile',['gcc','-std=c17','-O2','-fno-builtin','-L'+str(glibc),'-Wl,--dynamic-linker,'+str(glibc/'ld-linux-x86-64.so.2'),'-Wl,-rpath,'+str(glibc),public/'glibc_sort_probe.c','-o',binary]);observations['glibc.sort']=json.loads(run('glibc.sort',[binary]));assert observations['glibc.sort']['runtime']=='2.41';keep(binary)
  for config in ['double','integer']:
   prefix=private/('install-picolibc-'+config)/'lib/picolibc';binary=build/('pico-'+config);run('pico.'+config+'.compile',['gcc','-std=c17','-O2','-I'+str(prefix/'include'),public/'pico_probe.c',str(prefix/'lib/libc.a'),'-lm','-o',binary]);observations['pico.'+config]=run('pico.'+config,[binary]);keep(binary)
  prefix=private/'install-picolibc-custom/lib/picolibc';binary=build/'pico-custom';run('pico.custom.compile',['gcc','-std=c17','-O2','-I'+str(prefix/'include'),public/'pico_custom.c',str(prefix/'lib/libc.a'),'-lm','-o',binary]);run('pico.custom',[binary]);keep(binary)
  prefix=private/'install-picolibc-double/lib/picolibc';binary=build/'pico-file';command=['gcc','-std=c17','-O2','-I'+str(prefix/'include'),public/'pico_file.c',str(prefix/'lib/libc.a'),'-lm','-o',binary];run('pico.file.compile',command);run('pico.file',[binary,build/'synthetic-pico.txt']);keep(binary)
  run('pico.missing-seek.link',command[:-1]+[build/'pico-missing-seek','-Wl,--wrap=lseek'],negative=True,marker='__wrap_lseek')
  run('thread-sanitizer.compile',['clang','-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie','-I'+str(public),public/'locks_test.c',assessor/'logger.c','-pthread','-o',build/'locks-tsan']);observations['thread-sanitizer']=json.loads(run('thread-sanitizer',[build/'locks-tsan']))
  observations['finite-oracles']=json.loads(run('finite-oracles',['python3',public/'oracles.py']))
 except Exception as exc:
  failure=str(exc)
 else:failure=None
 pins=source_pins()
 if pins!=initial_pins:failure='Source inputs changed during execution'
 external=[]
 for relative in ['download_manifest.json','runtime_builds.json','glibc-2.41/stdlib/qsort.c','musl-1.2.5/src/stdlib/qsort.c','picolibc-1.8.10/doc/os.md','picolibc-1.8.10/doc/build.md','picolibc-1.8.10/meson_options.txt','picolibc-1.8.10/newlib/libc/tinystdio/fseek.c','build-picolibc-custom/build.log','build-picolibc-custom/resume_commands.json','build-glibc-2.41/resume_commands.json','install-glibc-2.41/lib/libc.so.6','install-musl-1.2.5/lib/libc.a','install-picolibc-double/lib/picolibc/lib/libc.a','install-picolibc-integer/lib/picolibc/lib/libc.a','install-picolibc-custom/lib/picolibc/lib/libc.a']:
  path=private/relative
  if path.is_file():external.append({'path':str(path),'sha256':digest(path)})
 report={'schema_version':1,'status':'pass' if failure is None else 'fail','scope':'T13 author execution only; original effects programs and selected exact runtime/configuration observations','timestamp':datetime.datetime.now(datetime.timezone.utc).isoformat(),'environment':{'platform':platform.platform(),'machine':platform.machine()},'source_sha256':pins,'external_inputs':external,'commands':rows,'observations':observations,'outputs_sha256':outputs,'failure':failure,'limitations':['Finite test paths and measured configurations do not establish learner mastery, external review, hard real-time bounds, all libc internals, or physical interrupt behavior.','Thread highwater includes pthread runtime and pattern collisions can underestimate writes; .su and closed call graph are separate evidence.','Picolibc native configurations delegate OS services to host libc.','Compiler helper and atomic observations are object audits only for ARM; no device execution.']}
 destination=ROOT/args.report;destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'status':report['status'],'commands':len(rows),'report':args.report,'failure':failure}),flush=True)
 return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
