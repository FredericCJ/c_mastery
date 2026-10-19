"""Execute exact original streaming matcher publication/cancellation composition."""
import datetime,json,pathlib,sys
import run_thread_lifecycle as harness
import run_certificate_transfer as common
ROOT=common.ROOT;PUBLIC=ROOT/'labs/match_transfer';PRIVATE=ROOT/'assessor/match_transfer';HERE=ROOT/'research/v5/expansion/match_transfer';BUILD=ROOT/'build'/('match-transfer-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=common.PROFILES;MUTANTS=[('SKIP','0','sink.count==5'),('REPLACE','0','==MT_BUSY'),('CANCEL','1','==AL_CANCELLED')]
IMPORTS=['assessor/algorithm_lookup/lookup.c','assessor/algorithm_depth/matching.c','labs/algorithm_lookup/lookup.h','labs/algorithm_depth/algorithms.h','labs/containers_depth/containers.h','labs/algorithm_frontier/link.ld','assessor/execution/support.c','tools/run_thread_lifecycle.py','tools/run_certificate_transfer.py']
def pins():return {p.relative_to(ROOT).as_posix():common.sha(p) for p in sorted({pathlib.Path(__file__),*[ROOT/n for n in IMPORTS],*[p for base in [PUBLIC,PRIVATE] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})}
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=common.external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections',*sum((['-I',ROOT/p] for p in ['labs/match_transfer','labs/algorithm_lookup','labs/algorithm_depth','labs/containers_depth']),[])]
 def src(stub=False):return [PUBLIC/'learner.c' if stub else PRIVATE/'match.c',*[ROOT/n for n in IMPORTS[:2]]]
 def build(tag,cc,flags,sources):
  image=BUILD/tag;run(tag+'.build',[cc,*strict,*flags,*sources,'-Wl,--gc-sections','-o',image]);harness.OUTPUTS.add(image);return image
 try:
  run('kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for mode,flags in PROFILES:
    tag=cc+'.'+mode;image=build(tag,cc,flags,[*src(),PUBLIC/'tests.c',PUBLIC/'native.c'])
    for case in ['0','1','2']:run(tag+'.case'+case,[image,case],marker='MT_PASS')
    for name,case,marker in MUTANTS:
     bad=build(tag+'.mutant.'+name,cc,[*flags,'-DMT_MUTANT_'+name],[*src(),PUBLIC/'tests.c',PUBLIC/'native.c']);run(tag+'.reject.'+name,[bad,case],41,marker)
    stub=build(tag+'.stub',cc,flags,[*src(True),PUBLIC/'tests.c',PUBLIC/'native.c'])
    for case in ['0','1','2']:run(tag+'.stub.reject'+case,[stub,case],41,'MT_ASSERT:')
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target.version',[cc,'--version']);target=['-mcpu=cortex-m4','-mthumb']+(['--target=arm-none-eabi','-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib'] if cc=='clang' else [])
   for mode in ['O0','O3']:
    tag=cc+'.target.'+mode;objects=[]
    for index,file in enumerate([*src(),PUBLIC/'tests.c',PUBLIC/'target.c',ROOT/'assessor/execution/support.c']):
     obj=BUILD/(tag+'.'+str(index)+'.o');objects.append(obj);run(tag+'.object'+str(index),[cc,*strict,*target,'-'+mode,'-ffreestanding','-fno-builtin','-fstack-usage','-c',file,'-o',obj]);harness.OUTPUTS.add(obj);harness.OUTPUTS.add(obj.with_suffix('.su'));run(tag+'.undefined'+str(index),['arm-none-eabi-nm','-u',obj]);run(tag+'.assembly'+str(index),['arm-none-eabi-objdump','-d',obj])
    image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.link',['arm-none-eabi-ld','-T',ROOT/'labs/algorithm_frontier/link.ld','--gc-sections','--no-undefined','-Map='+str(mapping),*objects,'-o',image]);harness.OUTPUTS.update([image,mapping]);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();run(tag+'.size',['arm-none-eabi-size',image]);run(tag+'.assembly',['arm-none-eabi-objdump','-d',image]);run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],marker='MT_TARGET_PASS')
 except Exception as e:failure=str(e)
 if before!=pins() or ext!=common.external():failure='Inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,commands=harness.COMMANDS,outputs_sha256={p.relative_to(ROOT).as_posix():common.sha(p) for p in sorted(harness.OUTPUTS)},limits=['Exact original X-T36-003 callback streaming composition; frozen prefix algorithm and adapter imported.','Original local synthetic callbacks only; synchronous cancellation and borrowed context, not asynchronous/OS cancellation or arbitrary-effect rollback.','All selected objects and images audited; original byte helpers only; unused provider references appear in object audits but are discarded from selected image.','Finite author execution and emulation are not physical observation, independent review or delayed learner performance.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return bool(failure)
if __name__=='__main__':raise SystemExit(main())
