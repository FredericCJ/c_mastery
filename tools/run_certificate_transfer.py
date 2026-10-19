"""Actual original X-T36-001 V2 / X-T36-002 V1 composition and controls."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as harness
ROOT=pathlib.Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/certificate_transfer';PRIVATE=ROOT/'assessor/certificate_transfer';HERE=ROOT/'research/v5/expansion/certificate_transfer'
BUILD=ROOT/'build'/('certificate-transfer-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('O0',['-O0','-g']),('O3',['-O3']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS=[('EARLY','0','==CT_MORE'),('UNSIGNED','0','==CT_OK'),('BOUNDS','1','==ALGO_BAD'),('HEIGHT','1','==ALGO_BAD')]
IMPORTS=['assessor/algorithm_frontier/frontier.c','assessor/algorithm_tree/certificate.c','assessor/containers_depth/indexed.c','labs/algorithm_frontier/frontier.h','labs/algorithm_tree/tree.h','labs/algorithm_depth/algorithms.h','labs/containers_depth/containers.h','labs/algorithms/include/algo.h','labs/algorithm_frontier/link.ld','assessor/execution/support.c','tools/run_thread_lifecycle.py']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted({pathlib.Path(__file__),*[ROOT/n for n in IMPORTS],*[p for base in [PUBLIC,PRIVATE] for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]})}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump','size','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','arm-none-eabi-size','qemu-system-arm']})}
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();ext=external();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();run=harness.run;failure=None
 includes=sum((['-I',ROOT/p] for p in ['labs/certificate_transfer','labs/algorithm_frontier','labs/algorithm_tree','labs/algorithm_depth','labs/containers_depth','labs/algorithms/include']),[])
 strict=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections',*includes]
 providers=[ROOT/n for n in IMPORTS[:3]]
 def src(stub=False):return ([PUBLIC/'learner.c'] if stub else [PRIVATE/'stream.c',PRIVATE/'tree.c'])+providers
 def build(tag,cc,flags,sources):
  image=BUILD/tag;run(tag+'.build',[cc,*strict,*flags,*sources,'-Wl,--gc-sections','-o',image]);harness.OUTPUTS.add(image);return image
 try:
  run('kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   run(cc+'.version',[cc,'--version'])
   for mode,flags in PROFILES:
    tag=cc+'.'+mode;image=build(tag,cc,flags,[*src(),PUBLIC/'tests.c',PUBLIC/'native.c'])
    for case in ['0','1']:run(tag+'.case'+case,[image,case],marker='CT_PASS')
    oracle=build(tag+'.oracle',cc,flags,[*src(),PUBLIC/'oracle_io.c']);result=BUILD/(tag+'.oracle.json');run(tag+'.independent',[sys.executable,PRIVATE/'oracle.py',oracle,result],marker='"passed": true');harness.OUTPUTS.add(result)
    for name,case,marker in MUTANTS:
     bad=build(tag+'.mutant.'+name,cc,[*flags,'-DCT_MUTANT_'+name],[*src(),PUBLIC/'tests.c',PUBLIC/'native.c']);run(tag+'.reject.'+name,[bad,case],41,marker)
    stub=build(tag+'.stub',cc,flags,[*src(True),PUBLIC/'tests.c',PUBLIC/'native.c'])
    for case in ['0','1']:run(tag+'.stub.reject'+case,[stub,case],41,'CT_ASSERT:')
   for mode in ['O0','O3']:
    for kind,file in [('iterative',PRIVATE/'tree.c'),('recursive',providers[1])]:
     tag=cc+'.'+mode+'.cost.'+kind;obj=BUILD/(tag+'.o');run(tag+'.compile',[cc,*strict,'-'+mode,'-fstack-usage','-c',file,'-o',obj]);harness.OUTPUTS.add(obj);harness.OUTPUTS.add(obj.with_suffix('.su'));run(tag+'.size',['nm','-S','--size-sort',obj]);run(tag+'.disassembly',['objdump','-d',obj]);run(tag+'.undefined',['nm','-u',obj])
  for cc in ['arm-none-eabi-gcc','clang']:
   run(cc+'.target.version',[cc,'--version'])
   target=['-mcpu=cortex-m4','-mthumb']+(['--target=arm-none-eabi','-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib'] if cc=='clang' else [])
   for mode in ['O0','O3']:
    tag=cc+'.target.'+mode;objects=[]
    for index,file in enumerate([*src(),PUBLIC/'tests.c',PUBLIC/'target.c',ROOT/'assessor/execution/support.c']):
     obj=BUILD/(tag+'.'+str(index)+'.o');objects.append(obj);run(tag+'.object'+str(index),[cc,*strict,*target,'-'+mode,'-ffreestanding','-fno-builtin','-fstack-usage','-c',file,'-o',obj]);harness.OUTPUTS.add(obj);harness.OUTPUTS.add(obj.with_suffix('.su'));run(tag+'.undefined'+str(index),['arm-none-eabi-nm','-u',obj]);run(tag+'.assembly'+str(index),['arm-none-eabi-objdump','-d',obj])
    image=BUILD/(tag+'.elf');mapping=BUILD/(tag+'.map');run(tag+'.link',['arm-none-eabi-ld','-T',ROOT/'labs/algorithm_frontier/link.ld','--gc-sections','--no-undefined','-Map='+str(mapping),*objects,'-o',image]);harness.OUTPUTS.update([image,mapping]);assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',image]).strip();run(tag+'.size',['arm-none-eabi-size',image]);run(tag+'.assembly',['arm-none-eabi-objdump','-d',image]);run(tag+'.execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],marker='CT_TARGET_PASS')
  cost=BUILD/'costs.json';run('costs',[sys.executable,PRIVATE/'costs.py',BUILD,cost],marker='CT_COST_PASS profiles=4');harness.OUTPUTS.add(cost)
 except Exception as e:failure=str(e)
 if before!=pins() or ext!=external():failure='Inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,commands=harness.COMMANDS,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(harness.OUTPUTS)},limits=['Exact two missing original exercise variants, composed with retained providers; no whole curriculum closure.','Whole selected images use direct linker and original byte helpers, no archives. Object references in discarded provider functions are separately visible; selected closure is not full provider API closure.','Tree oracle seed360021 uses descendant closure and leaf elimination; finite coverage is not proof.','Source-bound author execution is not learner integration, delayed performance, external review or physical observation.'])
 (HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=failure)));return bool(failure)
if __name__=='__main__':raise SystemExit(main())
