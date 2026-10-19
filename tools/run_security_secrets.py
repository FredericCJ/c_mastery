"""Synthetic token lifetimes/work, edition probes and separately hosted EVP bridge."""
import datetime,hashlib,json,pathlib,re,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_secrets';PUBLIC=ROOT/'labs/security_depth/secrets';PRIVATE=ROOT/'assessor/security_depth/secrets';BUILD=ROOT/'build'/('security-secrets-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
SSL=pathlib.Path('/home/frede/cmastery-security-sources/openssl-dev/sysroot');MUSL=pathlib.Path('/home/frede/cmastery-effects-tools/install-musl-1.2.5/bin/musl-gcc');PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['work','lifetime','addresses']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'fixtures/embedded/PR-T35-05.json',ROOT/'labs/replicated/hashing/hashing.h',ROOT/'labs/replicated/hashing/HASHING.md',ROOT/'assessor/replicated/hashing/hashing.c']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():
 files=[pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','openssl']]+[pathlib.Path('/usr/lib/x86_64-linux-gnu/libcrypto.so.3'),pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),MUSL,SSL.parent/'libssl-dev_3.5.5-1ubuntu3.7_amd64.deb']
 files += list((SSL/'usr/include').rglob('*.h'))
 return {str(p):sha(p) for p in sorted(set(files))}
def expected_commands():
 out={'kernel','libc','openssl','primitive.disassembly','fixture','musl.c23.unavailable'}
 for cc in ['gcc','clang']:
  out|={cc+'.'+n for n in ['version','c23.compile','c23.execute','c23.disassembly','c17.unavailable']}
  for profile,_ in PROFILES:
   t=cc+'.'+profile;out|={t+'.'+n for n in ['compile','stub.compile','object','undefined','disassembly','provider.compile','provider','provider.oracle','provider.dynamic','provider.stub.compile','provider.stub','provider.mutant.compile','provider.mutant']+CASES};out|={t+'.stub.'+case for case in CASES}
   for m in ['EARLY','CLEAR']:out|={t+'.mutation.'+m+'.compile',t+'.mutation.'+m}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[]
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC];core=[PRIVATE/'secrets.c',*[PUBLIC/n for n in ['platform.c','clearing.c','consume.c','tests.c']]];includes=['-I',SSL/'usr/include','-I',SSL/'usr/include/x86_64-linux-gnu'];crypto=pathlib.Path('/usr/lib/x86_64-linux-gnu/libcrypto.so.3')
 def compile(name,cc,flags,sources,binary,provider=False):run(name,[cc,*strict,*flags,*(includes+['-DSS_FIRST_PRINCIPLES','-I',ROOT/'labs/replicated/hashing'] if provider else []),*sources,*([ROOT/'assessor/replicated/hashing/hashing.c',crypto] if provider else []),'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('openssl',['openssl','version','-a']);run('primitive.disassembly',['objdump','-d','--disassemble=explicit_bzero','/usr/lib/x86_64-linux-gnu/libc.so.6'],marker='<explicit_bzero@@')
  run('fixture',[sys.executable,PRIVATE/'oracle.py'],marker='"status": "pass"')
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   binary=BUILD/(cc+'.c23');run(cc+'.c23.compile',[cc,'-std=c23','-Wall','-Wextra','-Werror','-Wpedantic','-O3',PUBLIC/'edition.c','-o',binary]);harness.OUTPUTS.add(binary);run(cc+'.c23.execute',[binary]);run(cc+'.c23.disassembly',['objdump','-d',binary],marker='memset_explicit');run(cc+'.c17.unavailable',[cc,*strict,'-fsyntax-only',PUBLIC/'edition.c'],1,'memset_explicit')
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,core,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    assembly=run(tag+'.disassembly',['objdump','-d',binary]);obj=BUILD/(tag+'.o');run(tag+'.object',[cc,*strict,*flags,'-c',PRIVATE/'secrets.c','-o',obj]);harness.OUTPUTS.add(obj);undefined=run(tag+'.undefined',['nm','-u',obj]);symbols=[line.split()[-1] for line in undefined.splitlines()]
    allowed={'ss_wipe'} if profile!='sanitized' else {'ss_wipe','__start_asan_globals','__stop_asan_globals'}|{x for x in symbols if x.startswith(('__asan_','__ubsan_'))};assert set(symbols)<=allowed,('unexpected component dependency',symbols)
    dead=re.search(r'<ss_dead_store>:\n(.*?)(?:\n\n|$)',assembly,re.S)[1];live=re.search(r'<ss_explicit_store>:\n(.*?)(?:\n\n|$)',assembly,re.S)[1];indexed=re.search(r'<ss_indexed>:\n(.*?)(?:\n\n|$)',assembly,re.S)[1];assert 'ss_wipe' in live
    if profile=='optimized':assert 'memset' not in dead and not re.search(r'\$0x0,.*\(%rsp\)',dead)
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[PUBLIC/'learner.c',*core[1:]],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'SS_ASSERT:')
    for m,case,marker in [('EARLY','work','out.work==4 && out.equal==(k==2)'),('CLEAR','lifetime','p.bytes[i]==0')]:
     mutant=BUILD/(tag+'.mutation.'+m);compile(tag+'.mutation.'+m+'.compile',cc,[*flags,'-DSS_MUTANT_'+m],core,mutant);run(tag+'.mutation.'+m,[mutant,case],41,marker)
    provider=BUILD/(tag+'.provider');ps=[PRIVATE/'provider.c',PUBLIC/'platform.c',PUBLIC/'provider_tests.c'];compile(tag+'.provider.compile',cc,flags,ps,provider,True);run(tag+'.provider',[provider],marker='OBS provider seven-failures');run(tag+'.provider.oracle',[sys.executable,PRIVATE/'oracle.py',BUILD/(tag+'.provider.log')],marker='"provider_vectors": 65');run(tag+'.provider.dynamic',['readelf','-dW',provider],marker='libcrypto.so.3')
    pbstub=BUILD/(tag+'.provider.stub');compile(tag+'.provider.stub.compile',cc,flags,[PUBLIC/'provider_stub.c',PUBLIC/'provider_tests.c'],pbstub);run(tag+'.provider.stub',[pbstub],41,'SS_ASSERT:')
    mutant=BUILD/(tag+'.provider.mutant');compile(tag+'.provider.mutant.compile',cc,[*flags,'-DSS_MUTANT_KEY'],ps,mutant,True);run(tag+'.provider.mutant',[mutant],41,'ledger.key_cleared==1')
    observations.append(dict(profile=tag,cases=CASES,changed_bytes=1024,early_work=[1,4,4],fixed_work=[4,4,4],address_count=8,address_trace_leaks=True,ordinary_clear_disassembly=dead,explicit_clear_disassembly=live,indexed_disassembly=indexed,component_undefined=symbols,provider_vectors=65,provider_faults=7,provider_owner_key_preserved=True,machine_constant_time=False,production_approval=False))
  run('musl.c23.unavailable',[MUSL,'-std=c23','-Wall','-Wextra','-Werror','-Wpedantic','-fsyntax-only',PUBLIC/'edition.c'],1,'memset_explicit');assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped source/tool inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,limits=['Only synthetic token[1,2,3,4], local fixed candidates and bounded original objects; no real credentials, external target or host policy change.','Logical work/address traces and inspected code do not establish constant-time machine behavior, physical leakage or complete erasure of registers/scratch/provider copies.','Core is caller-storage profileB with explicitly permitted platform wipe; malloc experiment belongs to hosted harness. OpenSSL bridge is separately declared hosted profileA, not a first-principles algorithm implementation.','Actual OpenSSL3.5.5/installed glibc and musl1.2.5 selected availability; N3096 is a C23 draft, exact final published C17/C23 source access remains open.','Provider fetch/context cleanup and owned key-copy erasure tested; opaque provider state/allocation/TLS/error queue and deployment approval remain outside these bounded controls.','Author execution is not independent review or delayed learner assessment; physical observation not performed.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
