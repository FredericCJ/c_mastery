"""T22 source-bound author experiments. Run under WSL; no repository history edits."""
import argparse,datetime,hashlib,json,os,pathlib,re,shlex,shutil,subprocess,sys,tempfile,traceback
ROOT=pathlib.Path(__file__).resolve().parents[1];PUB=ROOT/'labs/api_depth/library';PRIVATE=ROOT/'assessor/api_depth/library';HERE=ROOT/'research/v5/expansion/legacy_packaging';BUILD=ROOT/'build/packaging-followthrough'
DOXY=pathlib.Path('/home/frede/cmastery-packaging-tools/root/usr/bin/doxygen')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def source_pins():
 files=[p for d in [PUB,PRIVATE] for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'assessor/api_depth/api_depth.c',ROOT/'labs/api_depth/include/api_depth.h']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
class Campaign:
 def __init__(self):self.commands=[];self.observations={};self.outputs=set();self.external={};self.start=source_pins();BUILD.mkdir(parents=True,exist_ok=True)
 def run(self,key,args,cwd=None,expect=0,marker=None,env=None):
  e=dict(os.environ)
  for variable in ['CPATH','C_INCLUDE_PATH','LIBRARY_PATH','CFLAGS','CPPFLAGS','LDFLAGS','CMAKE_PREFIX_PATH']:e.pop(variable,None)
  e.update(env or {});e['ASAN_OPTIONS']='detect_leaks=1:halt_on_error=1';e['UBSAN_OPTIONS']='halt_on_error=1';e['LC_ALL']='C'
  p=subprocess.run([str(a) for a in args],cwd=cwd or ROOT,env=e,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180)
  okay=(p.returncode==expect if isinstance(expect,int) else p.returncode in expect) and (marker is None or marker in p.stdout)
  self.commands.append(dict(id=key,argv=[str(x) for x in args],cwd=str(cwd or ROOT),exit_code=p.returncode,expected_exit=expect if isinstance(expect,int) else sorted(expect),marker=marker,output=p.stdout,passed=okay));print(key,'PASS' if okay else 'FAIL',flush=True)
  if not okay:raise AssertionError(key+'\n'+p.stdout)
  return p.stdout
 def keep(self,p):self.outputs.add(pathlib.Path(p));return p
 def compile(self,key,compiler,flags,test,mutant=0,learner=False,release=1,representation=1,extra=None):
  conf=BUILD/(key+'.include');conf.mkdir(exist_ok=True);(conf/'lc_private.h').write_text(f'#define LC_RELEASE {release}\n#define LC_REPRESENTATION {representation}\n');self.keep(conf/'lc_private.h');exe=self.keep(BUILD/key)
  code=[PUB/'learner/lc.c'] if learner else [PRIVATE/'lc.c',ROOT/'assessor/api_depth/api_depth.c']
  if test=='lifecycle':code += [PRIVATE/x for x in ['constructor.c','host.c','board.c']]
  args=[compiler,'-std=c17','-Wall','-Wextra','-Werror','-Wno-error=misleading-indentation',*flags,'-DLC_MUTANT='+str(mutant),'-I'+str(PUB/'include'),'-I'+str(PUB),'-I'+str(conf),'-I'+str(ROOT/'labs/api_depth/include'),PUB/'tests'/f'{test}.c',*code,*(extra or []),'-o',exe]
  self.run(key+'.compile',args);return exe
 def core(self):
  for tool,args in [('cmake',['cmake','--version']),('git',['git','--version']),('python',[sys.executable,'--version']),('binutils',['objcopy','--version']),('kernel',['uname','-srm'])]:self.run(tool+'.version',args)
  for cc in ['gcc','clang']:
   self.run(cc+'.version',[cc,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie'])]:
    pre=cc+'.'+profile
    for test in ['buffer','lifecycle','abi','rename','scheduler','allocation']:
     exe=self.compile(pre+'.'+test,cc,flags,test,extra=['-Wl,--wrap=malloc,--wrap=free'] if test=='allocation' else []);self.run(pre+'.'+test,[exe],marker='LC_')
    for mutant,test in [(1,'buffer'),(2,'lifecycle'),(3,'lifecycle')]:
     key=pre+'.mutant'+str(mutant);exe=self.compile(key,cc,flags,test,mutant);self.run(key,[exe],expect=1,marker='LC_ASSERT')
    key=pre+'.learner';exe=self.compile(key,cc,flags,'buffer',learner=True);self.run(key,[exe],expect=1,marker='LC_ASSERT')
   for representation in [1,2]:
    key=cc+'.representation'+str(representation);exe=self.compile(key,cc,['-O2'],'buffer',representation=representation);self.run(key,[exe],marker='LC_BUFFER')
   for release in [1,2]:
    key=cc+'.release'+str(release);exe=self.compile(key,cc,['-O2'],'abi',release=release);self.run(key,[exe],marker='LC_ABI')
   self.run(cc+'.units_rejected',[cc,'-std=c17','-Werror','-I'+str(PUB/'include'),'-c',PUB/'examples/units_invalid.c','-o',BUILD/(cc+'.invalid.o')],expect=1,marker='incompatible')
  key='gcc.lto';exe=self.compile(key,'gcc',['-O3','-flto'],'lifecycle');self.run(key,[exe],marker='LC_LIFECYCLE');self.keep(BUILD/'lto.disassembly').write_text(self.run('gcc.lto.disassembly',['objdump','-dr',exe]))
  self.run('trace',[sys.executable,PUB/'tools/trace.py']);self.observations['models']=json.loads(self.run('models',[sys.executable,PRIVATE/'models.py',BUILD/'migration']));self.outputs.update((BUILD/'migration').glob('*'))
 def package(self,tag,cc='gcc',shared=False,release=1,helper=False,flags='-O2 -g',backend='HOST'):
  for name in [tag+' source',tag+' build',tag+' source hidden',tag+' build hidden']:
   old=BUILD/name
   if old.exists():
    assert old.resolve().is_relative_to(BUILD.resolve());old.rename(BUILD/(name+'.previous.'+datetime.datetime.now().strftime('%Y%m%d%H%M%S%f')))
  stage=BUILD/(tag+' source');stage.mkdir(exist_ok=True);shutil.copytree(PUB,stage,dirs_exist_ok=True);implementation=stage/'private';implementation.mkdir(exist_ok=True)
  for name in ['lc.c','helper.c','host.c','board.c']:shutil.copy2(PRIVATE/name,implementation/name)
  producer=stage/'producer';producer.mkdir(exist_ok=True);shutil.copy2(ROOT/'assessor/api_depth/api_depth.c',producer/'api_depth.c');shutil.copy2(ROOT/'labs/api_depth/include/api_depth.h',producer/'api_depth.h')
  build=BUILD/(tag+' build');prefix=BUILD/(tag+' prefix');prefix.mkdir(exist_ok=True)
  cflags=flags+' '+shlex.quote(f'-ffile-prefix-map={stage}=/course/src')+' '+shlex.quote(f'-ffile-prefix-map={build}=/course/build')
  self.run(tag+'.configure',['cmake','-S',stage,'-B',build,'-DCMAKE_C_COMPILER='+cc,'-DCMAKE_C_FLAGS='+cflags,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DCMAKE_INSTALL_LIBDIR=lib','-DBUILD_SHARED_LIBS='+('ON' if shared else 'OFF'),'-DLC_RELEASE='+str(release),'-DLC_IMPLEMENTATION_DIR='+str(implementation),'-DLC_PROVIDER_ROOT='+str(producer),'-DLC_STATIC_HELPER='+('ON' if helper else 'OFF'),'-DLC_BACKEND='+backend])
  self.run(tag+'.build',['cmake','--build',build,'--verbose']);self.run(tag+'.install',['cmake','--install',build,'--prefix',prefix])
  compile_rows=json.loads((build/'compile_commands.json').read_text());selected={pathlib.Path(r['file']).name for r in compile_rows};assert ('host.c' in selected)==(backend in ['HOST','BOTH']) and ('board.c' in selected)==(backend in ['MODEL','BOTH']);self.observations[tag+'.backend']=dict(selected=backend,sources=sorted(selected),hardware_execution=False)
  self.keep(BUILD/(tag+'.compile_commands.json')).write_text(json.dumps(compile_rows,indent=2))
  installed=[p for p in prefix.rglob('*') if p.is_file()];assert (prefix/'include/lc.h').is_file() and not (prefix/'include/lc_private.h').exists()
  for p in prefix.rglob('*.cmake'):
   text=p.read_text();assert str(stage) not in text and str(build) not in text
  self.outputs.update(installed);manifest=self.keep(BUILD/(tag+'.manifest.json'));manifest.write_text(json.dumps({p.relative_to(prefix).as_posix():sha(p) for p in installed},indent=2))
  # Rename only these verified generated workspace directories. No real checkout
  # is touched, and independent consumer compilation has no source include path.
  for p in [stage,build]:assert p.resolve().is_relative_to(BUILD.resolve());p.rename(p.with_name(p.name+' hidden'))
  copied=BUILD/(tag+' relocated prefix with spaces');shutil.copytree(prefix,copied,dirs_exist_ok=True)
  client=BUILD/(tag+' outside consumer');client.mkdir(exist_ok=True);shutil.copy2(PUB/'consumer/CMakeLists.txt',client/'CMakeLists.txt');shutil.copy2(PUB/'examples/client.c',client/'client.c')
  cb=client/'build';self.run(tag+'.consumer_configure',['cmake','-S',client,'-B',cb,'-DCMAKE_C_COMPILER='+cc,'-DCMAKE_PREFIX_PATH='+str(copied)])
  self.run(tag+'.consumer_build',['cmake','--build',cb,'--verbose']);self.run(tag+'.consumer',[cb/'consumer'],marker='LC_CONSUMER')
  self.keep(cb/'consumer');lib=next((copied/'lib').glob('libcourse.so.*')) if shared else copied/'lib/libcourse.a'
  self.run(tag+'.symbols',['nm','-g',lib]);self.run(tag+'.dependencies',['readelf','-d',lib]);self.outputs.add(lib)
  if shared:
   symbols=self.run(tag+'.exports',['nm','-D','--defined-only',lib]);names=[line.split()[-1] for line in symbols.splitlines() if line.split()];assert names and all(x.startswith('lc_') for x in names)
   runtime=self.run(tag+'.runtime',['ldd',lib]);paths=re.findall(r'(?:=>\s+)?(/\S+)',runtime)
   assert any('libc.so' in x for x in paths)
   for name in paths:
    p=pathlib.Path(name)
    if p.is_file():self.external[name]=sha(p)
   self.observations[tag+'.provenance']=dict(target_source='original adapter plus hash-pinned earlier struct api_buffer producer',runtime_files=paths,host_tools=['CMake','Doxygen','Python','Git','compiler/binutils'],distribution_permission='unresolved; local experimental artifacts only',oracle='independently formulated Python byte-sequence model; never called by target')
   self.observations[tag+'.ffi']=json.loads(self.run(tag+'.ffi',[sys.executable,PUB/'tests/ffi.py',lib]))
   debug=self.keep(BUILD/(tag+'.debug'));stripped=self.keep(BUILD/(tag+'.stripped.so'));shutil.copy2(lib,stripped)
   self.run(tag+'.debug_extract',['objcopy','--only-keep-debug',lib,debug]);self.run(tag+'.strip',['strip','--strip-debug',stripped]);self.run(tag+'.debuglink',['objcopy','--add-gnu-debuglink='+str(debug),stripped]);a=self.run(tag+'.build_id',['readelf','-n',stripped]);b=self.run(tag+'.debug_id',['readelf','-n',debug]);aid=re.search(r'Build ID: (\w+)',a);bid=re.search(r'Build ID: (\w+)',b);assert aid and bid and aid[1]==bid[1];self.observations[tag+'.debug_identity']=aid[1]
  if helper:
   (copied/'lib/libhelper.a').rename(copied/'lib/libhelper.a.hidden');self.run(tag+'.missing_dependency',['cmake','-S',client,'-B',client/'missing','-DCMAKE_PREFIX_PATH='+str(copied)],expect=1,marker='does not exist');(copied/'lib/libhelper.a.hidden').rename(copied/'lib/libhelper.a')
  # Diagnostic-only private and vendor includes: the installed boundary rejects
  # each without needing any board toolchain or guessed vendor header.
  for name in ['lc_private.h','vendor_only.h']:
   bad=BUILD/(tag+'.'+name+'.c');bad.write_text('#include <'+name+'>\n');self.keep(bad);self.run(tag+'.reject.'+name,[cc,'-I'+str(copied/'include'),'-c',bad,'-o',BUILD/'invalid.o'],expect=1,marker='No such file' if cc=='gcc' else 'file not found')
  self.outputs.update(p for p in copied.rglob('*') if p.is_file());return copied,lib
 def release_replacement(self,first,second):
  client=BUILD/'release1 outside consumer/build/consumer'
  for tag,prefix in [('old',first),('replacement',second),('rollback',first)]:self.run('deployment.'+tag,[client],env={'LD_LIBRARY_PATH':str(prefix/'lib')},marker='LC_CONSUMER')
  self.observations['deployment']=dict(policy='immutable isolated prefixes, selected loader search for each fresh process; no running-process replacement',rollback_verified=True,old_artifact_preserved=True)
  bad=BUILD/'missing-developer-path';self.run('minimal.missing_private_dependency',['cmake','-S',PUB,'-B',bad,'-DLC_PROVIDER_ROOT=/nonexistent/cmastery-developer-private'],expect=1,marker='Cannot find source file')
  self.run('bad_example.compile',['gcc','-I'+str(first/'include'),PUB/'examples/bad_example.c','-L'+str(first/'lib'),'-lcourse','-o',self.keep(BUILD/'bad_example')]);self.run('bad_example.reject',[BUILD/'bad_example'],env={'LD_LIBRARY_PATH':str(first/'lib')},expect=17)
 def documentation(self):
  env={'LD_LIBRARY_PATH':'/home/frede/cmastery-packaging-tools/root/usr/lib/x86_64-linux-gnu'};self.run('doxygen.version',[DOXY,'--version'],env=env)
  for mode in ['before','after']:
   header=self.keep(BUILD/(mode+'.h'));text=(PUB/'include/lc.h').read_text()
   if mode=='after':
    text=re.sub(r'struct lc_buffer \*buffer,\s*const unsigned char \*borrowed_bytes,\s*size_t length_bytes','struct lc_buffer *buffer /**< [in] Live library-owned object; may not alias input. */,\n                         const unsigned char *borrowed_bytes /**< [in] Readable byte span; null only at length0. */,\n                         size_t length_bytes /**< [in] Byte count, never element count. */',text)
    assert '/**< [in] Byte count' in text
    text=re.sub(r' \* @param (?:buffer|borrowed_bytes|length_bytes).*\n','',text)
   header.write_text(text);output=BUILD/('doxy-'+mode);pages=self.keep(BUILD/(mode+'.dox'));pages.write_text('/** @page architecture Architecture decisions\n'+(PUB/'ARCHITECTURE.md').read_text()+'\n*/\n/** @page exercise_contract Exercise contract\n'+(PUB/'CONTRACTS.md').read_text()+'\n*/\n');config=self.keep(BUILD/('Doxyfile.'+mode));config.write_text((PUB/'Doxyfile.in').read_text().replace('@HEADER@',str(header)).replace('@OUTPUT@',str(output)).replace('@ARCHDOC@',str(pages)))
   self.run('doxygen.'+mode,[DOXY,config],env=env);xml='\n'.join(p.read_text() for p in (output/'xml').glob('*.xml'));assert 'Byte count, never element count' in xml and 'rejection preserves' in xml
   self.keep(output/'xml/index.xml');self.keep(output/'html/index.html');self.keep(output/'html/architecture.html');self.keep(output/'html/exercise_contract.html');html=(output/'html'/(mode+'_8h.html')).read_text();assert 'href="architecture.html"' in html and 'href="exercise_contract.html"' in html;self.observations['doxygen.'+mode]={'parameter_documentation_present':True,'contract_present':True,'architecture_and_contract_links_resolve':True}
  for p in [DOXY,DOXY.parents[1]/'share/doc/doxygen/copyright',pathlib.Path('/usr/share/doc/cmake/copyright'),pathlib.Path('/usr/share/doc/libc6/copyright')]:self.external[str(p)]=sha(p)
  for p in pathlib.Path('/home/frede/cmastery-packaging-tools/downloads').glob('*.deb'):self.external[str(p)]=sha(p)
 def abi32(self):
  sysroot=ROOT/'build/posix-io-toolchain/sysroot';libs=sysroot/'usr/lib32';gccdir=sysroot/'usr/lib/gcc/x86_64-linux-gnu/15/32';loader=libs/'ld-linux.so.2'
  flags=['-O2','-m32','--sysroot='+str(sysroot),'-isystem',str(sysroot/'usr/include/x86_64-linux-gnu'),'-B'+str(gccdir)+'/', '-L'+str(libs),'-Wl,--dynamic-linker='+str(loader)]
  exe=self.compile('gcc.abi32','gcc',flags,'abi');self.run('gcc.abi32',[loader,'--library-path',libs,exe],marker='pointer=4 size_t=4');self.run('gcc.abi32.elf',['readelf','-h',exe],marker='ELF32')
  for p in [loader,*libs.glob('libc.so*')]:
   if p.is_file():self.external[str(p)]=sha(p)
 def history(self):
  for ambiguous in [False,True]:
   tag='bisect.ambiguous' if ambiguous else 'bisect.exact';repo=pathlib.Path(tempfile.mkdtemp(prefix=tag+'.',dir=BUILD));out=repo.parent/(repo.name+'.work');out.mkdir()
   assert repo.resolve().is_relative_to(BUILD.resolve())
   self.run(tag+'.init',['git','init',repo]);commits=[]
   # No git operation targets the actual curriculum checkout. --no-checkout
   # lets the predicate read revisions without reset or working-tree rewrites.
   for i,mode in enumerate([0,0,125,2,2] if ambiguous else [0,125,0,2,2]):
    (repo/'mode.h').write_text(('\n#error intentionally unbuildable\n' if mode==125 else '\n#define API_MUTANT '+str(mode)+'\n')+'/* original teaching revision '+str(i)+' */\n')
    self.run(tag+'.add'+str(i),['git','add','mode.h'],cwd=repo)
    self.run(tag+'.commit'+str(i),['git','-c','user.name=Curriculum Author','-c','user.email=curriculum-author@example.invalid','commit','-m','original teaching revision '+str(i)],cwd=repo)
    commits.append(self.run(tag+'.id'+str(i),['git','rev-parse','HEAD'],cwd=repo).strip())
   self.run(tag+'.start',['git','bisect','start','--no-checkout',commits[-1],commits[0]],cwd=repo)
   output=self.run(tag+'.run',['git','bisect','run',sys.executable,PUB/'tools/bisect_predicate.py',repo,out,ROOT/'assessor/api_depth/api_depth.c',ROOT/'labs/api_depth/include',PUB/'tests/bisect_case.c'],cwd=repo,expect={0,2} if ambiguous else 0)
   if ambiguous:assert 'first bad commit could be any of' in output and commits[2] in output and commits[3] in output
   else:assert commits[3]+' is the first bad commit' in output
   self.keep(out/'predicate.jsonl');log=self.keep(out/'bisect.log');log.write_text(self.run(tag+'.log',['git','bisect','log'],cwd=repo));self.observations[tag]=dict(commits=commits,culprits=commits[2:4] if ambiguous else [commits[3]],scope='disposable original local history only')
   # Explicitly demonstrate125 even if the search's exact path skips that point.
   self.run(tag+'.untestable_ref',['git','update-ref','BISECT_HEAD',commits[2 if ambiguous else 1]],cwd=repo)
   self.run(tag+'.untestable',[sys.executable,PUB/'tools/bisect_predicate.py',repo,out,ROOT/'assessor/api_depth/api_depth.c',ROOT/'labs/api_depth/include',PUB/'tests/bisect_case.c'],expect=125,marker='intentionally unbuildable')
  exe=self.keep(BUILD/'bisect_minimize');self.run('bisect.minimize.compile',['gcc','-std=c17','-DAPI_MUTANT=2','-I'+str(ROOT/'labs/api_depth/include'),PUB/'tests/bisect_case.c',ROOT/'assessor/api_depth/api_depth.c','-o',exe])
  for n in [9,5,3,2]:self.run('bisect.minimize.'+str(n),[exe,str(n)],expect=17,marker='SAME_FAILURE')
  self.run('bisect.minimize.1',[exe,'1']);self.observations['bisect.minimized']=dict(input_length=2,capacity=1,prior_length=0,one_shorter_does_not_fail=True)
 def finish(self,error=None):
  stable=self.start==source_pins();status='pass' if error is None and stable else 'fail';HERE.mkdir(parents=True,exist_ok=True)
  report=dict(schema_version=1,status=status,recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),failure=error or (None if stable else 'source changed during campaign'),source_sha256=self.start,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(self.outputs) if p.is_file()},external_source_sha256=self.external,commands=self.commands,observations=self.observations,limits=['Author execution only; no external review or learner performance.','Teaching board backend is a hosted state model, not physical observation.','Local package/ABI findings apply to recorded Linux toolchains; distribution/license permission remains unresolved.'])
  (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(status=status,commands=len(self.commands),failure=report['failure'])));return status=='pass'
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--core-only',action='store_true');a=parser.parse_args();c=Campaign()
 try:
  c.core()
  if not a.core_only:
   first,one=c.package('release1',shared=True);second,two=c.package('release2',shared=True,release=2);_,same=c.package('release1_repeat',shared=True);assert sha(one)==sha(same);c.observations['same_compiler_reproducibility']=dict(equal=True,sha256=sha(one))
   _,changed=c.package('clang_release1',cc='clang',shared=True);c.observations['changed_compiler']=dict(byte_equal=sha(one)==sha(changed),gcc_sha256=sha(one),clang_sha256=sha(changed),consumer_revalidated=True)
   c.package('static',helper=True);c.package('model_backend',backend='MODEL');c.package('both_backends',backend='BOTH');c.package('gcc_lto_package',shared=True,flags='-O3 -flto -g');c.release_replacement(first,second);c.documentation();c.abi32();c.history();c.observations['metrics']=json.loads(c.run('metrics',[sys.executable,PUB/'tools/metrics.py',PRIVATE/'constructor.c']))
  return 0 if c.finish() else 1
 except Exception as exc:c.finish(str(exc));traceback.print_exc();return 1
if __name__=='__main__':raise SystemExit(main())
