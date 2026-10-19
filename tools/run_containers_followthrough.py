#!/usr/bin/env python3
"""Execute original T14 components, intended faults, independent models and audits."""
import datetime,hashlib,json,os,pathlib,platform,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 public=ROOT/'labs/containers_depth';assessor=ROOT/'assessor/containers_depth';folder=ROOT/'research/v5/expansion/legacy_containers';build=ROOT/'build/containers-followthrough';build.mkdir(parents=True,exist_ok=True)
 def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in [*public.glob('*'),*assessor.glob('*'),folder/'resources.json',pathlib.Path(__file__)] if p.is_file()}
 initial=pins();rows=[];outputs={};observations={};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(name,command,negative=False,marker=None):
  started=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=90)
  passed=(r.returncode!=0 if negative else r.returncode==0) and (marker is None or marker in r.stdout);rows.append({'id':name,'command':list(map(str,command)),'exit_code':r.returncode,'passed':passed,'seconds':time.monotonic()-started,'output':r.stdout,'expected':'nonzero with intended assertion' if negative else 'zero'});print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+r.stdout)
  return r.stdout
 def keep(p):outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 units={'sequence':['sequence.c'],'ordered':['ordered.c'],'indexed':['indexed.c'],'transfer':['sequence.c','ordered.c','transfer.c'],'payload':['sequence.c'],'graph_consumer':['indexed.c','graph_consumer.c']}
 try:
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    prefix=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(public)]
    for unit,sources in units.items():
     binary=build/(prefix+'.'+unit);run(prefix+'.'+unit+'.compile',base+[public/(unit+'_test.c'),*[assessor/s for s in sources],'-o',binary]);run(prefix+'.'+unit,[binary],marker='PASS');keep(binary)
     stub=build/(prefix+'.'+unit+'.learner');run(prefix+'.'+unit+'.learner.compile',base+[public/(unit+'_test.c'),public/'learner.c','-o',stub]);run(prefix+'.'+unit+'.learner',[stub],negative=True,marker='ASSERT')
    for mutant in range(1,8):
     unit='sequence' if mutant<=3 else 'ordered' if mutant<=5 else 'indexed';binary=build/(prefix+'.mutant'+str(mutant));run(prefix+'.mutant'+str(mutant)+'.compile',base+['-DCD_MUTANT='+str(mutant),public/(unit+'_test.c'),*[assessor/s for s in units[unit]],'-o',binary]);run(prefix+'.mutant'+str(mutant),[binary],negative=True,marker='ASSERT')
    binary=build/(prefix+'.vector');run(prefix+'.vector.compile',base+[public/'vector_driver.c',assessor/'sequence.c','-o',binary]);observations[prefix+'.vector']=json.loads(run(prefix+'.vector',['python3',public/'vector_oracle.py',binary]));keep(binary)
    binary=build/(prefix+'.mutant8');run(prefix+'.mutant8.compile',base+['-DCD_MUTANT=8',public/'vector_driver.c',assessor/'sequence.c','-o',binary]);observations[prefix+'.mutant8']=json.loads(run(prefix+'.mutant8',['python3',public/'vector_oracle.py',binary,'--mutant'],marker='intended_mutant_rejected'))
   binary=build/(compiler+'.benchmark');run(compiler+'.benchmark.compile',[compiler,'-std=c17','-O3','-Wall','-Wextra','-Werror','-I'+str(public),public/'benchmark.c',assessor/'sequence.c',assessor/'ordered.c',assessor/'indexed.c','-o',binary]);text=run(compiler+'.benchmark',[binary]);samples=[json.loads(line) for line in text.splitlines()];assert len(samples)==126 and all(x['nanoseconds']>=0 for x in samples);observations[compiler+'.benchmark']=samples;keep(binary)
   objects=[]
   for source in ['sequence.c','ordered.c','indexed.c','transfer.c','graph_consumer.c','byte_helpers.c']:
    path=public/source if source=='byte_helpers.c' else assessor/source;obj=build/(compiler+'.'+source+'.o');run(compiler+'.strict.'+source+'.compile',[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(public),'-c',path,'-o',obj]);undefined=run(compiler+'.strict.'+source+'.undefined',['nm','-u',obj]);assert all(line.split()[-1] in {'memcpy','memset'} for line in undefined.splitlines() if line.strip()),undefined;objects.append(obj);keep(obj)
   binary=build/(compiler+'.strict.elf');run(compiler+'.strict.link',[compiler,'-nostdlib','-no-pie','-Wl,--no-undefined,-e,cd_array_init',*objects,'-o',binary]);assert not run(compiler+'.strict.final-undefined',['nm','-u',binary]).strip();keep(binary)
  observations['finite-oracles']=json.loads(run('finite-oracles',['python3',assessor/'finite_oracles.py']))
 except Exception as exc:failure=str(exc)
 else:failure=None
 final=pins()
 if final!=initial:failure='source changed during campaign'
 report={'schema_version':1,'status':'pass' if failure is None else 'fail','scope':'T14 original containers and exact finite transfer models; author execution only','timestamp':datetime.datetime.now(datetime.timezone.utc).isoformat(),'environment':{'platform':platform.platform(),'machine':platform.machine()},'source_sha256':final,'outputs_sha256':outputs,'commands':rows,'observations':observations,'failure':failure,'limitations':['No external review or independent delayed learner performance.','No actual concurrent implementation, persistent device, physical cache or universal timing proof.','ABI/helper closure is scoped to the audited x86 host builds; host adapters are separate dependencies.','Finite source/model spaces do not prove all inputs or absolute mastery.']}
 (folder/'validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'status':report['status'],'commands':len(rows),'failure':failure}),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
