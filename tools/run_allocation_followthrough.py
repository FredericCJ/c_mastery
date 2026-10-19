#!/usr/bin/env python3
"""T16 author mechanisms and finite transfer evidence; no learner/board claim."""
import datetime,hashlib,json,os,pathlib,platform,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
PROVIDERS=['labs/algorithms/include/algo.h','assessor/algorithms/algo.c','labs/machine_image/freestanding_depth/free.h','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c','reviews/v5/expansion/legacy_systems_freestanding/full_validation.json']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 files=[*(ROOT/'labs/allocation_depth').glob('*'),*(ROOT/'assessor/allocation_depth').glob('*'),ROOT/'research/v5/expansion/legacy_allocation/resources.json',pathlib.Path(__file__),*[ROOT/p for p in PROVIDERS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in files if p.is_file()}
def main():
 public=ROOT/'labs/allocation_depth';private=ROOT/'assessor/allocation_depth';folder=ROOT/'research/v5/expansion/legacy_allocation';build=ROOT/'build/allocation-followthrough';build.mkdir(parents=True,exist_ok=True)
 initial=source_pins();rows=[];outputs={};observations={};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(name,command,negative=False,marker=None):
  started=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=120)
  passed=(r.returncode!=0 if negative else r.returncode==0) and (marker is None or marker in r.stdout)
  rows.append(dict(id=name,command=list(map(str,command)),exit_code=r.returncode,passed=passed,seconds=time.monotonic()-started,output=r.stdout,expected='nonzero intended diagnostic' if negative else 'zero'))
  print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+r.stdout)
  return r.stdout
 def keep(p):outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 includes=['-I'+str(public),'-I'+str(ROOT/'labs/algorithms/include'),'-I'+str(ROOT/'labs/machine_image/freestanding_depth')]
 units={'arena':[private/'arena.c'],'heap':[private/'heap.c'],'extents':[private/'extents.c',private/'descriptor.c',private/'heap.c'],'pools':[private/'pools.c',ROOT/'assessor/algorithms/algo.c'],'buddy':[private/'buddy.c'],'backends':[private/'arena.c',private/'heap.c']}
 mutations={1:'arena',2:'arena',3:'heap',4:'heap',5:'heap',6:'pools',7:'buddy'}
 try:
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    prefix=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,*includes]
    for unit,sources in units.items():
     binary=build/(prefix+'.'+unit);run(prefix+'.'+unit+'.compile',base+[public/(unit+'_test.c'),*sources,'-pthread','-lm','-o',binary]);raw=run(prefix+'.'+unit,[binary],marker='PASS');observations[prefix+'.'+unit]=json.loads(raw) if unit=='backends' else raw;keep(binary)
     dependencies=[ROOT/'assessor/algorithms/algo.c'] if unit=='pools' else []
     stub=build/(prefix+'.'+unit+'.learner');run(prefix+'.'+unit+'.learner.compile',base+[public/(unit+'_test.c'),public/'learner.c',*dependencies,'-pthread','-lm','-o',stub]);run(prefix+'.'+unit+'.learner',[stub],negative=True,marker='ASSERT');keep(stub)
    for mutant,unit in mutations.items():
     binary=build/(prefix+'.mutant'+str(mutant));run(prefix+'.mutant'+str(mutant)+'.compile',base+['-DAL_MUTANT='+str(mutant),public/(unit+'_test.c'),*units[unit],'-lm','-o',binary]);run(prefix+'.mutant'+str(mutant),[binary],negative=True,marker='ASSERT');keep(binary)
    binary=build/(prefix+'.placement');run(prefix+'.placement.compile',base+[public/'heap_driver.c',private/'heap.c','-o',binary]);observations[prefix+'.placement']=json.loads(run(prefix+'.placement',['python3',public/'placement_oracle.py',binary]));keep(binary)
   binary=build/(compiler+'.benchmark');run(compiler+'.benchmark.compile',[compiler,'-std=c17','-O3','-Wall','-Wextra','-Werror',*includes,public/'benchmark.c',private/'heap.c','-o',binary]);raw=run(compiler+'.benchmark',[binary]);samples=[json.loads(line) for line in raw.splitlines()];assert len(samples)==108 and all(s['ns']>=0 for s in samples);observations[compiler+'.benchmark']=samples;keep(binary)
   for mode,flags in [('good',[]),('bad',['-DAL_MUTANT=3'])]:
    binary=build/(compiler+'.corruption.'+mode);run(compiler+'.corruption.'+mode+'.compile',[compiler,'-std=c17','-O2','-Wall','-Wextra','-Werror',*includes,*flags,public/'corruption_driver.c',private/'heap.c','-o',binary]);keep(binary)
   observations[compiler+'.minimized']=json.loads(run(compiler+'.minimized',['python3',public/'minimize_corruption.py',build/(compiler+'.corruption.good'),build/(compiler+'.corruption.bad')]))
   strict=[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror',*includes];objects=[]
   sources=[private/(name+'.c') for name in ['arena','heap','extents','descriptor','buddy']]+[ROOT/PROVIDERS[3],ROOT/PROVIDERS[4],public/'strict_image.c']
   for i,source in enumerate(sources):
    obj=build/(compiler+'.strict.'+str(i)+'.o');key=compiler+'.strict.'+source.stem;run(key+'.compile',strict+['-c',source,'-o',obj]);run(key+'.undefined',['nm','-u',obj]);run(key+'.disassembly',['objdump','-dr',obj]);keep(obj);objects.append(obj)
   obj=build/(compiler+'.entry.o');run(compiler+'.strict.entry',[compiler,'-c',public/'strict_entry.S','-o',obj]);objects.append(obj);keep(obj)
   binary=build/(compiler+'.strict.elf');mapping=build/(compiler+'.strict.map');run(compiler+'.strict.link',[compiler,'-nostdlib','-static','-no-pie','-Wl,--no-undefined,-e,_start,-Map,'+str(mapping),*objects,'-o',binary]);assert not run(compiler+'.strict.undefined',['nm','-u',binary]).strip();assert 'INTERP' not in run(compiler+'.strict.headers',['readelf','-l',binary]);run(compiler+'.strict.execute',[binary]);keep(binary);keep(mapping)
  for target in ['cortex-m0','cortex-m4']:
   for name in ['arena','heap','extents','descriptor','buddy']:
    obj=build/(target+'.'+name+'.o');key=target+'.'+name;run(key+'.compile',['clang','--target=arm-none-eabi','-mcpu='+target,'-mthumb','-std=c17','-O2','-ffreestanding','-fno-builtin','-fstack-usage',*includes,'-c',private/(name+'.c'),'-o',obj]);run(key+'.undefined',['nm','-u',obj]);keep(obj)
  for path in build.glob('*.su'):keep(path);observations[path.name]=path.read_text()
  for mode,source,deps in [('repaired',public/'backends_test.c',[private/'arena.c',private/'heap.c']),('racy',public/'race_target.c',[])]:
   binary=build/('tsan.'+mode);run('tsan.'+mode+'.compile',['clang','-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie',*includes,source,*deps,'-pthread','-o',binary]);raw=run('tsan.'+mode,[binary],negative=mode=='racy',marker='ThreadSanitizer: data race' if mode=='racy' else 'PASS');observations['tsan.'+mode]=raw;keep(binary)
  observations['finite-oracles']=json.loads(run('finite-oracles',['python3',private/'finite_oracles.py']))
 except Exception as error:failure=str(error)
 else:failure=None
 final=source_pins()
 if initial!=final:failure='source changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',scope='T16 author actual mechanisms, native adapters and explicit finite transfer derivations',timestamp=datetime.datetime.now(datetime.timezone.utc).isoformat(),environment=dict(platform=platform.platform(),machine=platform.machine()),source_sha256=final,outputs_sha256=outputs,commands=rows,observations=observations,failure=failure,limitations=['No independent learner performance or external review.','Finite histories, mathematical dossiers and sanitizer observations do not prove arbitrary C safety or absolute mastery.','ARM evidence is object/helper/stack auditing only, not final firmware execution or physical timing.','Strict final closure/execution applies only to selected original Linux x86-64 images; the hosted pool producer retains its existing profile.','The instrumented race is an intentional original local diagnostic; its numeric output has no defined semantic oracle.','Exact published C17/C23 text remains distinct from inspected drafts.','Raw batch timings include mandatory validation and harness/reset overhead; additional validation is toggled separately.'])
 (folder/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(rows),failure=failure)),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
