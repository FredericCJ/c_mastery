"""Source-bound atomic model, native ownership and strict target experiments."""
import hashlib,json,pathlib,re,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_atomics';BUILD=ROOT/'build/thread-atomics'
PUBLIC=ROOT/'labs/thread_depth/atomics';PRIVATE=ROOT/'assessor/thread_depth/atomics'
GROUPS={'publication':['graph','conflict','snapshot','handoff'],'operation':['operations','lockfree','model','dependency'],'cas':['orders','retry','ownership']}
MUTANTS={'graph':('GRAPH','publication[0] & (1u << 3)'), 'conflict':('CONFLICT','ta_data_race(cases[i][0]'), 'snapshot':('SNAPSHOT','TA_LIMIT && n == 77'), 'handoff':('HANDOFF','ta_slot_publish(&s, 99) == TA_LIMIT'), 'operations':('COUNTER','ta_counter_add(&c, 2) == UINT_MAX'), 'lockfree':('LOCKFREE','ta_lockfree_admit(1, 1, 1) == 0'), 'model':('MODEL','ta_litmus(l, m, a, b) == expected'), 'dependency':('DEPENDENCY','ta_dependency(1, 1, 0) == 0'), 'orders':('ORDER','ta_order_pair(s, f) == expected'), 'retry':('RETRY','r.desired == 21'), 'ownership':('OWNER','ownership_preserved')}
OLD=['fixtures/systems/PR-T25-03.json','tools/verify_systems_fixtures.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'labs/thread_depth/lifecycle/race_control.c',*[ROOT/p for p in OLD]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def main():
 BUILD.mkdir(parents=True,exist_ok=True);before=pins();failure=None;observations=[];external={};target=[]
 harness.BUILD=BUILD;harness.PUBLIC=PUBLIC;harness.PRIVATE=PRIVATE;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run
 sources=[PRIVATE/(n+'.c') for n in ['cas','publication','models']]
 def compile(cc,flags,name,items,binary):
  run(name,[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread',*(['-Wno-atomic-alignment'] if cc=='clang' else []),*flags,'-I',PUBLIC,*items,'-latomic','-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('environment.kernel',['uname','-a']);run('environment.libc',['ldd','--version'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-DTA_TSAN'])]:
    tag=cc+'.'+profile
    for group,cases in GROUPS.items():
     binary=BUILD/(tag+'.'+group);tests=PUBLIC/(group+'_tests.c');compile(cc,flags,tag+'.'+group+'.compile',sources+[tests],binary)
     for case in cases:run(tag+'.'+case,[binary,case],marker='OBS '+case)
     stub=BUILD/(tag+'.'+group+'.learner');compile(cc,flags,tag+'.'+group+'.learner.compile',[PUBLIC/'learner.c',tests],stub)
     for case in cases:run(tag+'.learner.'+case,[stub,case],41,'TA_ASSERT:')
     for case in cases:
      define,marker=MUTANTS[case];mutant=BUILD/(tag+'.mutation.'+case);compile(cc,flags+['-DTA_MUTANT_'+define],tag+'.mutation.'+case+'.compile',sources+[tests],mutant);run(tag+'.mutation.'+case,[mutant,case],41,marker)
    if profile=='tsan':
     race=BUILD/(tag+'.race');compile(cc,flags,tag+'.race.compile',[ROOT/'labs/thread_depth/lifecycle/race_control.c'],race);run(tag+'.race',[race],66,'WARNING: ThreadSanitizer: data race')
    observations.append(dict(profile=tag,cases=[c for cs in GROUPS.values() for c in cs],fence_execution=profile!='tsan',fence_exclusion='TSan does not model thread fences' if profile=='tsan' else None,independent_or_physical=False))
   binary=BUILD/(cc+'.c23');run(cc+'.c23.compile',[cc,'-std=c23','-Wall','-Wextra','-Werror','-Wpedantic','-pthread',*(['-Wno-atomic-alignment'] if cc=='clang' else []),'-O2','-I',PUBLIC,*sources,PUBLIC/'operation_tests.c','-latomic','-o',binary]);harness.OUTPUTS.add(binary);run(cc+'.c23.dependency',[binary,'dependency'],marker='OBS dependency')
  output=run('handoff.model',[sys.executable,PRIVATE/'handoff_model.py'],marker='"status": "pass"');finite=json.loads(output)
  run('baseline.abstract-closure',[sys.executable,'-c',"import json,sys;sys.path.insert(0,'tools');from verify_systems_fixtures import oracle;d=json.load(open('fixtures/systems/PR-T25-03.json'));assert oracle(d['oracle_type'],d['input'])==d['expected'];print('PASS retained abstract closure only')"],marker='PASS retained abstract closure only')
  sysroot=ROOT/'build/posix-io-toolchain/sysroot';libs=sysroot/'usr/lib32';loader=libs/'ld-linux.so.2';gccdir=sysroot/'usr/lib/gcc/x86_64-linux-gnu/15/32';binary=BUILD/'i386'
  flags=['-m32','--sysroot='+str(sysroot),'-isystem',str(sysroot/'usr/include/x86_64-linux-gnu'),'-B'+str(gccdir)+'/', '-L'+str(libs),'-Wl,--dynamic-linker='+str(loader),'-O2']
  compile('gcc',flags,'i386.compile',sources+[PUBLIC/'operation_tests.c'],binary)
  for case in GROUPS['operation']:run('i386.'+case,[loader,'--library-path',libs,binary,case],marker='OBS '+case)
  for p in [loader,*libs.glob('libatomic.so*'),*libs.glob('libc.so*')]:
   if p.is_file():external[str(p)]=sha(p)
  provider=pathlib.Path(run('host.libatomic.path',['gcc','-print-file-name=libatomic.so']).strip()).resolve();external[str(provider)]=sha(provider);run('host.libatomic.symbols',['nm','-D',provider],marker='__atomic_load')
  for cc in ['clang','arm-none-eabi-gcc']:
   run(cc+'.target-version',[cc,'--version'])
   for cpu in ['cortex-m4','cortex-m0']:
    tag=cc+'.'+cpu;flags=([cc,'--target=arm-none-eabi'] if cc=='clang' else [cc])+['-mcpu='+cpu,'-mthumb','-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-Wpedantic','-Werror',*(['-Wno-atomic-alignment'] if cc=='clang' else []),'-O2','-I',PUBLIC]
    probe=BUILD/(tag+'.probe.o');run(tag+'.probe.compile',flags+['-c',PUBLIC/'target.c','-o',probe]);harness.OUTPUTS.add(probe)
    undefined=run(tag+'.undefined',['arm-none-eabi-nm','-u',probe]);assembly=run(tag+'.disassembly',['arm-none-eabi-objdump','-dr',probe],marker='<target_native>')
    for symbol in ['target_consume','target_acquire','target_control','target_release_fence','target_signal_fence']:run(tag+'.'+symbol,['arm-none-eabi-objdump','-dr','--disassemble='+symbol,probe],marker='<'+symbol+'>')
    out=BUILD/(tag+'.selected.elf');native=cpu=='cortex-m4';entry='target_native' if native else 'target_native'
    args=['arm-none-eabi-ld','--gc-sections','--entry='+entry,probe,'-o',out]
    run(tag+'.native-link',args,0 if native else 1,None if native else '__atomic_fetch_add_4')
    if native:
     harness.OUTPUTS.add(out);assert not run(tag+'.native-undefined',['arm-none-eabi-nm','-u',out]).strip();assert re.search(r'\bldrex\b',assembly) and re.search(r'\bstrex\b',assembly),(tag,'native instruction evidence missing')
     objects=[]
     for source in sources:
      obj=BUILD/(tag+'.'+source.stem+'.o');run(tag+'.core.'+source.stem,flags+['-c',source,'-o',obj]);harness.OUTPUTS.add(obj);objects.append(obj)
     image=BUILD/(tag+'.core.elf');mapping=BUILD/(tag+'.core.map');run(tag+'.core.link',['arm-none-eabi-ld','--entry=ta_slot_read','-Map='+str(mapping),*objects,'-o',image]);harness.OUTPUTS.update([image,mapping]);assert not run(tag+'.core.undefined',['arm-none-eabi-nm','-u',image]).strip()
    for entry,marker in [('target_wide','__atomic_fetch_add_8'),('target_pair','__atomic_load')]:run(tag+'.reject.'+entry,['arm-none-eabi-ld','--gc-sections','--entry='+entry,probe,'-o',BUILD/(tag+'.rejected.elf')],1,marker)
    target.append(dict(compiler=cc,cpu=cpu,native_closure=native,undefined=undefined.splitlines(),execution='not-run',isr_authorization=False))
 except Exception as ex:failure=str(ex);finite=locals().get('finite')
 after=pins()
 if before!=after:failure='Scoped source changed during execution'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=after,external_sha256=external,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,finite_model=finite,target_audits=target,limits=['Author implementation/execution only; no external review, independent learner delay or mastery awarded.','Exact final C17 and published C23 passages remain unavailable; N1570/N3096 are drafts and compiler modes are separate observations.','The MP/SB model covers only the stated constant-store fragment; the handoff model covers two generations and fixed readers, not all C executions.','TSan fence execution is explicitly omitted because the tool does not model thread-fence synchronization; six other native profiles execute it.','Host libatomic is explicitly linked for aggregate probes; strict target core never imports it.','M4 image closure is compilation/link inspection only; M0 and widened operations are deliberately rejected without providers. No ISR safety, target execution, physical evidence or progress bound is claimed.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
