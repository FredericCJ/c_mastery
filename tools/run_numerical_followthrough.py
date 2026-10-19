"""Execute bounded T17 mechanisms and separate numerical model experiments."""
import datetime,hashlib,json,os,pathlib,platform,subprocess,time
ROOT=pathlib.Path(__file__).resolve().parents[1]
PROVIDERS=['labs/checkpoint_depth/include/checkpoint_numeric.h','assessor/checkpoint_depth/checkpoint_numeric.c','reviews/v5/expansion/legacy_completion/checkpoint_numeric.json','labs/machine_image/freestanding_depth/free.h','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c','reviews/v5/expansion/legacy_systems_freestanding/full_validation.json']
UNITS={'sum':['sum'],'fixed':['fixed'],'matrix':['matrix'],'interp':['interp'],'solve':['solve','matrix'],'euler':['dynamics'],'layout':['layout'],'filter':['dynamics','fixed']}
MUTANTS={1:'sum',2:'fixed',3:'matrix',4:'interp',5:'solve',6:'euler',7:'layout',8:'filter'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 files=[*(ROOT/'labs/numerical_depth').glob('*'),*(ROOT/'assessor/numerical_depth').glob('*'),ROOT/'research/v5/expansion/legacy_numerics/resources.json',pathlib.Path(__file__),*[ROOT/p for p in PROVIDERS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in files if p.is_file()}
def main():
 public=ROOT/'labs/numerical_depth';private=ROOT/'assessor/numerical_depth';folder=ROOT/'research/v5/expansion/legacy_numerics';build=ROOT/'build/numeric-followthrough';build.mkdir(parents=True,exist_ok=True)
 initial=source_pins();rows=[];outputs={};observations={};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(name,command,negative=False,marker=None):
  start=time.monotonic();r=subprocess.run(list(map(str,command)),cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180)
  passed=(r.returncode!=0 if negative else r.returncode==0) and (marker is None or marker in r.stdout);rows.append(dict(id=name,command=list(map(str,command)),exit_code=r.returncode,passed=passed,seconds=time.monotonic()-start,output=r.stdout,expected='nonzero intended assertion' if negative else 'zero'));print(name,'PASS' if passed else 'FAIL',flush=True)
  if not passed:raise RuntimeError(name+'\n'+r.stdout)
  return r.stdout
 def keep(p):outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 includes=['-I'+str(public),'-I'+str(ROOT/'labs/checkpoint_depth/include')];producer=ROOT/PROVIDERS[1]
 try:
  run('runtime',['ldd','--version']);run('platform',['uname','-a'])
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    prefix=compiler+'.'+profile;base=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-ffp-contract=off','-fno-fast-math',*flags,*includes]
    for unit,names in UNITS.items():
     sources=[private/(n+'.c') for n in names];deps=[producer] if unit=='filter' else [];binary=build/(prefix+'.'+unit)
     run(prefix+'.'+unit+'.compile',base+[public/(unit+'_test.c'),*sources,*deps,'-lm','-o',binary]);observations[prefix+'.'+unit]=run(prefix+'.'+unit,[binary],marker='PASS');keep(binary)
     stub=build/(prefix+'.'+unit+'.learner');run(prefix+'.'+unit+'.learner.compile',base+[public/(unit+'_test.c'),public/'learner.c',*deps,'-lm','-o',stub]);run(prefix+'.'+unit+'.learner',[stub],negative=True,marker='ND_ASSERT');keep(stub)
     if unit=='filter':
      stub=build/(prefix+'.filter.producer-learner');run(prefix+'.filter.producer-learner.compile',base+[public/'filter_test.c',*sources,public/'filter_learner.c','-lm','-o',stub]);run(prefix+'.filter.producer-learner',[stub],negative=True,marker='ND_ASSERT');keep(stub)
    for mutant,unit in MUTANTS.items():
     binary=build/(prefix+'.mutant'+str(mutant));sources=[private/(n+'.c') for n in UNITS[unit]];deps=[producer] if unit=='filter' else []
     run(prefix+'.mutant'+str(mutant)+'.compile',base+['-DND_MUTANT='+str(mutant),public/(unit+'_test.c'),*sources,*deps,'-lm','-o',binary]);run(prefix+'.mutant'+str(mutant),[binary],negative=True,marker='ND_ASSERT');keep(binary)
    binary=build/(prefix+'.oracle');run(prefix+'.oracle.compile',base+[public/'driver.c',*[private/(n+'.c') for n in ['sum','interp','solve','matrix','dynamics']],'-lm','-o',binary]);observations[prefix+'.oracle']=json.loads(run(prefix+'.oracle',['python3',public/'oracle.py',binary]));keep(binary)
   for mode,flags in [('strict',['-frounding-math','-ffp-contract=off','-fno-fast-math']),('relaxed',['-ffast-math','-ffp-contract=fast'])]:
    binary=build/(compiler+'.environment.'+mode);run(compiler+'.environment.'+mode+'.compile',[compiler,'-std=c17','-O3',*flags,*includes,public/'environment.c',private/'sum.c','-pthread','-lm','-o',binary]);raw=json.loads(run(compiler+'.environment.'+mode,[binary]));observations[compiler+'.environment.'+mode]=raw;keep(binary)
    if mode=='strict':
     values=list(map(float.fromhex,raw['rounding']));assert values==[1.,1.+2**-52,1.,1.],raw;assert float.fromhex(raw['separate'])==0 and float.fromhex(raw['fma'])==-2**-54;assert list(map(float.fromhex,raw['two_worker_splits']))==[1.,0.] and float.fromhex(raw['compensated'])==1
    run(compiler+'.environment.'+mode+'.disassembly',['objdump','-d',binary])
   binary=build/(compiler+'.benchmark');vf=['-fopt-info-vec-all='+str(build/(compiler+'.vectorization.txt'))] if compiler=='gcc' else ['-Rpass=loop-vectorize','-Rpass-missed=loop-vectorize'];run(compiler+'.benchmark.compile',[compiler,'-std=c17','-O3','-ffp-contract=off','-fno-fast-math',*vf,*includes,public/'benchmark.c','-o',binary]);raw=run(compiler+'.benchmark',[binary]);samples=[json.loads(x) for x in raw.splitlines()];assert len(samples)==108 and all(x['ns']>=0 for x in samples);observations[compiler+'.benchmark']=samples;keep(binary);asm=run(compiler+'.benchmark.disassembly',['objdump','-dr',binary]);assert 'call' in asm and '<run' in asm
   objects=[]
   for name,source in [('dynamics',private/'dynamics.c'),('producer',producer)]:
    obj=build/(compiler+'.profileC.'+name+'.o');run(compiler+'.profileC.'+name+'.compile',[compiler,'-std=c17','-O2','-fno-stack-protector','-ffp-contract=off','-fno-fast-math','-fstack-usage',*includes,'-c',source,'-o',obj]);undefined=run(compiler+'.profileC.'+name+'.undefined',['nm','-u',obj]);assert {x.split()[-1] for x in undefined.splitlines()}<={'memcpy','memmove','memset','memcmp'},undefined;run(compiler+'.profileC.'+name+'.disassembly',['objdump','-dr',obj]);keep(obj);objects.append(obj)
   for name in ['bytes','memory_provider']:
    obj=build/(compiler+'.profileC.'+name+'.o');run(compiler+'.profileC.'+name+'.compile',[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-I'+str(ROOT/'labs/machine_image/freestanding_depth'),'-c',ROOT/('assessor/legacy_systems/freestanding_depth/'+name+'.c'),'-o',obj]);keep(obj);objects.append(obj)
   combined=build/(compiler+'.profileC.o');run(compiler+'.profileC.combine',['ld','-r',*objects,'-o',combined]);assert not run(compiler+'.profileC.closed',['nm','-u',combined]).strip();keep(combined)
   binary=build/(compiler+'.profileC.test');run(compiler+'.profileC.test.compile',[compiler,'-std=c17','-O2','-ffp-contract=off',*includes,public/'filter_test.c',private/'fixed.c',combined,'-lm','-o',binary]);run(compiler+'.profileC.execute',[binary],marker='PASS');keep(binary)
  binary=build/'gcc.environment.x87';run('gcc.environment.x87.compile',['gcc','-std=c17','-O3','-mfpmath=387','-fexcess-precision=fast','-ffp-contract=off',*includes,public/'environment.c',private/'sum.c','-pthread','-lm','-o',binary]);observations['gcc.environment.x87']=json.loads(run('gcc.environment.x87',[binary]));keep(binary);run('gcc.environment.x87.disassembly',['objdump','-d',binary])
  observations['finite-oracles']=json.loads(run('finite-oracles',['python3',private/'finite_oracles.py']))
  for p in build.glob('*.su'):keep(p);observations[p.name]=p.read_text()
  for p in build.glob('*.vectorization.txt'):keep(p);observations[p.name]=p.read_text()
 except Exception as error:failure=str(error)
 else:failure=None
 final=source_pins()
 if initial!=final:failure='source changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',scope='T17 author numerical mechanisms, actual frozen FIR/IIR composition and explicitly separate finite transfer derivations',timestamp=datetime.datetime.now(datetime.timezone.utc).isoformat(),environment=dict(platform=platform.platform(),machine=platform.machine()),source_sha256=final,outputs_sha256=outputs,commands=rows,observations=observations,failure=failure,limitations=['No external review or independent learner/delayed performance.','Finite tests and exact derivations do not prove absolute mastery or all numerical input behavior.','Binary64/rounding/compiler assumptions are narrower than ISO C17; relaxed and x87 observations are separate semantics.','Host batch timing includes call/barrier/sink/clock effects and supplies no universal speedup or physical target claim.','Profile C object audit is the selected x86-64 build; no unexecuted architecture helper closure is implied.','Superaccumulator and physical-unit/cache/model transfer dossiers are identified analyses, not falsely claimed C backends.','Exact published C17 text remains a distinct reference gap.'])
 (folder/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(rows),failure=failure)),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
