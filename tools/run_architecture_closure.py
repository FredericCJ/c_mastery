"""Audit the entire portable T20 composition and its exact earlier providers."""
import datetime,hashlib,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/api_depth/architecture';PRIVATE=ROOT/'assessor/api_depth/architecture';FOLDER=ROOT/'research/v5/expansion/legacy_architecture'
UNITS=['core','queue','ports','machine','config','supervisor','indicator','borrow','ring','effects']
PROVIDERS=['assessor/containers_depth/sequence.c','assessor/foundations/foundation.c','assessor/api_depth/stream.c','assessor/api_depth/api_depth.c','assessor/api_depth/abstractions/registry.c','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c']
HEADERS=['labs/containers_depth/containers.h','labs/foundations/include/foundation.h','labs/api_depth/stream/stream.h','labs/api_depth/include/api_depth.h','labs/api_depth/abstractions/registry.h','labs/machine_image/freestanding_depth/free.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 files=[*[PRIVATE/(u+'.c') for u in UNITS],*PUBLIC.glob('*.h'),*[ROOT/x for x in PROVIDERS+HEADERS],pathlib.Path(__file__),PUBLIC/'composition_test.c',PUBLIC/'core_test.c',PUBLIC/'test.h']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in files}
def main():
 build=ROOT/'build/architecture-closure';build.mkdir(parents=True,exist_ok=True);initial=source_pins();commands=[];outputs={};observations={}
 def run(name,cmd):
  r=subprocess.run(list(map(str,cmd)),cwd=build,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);commands.append(dict(id=name,command=list(map(str,cmd)),passed=r.returncode==0,exit_code=r.returncode,output=r.stdout));print(name,'PASS' if not r.returncode else 'FAIL',flush=True);assert not r.returncode,(name,r.stdout);return r.stdout
 def keep(p):outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 includes=['-I'+str(p) for p in {PUBLIC,*[(ROOT/p).parent for p in HEADERS]}]
 try:
  for compiler in ['gcc','clang']:
   run(compiler+'.version',[compiler,'--version']);objects=[]
   for index,source in enumerate([*[PRIVATE/(u+'.c') for u in UNITS],*[ROOT/x for x in PROVIDERS]]):
    obj=build/(compiler+'.'+str(index)+'.o');run(compiler+'.object'+str(index),[compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage',*includes,'-c',source,'-o',obj]);objects.append(obj);keep(obj)
   combined=build/(compiler+'.composition.o');run(compiler+'.link',['ld','-r',*objects,'-o',combined]);undefined=run(compiler+'.closed',['nm','-u',combined]);assert not undefined.strip();run(compiler+'.disassembly',['objdump','-dr',combined]);run(compiler+'.symbols',['nm',combined]);keep(combined)
   for test in ['core','composition']:
    binary=build/(compiler+'.'+test);run(compiler+'.'+test+'.compile',[compiler,'-std=c17','-O2',*includes,PUBLIC/(test+'_test.c'),combined,'-o',binary]);observations[compiler+'.'+test]=run(compiler+'.'+test,[binary]);assert 'PASS' in observations[compiler+'.'+test];keep(binary)
  for p in build.glob('*.su'):observations[p.name]=p.read_text();keep(p)
 except Exception as error:failure=str(error)
 else:failure=None
 final=source_pins()
 if initial!=final:failure='source changed during closure audit'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),scope='Complete named profile-C composition including actual ring, checked extent/cursor, copied stream, registry and T11 helper providers',source_sha256=final,outputs_sha256=outputs,commands=commands,observations=observations,failure=failure,limitations=['Selected native GCC/Clang objects and explicit original helper sources; no implicit CRT or library in the relocatable composition.','Hosted test harness remains separately linked with libc.','Callback storage and synchronization obligations remain contractual; no arbitrary pointer or physical target guarantees.'])
 (FOLDER/'closure.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(commands),failure=failure)),flush=True);return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
