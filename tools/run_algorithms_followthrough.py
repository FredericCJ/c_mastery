#!/usr/bin/env python3
"""T15 original target mechanisms, explicit predecessor closure and finite models."""
import datetime,hashlib,json,os,pathlib,platform,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 public=ROOT/'labs/algorithm_depth';private=ROOT/'assessor/algorithm_depth';folder=ROOT/'research/v5/expansion/legacy_algorithms';build=ROOT/'build/algorithms-followthrough';build.mkdir(parents=True,exist_ok=True)
 providers=[ROOT/name for name in ['labs/containers_depth/containers.h','assessor/containers_depth/ordered.c','assessor/containers_depth/indexed.c','research/v5/expansion/legacy_containers/validation.json','labs/machine_image/freestanding_depth/free.h','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c','reviews/v5/expansion/legacy_systems_freestanding/full_validation.json']]
 def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in [*public.glob('*'),*private.glob('*'),folder/'resources.json',pathlib.Path(__file__),*providers] if p.is_file()}
 initial=pins();rows=[];outputs={};observations={};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(name,command,negative=False,marker=None):
  started=time.monotonic();result=subprocess.run(list(map(str,command)),cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=120)
  passed=(result.returncode!=0 if negative else result.returncode==0) and (marker is None or marker in result.stdout)
  rows.append(dict(id=name,command=list(map(str,command)),exit_code=result.returncode,passed=passed,seconds=time.monotonic()-started,output=result.stdout,expected='nonzero intended assertion' if negative else 'zero'))
  print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+result.stdout)
  return result.stdout
 def keep(path):outputs[path.relative_to(ROOT).as_posix()]=sha(path)
 units={'sorting':[providers[1]],'graph_dp':[providers[2]],'matching':[],'bytes_numbers':[providers[5]],'wire_frames':[],'parser':[],'rle':[]}
 mutations={1:'sorting',2:'sorting',3:'graph_dp',4:'matching',5:'matching',6:'bytes_numbers',7:'wire_frames',8:'parser',9:'rle'}
 includes=['-I'+str(public),'-I'+str(ROOT/'labs/containers_depth'),'-I'+str(ROOT/'labs/machine_image/freestanding_depth')]
 try:
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    prefix=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,*includes]
    for unit,dependencies in units.items():
     binary=build/(prefix+'.'+unit);run(prefix+'.'+unit+'.compile',base+[public/(unit+'_test.c'),private/(unit+'.c'),*dependencies,'-o',binary]);run(prefix+'.'+unit,[binary],marker='PASS');keep(binary)
     stub=build/(prefix+'.'+unit+'.learner');run(prefix+'.'+unit+'.learner.compile',base+[public/(unit+'_test.c'),public/'learner.c',*dependencies,'-o',stub]);run(prefix+'.'+unit+'.learner',[stub],negative=True,marker='ASSERT')
    for mutant,unit in mutations.items():
     binary=build/(prefix+'.mutant'+str(mutant));run(prefix+'.mutant'+str(mutant)+'.compile',base+['-DAD_MUTANT='+str(mutant),public/(unit+'_test.c'),private/(unit+'.c'),*units[unit],'-o',binary]);run(prefix+'.mutant'+str(mutant),[binary],negative=True,marker='ASSERT')
    binary=build/(prefix+'.numbers');run(prefix+'.numbers.compile',base+[public/'numbers_driver.c',private/'bytes_numbers.c',providers[5],'-o',binary]);observations[prefix+'.numbers']=json.loads(run(prefix+'.numbers',['python3',public/'numbers_oracle.py',binary]));keep(binary)
   binary=build/(compiler+'.benchmark');run(compiler+'.benchmark.compile',[compiler,'-std=c17','-O3','-Wall','-Wextra','-Werror',*includes,public/'benchmark.c',private/'sorting.c',private/'rle.c',providers[1],providers[5],'-o',binary]);raw=run(compiler+'.benchmark',[binary]);samples=[json.loads(line) for line in raw.splitlines()];assert len(samples)==522 and all(sample['nanoseconds']>=0 for sample in samples);observations[compiler+'.benchmark']=samples;keep(binary)
   strict=[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror',*includes];objects=[]
   sources=[private/(unit+'.c') for unit in units]+[providers[1],providers[2],providers[5],providers[6],public/'strict_image.c']
   for index,source in enumerate(sources):
    obj=build/(compiler+'.strict.'+str(index)+'.o');name=compiler+'.strict.'+source.stem;run(name+'.compile',strict+['-c',source,'-o',obj]);run(name+'.undefined',['nm','-u',obj]);run(name+'.disassembly',['objdump','-dr',obj]);objects.append(obj);keep(obj)
   entry=build/(compiler+'.entry.o');run(compiler+'.strict.entry',[compiler,'-c',public/'strict_entry.S','-o',entry]);objects.append(entry);keep(entry)
   image=build/(compiler+'.strict.elf');mapping=build/(compiler+'.strict.map');run(compiler+'.strict.link',[compiler,'-nostdlib','-static','-no-pie','-Wl,--no-undefined,-e,_start,-Map,'+str(mapping),*objects,'-o',image]);assert not run(compiler+'.strict.undefined',['nm','-u',image]).strip();headers=run(compiler+'.strict.headers',['readelf','-l',image]);assert 'INTERP' not in headers;run(compiler+'.strict.execute',[image]);keep(image);keep(mapping)
   imports={}
   for policy,flags in [('hosted',[]),('restricted',['-ffreestanding','-fno-builtin'])]:
    obj=build/(compiler+'.lowering.'+policy+'.o');run(compiler+'.lowering.'+policy+'.compile',[compiler,'-std=c17','-O3',*flags,'-c',public/'lowering_probe.c','-o',obj]);imports[policy]=run(compiler+'.lowering.'+policy+'.undefined',['nm','-u',obj]);run(compiler+'.lowering.'+policy+'.disassembly',['objdump','-dr',obj]);keep(obj)
   assert 'memcpy' in imports['hosted'] and not imports['restricted'].strip(),imports
   observations[compiler+'.lowering']={'forbidden_hosted_import_detected':True,'restricted_provider_reaudited':True,'imports':imports}
  observations['finite-oracles']=json.loads(run('finite-oracles',['python3',private/'finite_oracles.py']))
 except Exception as error:failure=str(error)
 else:failure=None
 final=pins()
 if initial!=final:failure='source changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',scope='T15 author original mechanisms and explicitly bounded transfer models',timestamp=datetime.datetime.now(datetime.timezone.utc).isoformat(),environment=dict(platform=platform.platform(),machine=platform.machine()),source_sha256=final,outputs_sha256=outputs,commands=rows,observations=observations,failure=failure,limitations=['No external review or independent delayed learner performance.','Finite source/model spaces and sanitizer silence are not absolute mastery or all-input proof.','Runtime closure is scoped to the executed Linux x86-64 custom images and exact compiler builds; not board execution.','Timings include the declared local control overhead and do not establish physical cache causality or universal speedups.','Exact published C17/C23 source access remains a separate reference gap.'])
 (folder/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(rows),failure=failure)),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
