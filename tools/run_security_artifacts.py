"""Actual artifact policy/signatures, composed service, reproducibility counterexample."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_artifacts';PUBLIC=ROOT/'labs/security_depth/artifacts';PRIVATE=ROOT/'assessor/security_depth/artifacts';BUILD=ROOT/'build'/('security-artifacts-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));SSL=pathlib.Path('/home/frede/cmastery-security-sources/openssl-dev/sysroot')
PROFILES=[('debug',['-O0','-g']),('optimized',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])];CASES=['artifact','rotation','service','shared','integrated']
IMPORTS=['tools/run_thread_lifecycle.py','labs/security_depth/command/command.h','labs/security_depth/command/CONTRACTS.md','assessor/security_depth/command/command.c','assessor/security_depth/command/service.c','labs/security_depth/worker/worker.h','labs/security_depth/worker/CONTRACTS.md','assessor/security_depth/worker/worker.c','labs/security_depth/secrets/secrets.h','labs/security_depth/secrets/platform.c','labs/security_depth/secrets/CONTRACTS.md','fixtures/embedded/PR-T35-06.json']
MUTANTS={'TRUST':('artifact','r.signature_valid==1 && r.trusted_signer==0 && r.ready==0'),'ROTATE':('rotation','sa_rotate(&t,pub,2,3,signature)==-1'),'QUOTA':('service','sa_submit(&s,&r,&a,resource,sizeof resource,&second)==SC_BUDGET')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
 files=[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),*[ROOT/n for n in IMPORTS]]
 return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():
 files=[pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','objdump','readelf','openssl']]+[pathlib.Path('/usr/lib/x86_64-linux-gnu/libcrypto.so.3'),pathlib.Path('/usr/lib/x86_64-linux-gnu/libc.so.6'),SSL.parent/'libssl-dev_3.5.5-1ubuntu3.7_amd64.deb']+list((SSL/'usr/include').rglob('*.h'))
 return {str(p):sha(p) for p in sorted(set(files))}
def expected_commands():
 out={'kernel','libc','openssl','fixture'}
 for cc in ['gcc','clang']:
  out.add(cc+'.version')
  for p,_ in PROFILES:
   tag=cc+'.'+p;out|={tag+'.'+n for n in ['compile','dynamic','stub.compile']+CASES};out|={tag+'.stub.'+n for n in CASES}
   for m in MUTANTS:out|={tag+'.mutation.'+m+'.compile',tag+'.mutation.'+m}
   for f in range(1,5):out|={tag+'.failure.'+str(f)+'.compile',tag+'.failure.'+str(f)}
  for build in ['first','second','repair']:out|={cc+'.release.'+build+'.'+n for n in ['compile','execute','disassembly','sections']}
 return out
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None;observations=[];repro=[]
 includes=[PUBLIC,ROOT/'labs/security_depth/command',ROOT/'labs/security_depth/worker',ROOT/'labs/security_depth/secrets',SSL/'usr/include',SSL/'usr/include/x86_64-linux-gnu'];strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*[item for p in includes for item in ['-I',p]]];crypto=pathlib.Path('/usr/lib/x86_64-linux-gnu/libcrypto.so.3');imports=[ROOT/n for n in IMPORTS if n.endswith('.c')]
 sources=[PRIVATE/'artifacts.c',PRIVATE/'service.c',*imports,PUBLIC/'tests.c']
 def compile(name,cc,flags,src,binary,provider=True):run(name,[cc,*strict,*flags,*src,*([crypto] if provider else []),'-o',binary]);harness.OUTPUTS.add(binary)
 try:
  run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('openssl',['openssl','version','-a']);run('fixture',[sys.executable,PRIVATE/'fixture.py'],marker='"status": "pass"')
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for profile,flags in PROFILES:
    tag=cc+'.'+profile;binary=BUILD/tag;compile(tag+'.compile',cc,flags,sources,binary)
    for case in CASES:run(tag+'.'+case,[binary,case],marker='OBS '+case)
    run(tag+'.dynamic',['readelf','-dW',binary],marker='libcrypto.so.3')
    stub=BUILD/(tag+'.stub');compile(tag+'.stub.compile',cc,flags,[PUBLIC/'learner.c',*imports,PUBLIC/'tests.c'],stub)
    for case in CASES:run(tag+'.stub.'+case,[stub,case],41,'SA_ASSERT:')
    for name,(case,marker) in MUTANTS.items():
     mutant=BUILD/(tag+'.mutation.'+name);compile(tag+'.mutation.'+name+'.compile',cc,[*flags,'-DSA_MUTANT_'+name],sources,mutant);run(tag+'.mutation.'+name,[mutant,case],41,marker)
    for f in range(1,5):
     binary=BUILD/(tag+'.failure.'+str(f));compile(tag+'.failure.'+str(f)+'.compile',cc,[*flags,'-DSA_PROVIDER_FAILURE='+str(f)],sources,binary);run(tag+'.failure.'+str(f),[binary,'provider_failure'],marker='OBS provider_failure')
    observations.append(dict(profile=tag,cases=CASES,actual_ed25519=True,adjacent_digest_replace=True,untrusted_actual_signature=True,rotation_revokes_old=True,commit_separate=True,fresh_trust_adapter=True,cached_report_counterexample=True,command_signature=False,provider_faults=4,new_opcode=6,resource_byte_budget=4,ordinary_work_budget=6,changed_widths=[1,2],same_process_authority_mutation=True,host_compromise=False,physical=False))
   binaries=[]
   for build in ['first','second','repair']:
    binary=BUILD/(cc+'.release.'+build);flags=['-O2','-Wl,--build-id=none']+([] if build=='repair' else ['-DSA_RELEASE_FLAW']);tag=cc+'.release.'+build;compile(tag+'.compile',cc,flags,[PUBLIC/'release_probe.c'],binary,False);run(tag+'.execute',[binary],marker='synthetic_effect='+('0' if build=='repair' else '1'));run(tag+'.disassembly',['objdump','-d',binary]);run(tag+'.sections',['readelf','-SW',binary]);binaries.append(binary)
   assert sha(binaries[0])==sha(binaries[1]) and sha(binaries[0])!=sha(binaries[2]);repro.append(dict(compiler=cc,first=binaries[0].relative_to(ROOT).as_posix(),second=binaries[1].relative_to(ROOT).as_posix(),repair=binaries[2].relative_to(ROOT).as_posix(),same_bytes=True,reproduced_unauthorized_effect=1,repair_effect=0,trustworthiness_inferred=False))
  assert sha(ROOT/repro[0]['first'])!=sha(ROOT/repro[1]['first']);assert {x['id'] for x in harness.COMMANDS}==expected_commands()
 except Exception as ex:failure=str(ex)
 if before!=pins() or ext!=external():failure='Scoped source/tool inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},commands=harness.COMMANDS,observations=observations,reproducibility=repro,limits=['Only original local synthetic payloads, fixed public teaching seeds and command effects; no production keys, external targets, host policy changes or firmware flashing.','OpenSSL3.5.5 implements expressly permitted signature/hash boundary; this is policy/ownership composition, not a new cryptographic algorithm or provider security certification.','Commit is a trusted logical local fact, not evidence of durable flash installation; rotation assumes authentic protected initial trust state and serialized ownership.','Shared-memory attacker can change authority/trust/report fields; logical API checks are not a process isolation boundary.','Two builds reproduce the same original flawed source; cross-toolchain binary difference is investigated through commands/sections/disassembly/semantics, not preferred hash selection or independence claim.','Current bounded source-bound author execution, not external review, independent delayed learner or physical observation; exact final published C17/C23 remains open.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
