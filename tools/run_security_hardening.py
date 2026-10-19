"""Actual original Linux binaries, runtime maps, costs and Arm helper closure."""
import datetime,hashlib,json,pathlib,re,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_hardening';PUBLIC=ROOT/'labs/security_depth/hardening';PRIVATE=ROOT/'assessor/security_depth/hardening';BUILD=ROOT/'build'/('security-hardening-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
MUSL=pathlib.Path('/home/frede/cmastery-effects-tools/install-musl-1.2.5');EFFECTS=MUSL.parent
HARD=['-fPIE','-pie','-Wl,-z,relro,-z,now,-z,noexecstack','-fstack-protector-strong','-D_FORTIFY_SOURCE=3']
PLAIN=['-fno-pie','-no-pie','-Wl,-z,norelro,-z,lazy,-z,noexecstack','-fno-stack-protector','-U_FORTIFY_SOURCE']
VARIANTS=[('gcc.plain','gcc',PLAIN),('gcc.hardened','gcc',HARD),('gcc.aggregate','gcc',['-fhardened','-Wno-error=hardened']),('clang.plain','clang',PLAIN),('clang.hardened','clang',HARD),('musl.static',str(MUSL/'bin/musl-gcc'),['-static','-fstack-protector-strong','-D_FORTIFY_SOURCE=3','-Wl,-z,relro,-z,now,-z,noexecstack'])]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'fixtures/embedded/PR-T35-04.json']
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():
 files=[pathlib.Path(shutil.which(x)).resolve() for x in ['gcc','clang','readelf','objdump','nm','size','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump']]
 files += [MUSL/n for n in ['bin/musl-gcc','lib/musl-gcc.specs','lib/libc.a','include/string.h','include/features.h']]
 files += [EFFECTS/'musl-1.2.5/include/string.h',EFFECTS/'glibc-2.41/string/bits/string_fortified.h',pathlib.Path('/usr/include/string.h'),pathlib.Path('/usr/include/features.h'),pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6')]
 return {str(p):sha(p) for p in sorted(set(files))}
def expected_commands():
 names={'kernel','libc','page.size','aslr.policy','gcc.version','clang.version','ld.version','arm.version','gcc.hardened.options','clang.unsupported','fixture','fixture.stub','arm.probe','arm.touch','arm.entry','arm.missing','arm.undefined','arm.handler','arm.link','arm.final.undefined','arm.disassembly','arm.headers'}
 for tag,_,_ in VARIANTS:
  names|={tag+'.'+k for k in ['compile','bounds','policy','headers','programs','dynamic','symbols','disassembly','size','link_trace','preprocessor','bench']}
  names|={tag+'.maps.'+str(i) for i in range(4)}
  if tag in ['gcc.hardened','gcc.aggregate','clang.hardened']:names.add(tag+'.defect')
 for cc in ['gcc','clang']:
  names|={cc+'.'+k for k in ['guard.compile','guard','sanitizer.compile','sanitizer.bounds','sanitizer.defect','stub.compile','stub.bounds','stub.policy','handler.compile','handler','handler.stub.compile','handler.stub','handler.mutant.compile','handler.mutant']}
  for m in ['BOUND','POLICY']:names|={cc+'.mutation.'+m+'.compile',cc+'.mutation.'+m}
 return names
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];target={}
 sources=[PRIVATE/'repair.c',PUBLIC/'flawed.c',PUBLIC/'consume.c',PUBLIC/'tests.c'];strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I',PUBLIC]
 def compile(ident,cc,flags,src,binary):run(ident,[cc,*strict,*flags,*src,'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);policy=int(run('aslr.policy',['cat','/proc/sys/kernel/randomize_va_space']).strip());assert policy in [0,1,2];assert int(run('page.size',['getconf','PAGESIZE']).strip())==4096
  for cc in ['gcc','clang']:run(cc+'.version',[cc,'--version'])
  run('ld.version',['ld','--version']);run('arm.version',['arm-none-eabi-gcc','--version']);run('gcc.hardened.options',['gcc','--help=hardened'],marker='-fstack-protector-strong');run('clang.unsupported',['clang','-fhardened','-fsyntax-only','-x','c','/dev/null'],1,"unknown argument: '-fhardened'")
  run('fixture',[sys.executable,PUBLIC/'audit_tests.py',PRIVATE/'audit.py',ROOT/'fixtures/embedded/PR-T35-04.json'],marker='OBS declared_audit');run('fixture.stub',[sys.executable,PUBLIC/'audit_tests.py',PUBLIC/'audit_stub.py',ROOT/'fixtures/embedded/PR-T35-04.json'],41,'SH_ASSERT:declared_fixture')
  for tag,cc,flags in VARIANTS:
   binary=BUILD/tag;compile(tag+'.compile',cc,['-O2',*flags],sources,binary)
   run(tag+'.bounds',[binary,'bounds'],marker='OBS bounds');run(tag+'.policy',[binary,'policy'],marker='unauthorized7to8 repaired')
   header=run(tag+'.headers',['readelf','-hW',binary]);program=run(tag+'.programs',['readelf','-lW',binary]);dynamic=run(tag+'.dynamic',['readelf','-dW',binary]);symbols=run(tag+'.symbols',['readelf','-sW',binary]);assembly=run(tag+'.disassembly',['objdump','-d',binary]);size=run(tag+'.size',['size',binary]);run(tag+'.link_trace',[cc,*strict,'-O2',*flags,*sources,'-o',BUILD/(tag+'.trace'),'-###'])
   ppflags=[f for f in flags if not f.startswith('-Wl,') and f not in ['-pie','-no-pie','-static']]
   pp=run(tag+'.preprocessor',[cc,*strict,'-O2',*ppflags,'-dM','-E',PUBLIC/'flawed.c']);bench=run(tag+'.bench',[binary,'bench'],marker='CHECKSUM 223200000');samples=[int(x) for x in re.findall(r'^SAMPLE \d+ (\d+)$',bench,re.M)];assert len(samples)==31
   maps=[run(tag+'.maps.'+str(i),[binary,'maps']) for i in range(4)];bases=[];map_permissions=[]
   for text in maps:
    rows=[line for line in text.splitlines() if str(binary) in line];assert rows;first=rows[0].split();base=int(first[0].split('-')[0],16);bases.append(base);map_permissions.append([x.split()[1] for x in rows]);assert all(not ('w' in x.split()[1] and 'x' in x.split()[1]) for x in rows)
    stack=[x for x in text.splitlines() if '[stack]' in x];assert len(stack)==1 and 'x' not in stack[0].split()[1]
   hardened=tag in ['gcc.hardened','gcc.aggregate','clang.hardened'];plain=tag.endswith('plain');pie='DYN' in re.search(r'Type:.*',header)[0];relro='GNU_RELRO' in program;now='BIND_NOW' in dynamic or re.search(r'FLAGS_1.*NOW',dynamic) is not None;guard='__stack_chk_fail' in symbols;checked='__memcpy_chk' in symbols
   if hardened:assert pie and relro and now and guard and checked and 'PIE' in dynamic
   if plain:assert not pie and not relro and not now and not guard and not checked and len(set(bases))==1
   if tag=='musl.static':assert guard and not checked and 'INTERP' not in program and 'no dynamic section' in dynamic
   optimized=re.search(r'<sh_optimized_away>:\n(.*?)(?:\n\n|$)',assembly,re.S)[1];assert '__stack_chk_fail' not in optimized and '%fs:' not in optimized
   site=re.search(r'<sh_flawed_copy>:\n(.*?)(?:\n\n|$)',assembly,re.S)[1]
   if hardened:
    assert '__memcpy_chk' in site and '__stack_chk_fail' in site
    run(tag+'.defect',[binary,'defect'],-6,'buffer overflow detected')
    # The page-covered RELRO prefix must actually be mapped read-only.
    match=re.search(r'GNU_RELRO\s+(0x\w+)\s+(0x\w+)\s+(0x\w+)\s+(0x\w+)\s+(0x\w+)',program);start=int(match[2],16)&~4095;end=(int(match[2],16)+int(match[5],16))&~4095;assert end>start
    for text,base in zip(maps,bases):
     for page in range(base+start,base+end,4096):
      rows=[x.split() for x in text.splitlines()];cover=[x for x in rows if int(x[0].split('-')[0],16)<=page<int(x[0].split('-')[1],16)];assert len(cover)==1 and 'w' not in cover[0][1]
   observations.append(dict(variant=tag,pie=pie,relro=relro,eager_binding=now,guard=guard,checked_call=checked,aslr_policy=policy,bases=bases,observed_base_change=len(set(bases))>1,permissions=map_permissions,optimized_away_unguarded=True,valid_copy_samples_ns=samples,iterations_per_sample=200000,checksum=223200000,size_output=size,flawed_copy_stack_adjustments=[x.strip() for x in site.splitlines() if '%rsp' in x or 'push' in x or 'pop' in x],dynamic_dependencies=re.findall(r'NEEDED.*\[(.*?)\]',dynamic),interpreter=re.findall(r'Requesting program interpreter: (.*?)\]',program),fortify_macros=[x for x in pp.splitlines() if 'FORTIFY' in x],data_only_violation=True,repair_passed=True))
  for cc in ['gcc','clang']:
   binary=BUILD/(cc+'.guard');compile(cc+'.guard.compile',cc,['-O2','-fstack-protector-strong','-mstack-protector-guard=global'],[PUBLIC/'target_probe.c',PUBLIC/'guard_control.c'],binary);run(cc+'.guard',[binary],42,'OBS actual_guard_mismatch_handler')
   binary=BUILD/(cc+'.sanitizer');compile(cc+'.sanitizer.compile',cc,['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-U_FORTIFY_SOURCE'],sources,binary);run(cc+'.sanitizer.bounds',[binary,'bounds'],marker='OBS bounds');run(cc+'.sanitizer.defect',[binary,'defect'],-6,'AddressSanitizer: stack-buffer-overflow')
   stub=BUILD/(cc+'.stub');compile(cc+'.stub.compile',cc,['-O2',*HARD],[PUBLIC/'learner.c',*sources[1:]],stub)
   for case in ['bounds','policy']:run(cc+'.stub.'+case,[stub,case],41,'SH_ASSERT:')
   for mutant,case,marker in [('BOUND','bounds','sh_copy(payload, 8, &out) == 0 && out == 36'),('POLICY','policy','sh_authorize(1,0,&n)==-1 && n==7')]:
    binary=BUILD/(cc+'.mutation.'+mutant);compile(cc+'.mutation.'+mutant+'.compile',cc,['-O2',*HARD,'-DSH_MUTANT_'+mutant],sources,binary);run(cc+'.mutation.'+mutant,[binary,case],41,marker)
   for kind,source,extra,expected in [('handler',PRIVATE/'target.c',[],0),('handler.stub',PUBLIC/'target_stub.c',[],41),('handler.mutant',PRIVATE/'target.c',['-DSH_MUTANT_RECORD'],41)]:
    binary=BUILD/(cc+'.'+kind);compile(cc+'.'+kind+'.compile',cc,['-O2','-fno-stack-protector',*extra],[source,PUBLIC/'target_tests.c'],binary);run(cc+'.'+kind,[binary],expected,'OBS bounded_failure_record' if expected==0 else 'SH_ASSERT:bounded_failure_record')
  arm=['arm-none-eabi-gcc',*strict,'-mcpu=cortex-m4','-mthumb','-O2','-ffreestanding','-fno-builtin','-fstack-protector-strong','-fdata-sections','-ffunction-sections']
  for name,file in [('probe',PUBLIC/'target_probe.c'),('touch',PUBLIC/'target_touch.c'),('entry',PUBLIC/'target_entry.c')]:run('arm.'+name,[*arm,'-c',file,'-o',BUILD/(name+'.o')]);harness.OUTPUTS.add(BUILD/(name+'.o'))
  objects=[BUILD/(n+'.o') for n in ['probe','touch','entry']];link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-Wl,--no-undefined','-T',PUBLIC/'target.ld',*objects]
  undefined=run('arm.undefined',['arm-none-eabi-nm','-u',BUILD/'probe.o']);assert '__stack_chk_guard' in undefined and '__stack_chk_fail' in undefined
  run('arm.missing',[*link,'-o',BUILD/'missing.elf'],1,'__stack_chk_fail')
  run('arm.handler',[*arm,'-fno-stack-protector','-c',PRIVATE/'target.c','-o',BUILD/'handler.o']);harness.OUTPUTS.add(BUILD/'handler.o')
  image=BUILD/'target.elf';run('arm.link',[*link,BUILD/'handler.o','-Wl,-Map='+str(BUILD/'target.map'),'-o',image]);harness.OUTPUTS.update([image,BUILD/'target.map'])
  unresolved=run('arm.final.undefined',['arm-none-eabi-nm','-u',image]);assert not unresolved.strip();disasm=run('arm.disassembly',['arm-none-eabi-objdump','-d',image]);headers=run('arm.headers',['readelf','-hlW',image]);assert 'INTERP' not in headers and 'DYNAMIC' not in headers and 'wfi' in disasm
  target=dict(isa='Cortex-M4 Thumb',missing_guard_link_rejected=True,undefined_symbols=[],loader=False,libc=False,fixed_teaching_guard=True,native_record_test=True,target_execution=False,physical=False)
  assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped source/tool inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,target=target,limits=['Original synthetic programs only; nine-byte bounds defect executed only in separate fortify/ASan sensitivity processes. No arbitrary execution/privilege escalation claim.','Read-only current ASLR policy and local maps; no host settings, permissions or hardware flashing changed.','Actual GCC15.2/Clang21 and selected installed glibc versus musl1.2.5; aggregate flags and runtime eligibility are version-specific.','31 raw local timing samples per variant, equal valid-copy contract; noise/startup/cache/system interference preclude universal overhead claims.','Arm guard handler link closure plus native bounded-record execution; no Arm startup/CPU/physical execution, no secure entropy or fault-tolerant logging guarantee.','ISO C17 baseline; GNU/ELF/Linux and compiler instrumentation are separate authorities. Exact final published C17/C23 sources, external review and independent delayed learner remain open.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
