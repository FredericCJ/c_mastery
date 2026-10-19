"""Audited certificate source-bound host campaigns and separate Cortex-M4 link/dependency audit."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES={'service_depth':['memory','channel','files','journal']}
def main():
 family='service_depth'
 p=argparse.ArgumentParser();p.add_argument('--report',type=Path);args=p.parse_args();build=ROOT/'build'/family/'campaign';build.mkdir(parents=True,exist_ok=True)
 paths=[x for base in [ROOT/'labs'/family,ROOT/'assessor'/family,ROOT/'fixtures'/family] for x in base.rglob('*') if x.is_file() and '__pycache__' not in x.parts]
 paths += [ROOT/'tools'/n for n in [f'run_{family}.py',f'verify_{family}_fixtures.py']]
 paths += list((ROOT/'research/v5/expansion/service_depth').glob('*'))
 paths += [ROOT/'curriculum/projects'/('PR-T56-%02d.md'%n) for n in [2,4,5,8,10,11]]
 paths += [ROOT/p for p in ['labs/protocols/targets/runtime.c','research/v5/expansion/legacy_amp_services/resources.json']]
 paths += [ROOT/'curriculum/v5/exercises'/('X-T56-%03d.md'%n) for n in range(2,6)]
 inspected={x.relative_to(ROOT).as_posix():hashlib.sha256(x.read_bytes()).hexdigest() for x in paths}
 from v5_common import identity
 report=dict(schema_version=5,family=family,status='failed',passed=False,input_sha256=identity()['sha256'],inspected_paths=inspected,commands=[],limits=['Author execution only; independent learner assessment not performed.','Portable Cortex-M evidence is link/dependency inspection; the separately bound adapter campaign executes QEMU guests, never physical boards.','Portable core/composition logs do not establish real adapters; the mandatory separately source-bound adapter campaign supplies that bounded author evidence.'])
 def run(cmd,expected=0,marker=None,env=None,optional=False):
  result=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=dict(os.environ,**(env or {})),text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=1800 if any('run_service_adapters.py' in str(x) for x in cmd) else 300)
  index=len(report['commands']);log=build/f'{index:03}.log';log.write_text(result.stdout,encoding='utf-8')
  ok=result.returncode==expected and (marker is None or marker in result.stdout)
  report['commands'].append(dict(command=list(map(str,cmd)),returncode=result.returncode,expected=expected,passed=ok,optional=optional,environment_overrides=env or {},log=log.relative_to(ROOT).as_posix(),stdout=result.stdout))
  if not ok and not optional:raise RuntimeError(f'Command {index} failed: {cmd}: {result.stdout[-1800:]}')
  return result
 try:
  gate=json.loads((ROOT/'research/v5/expansion/service_depth/research_gate.json').read_text(encoding='utf-8'))
  assert gate['status']=='passage-inspected'
  for path,expected_hash in gate['research_input_sha256'].items():assert hashlib.sha256((ROOT/path).read_bytes()).hexdigest()==expected_hash,path
  for cc in ['gcc','clang']:
   run([cc,'--version'])
   for mode in ['Debug','Release','Sanitize']:
    folder=build/(cc+'-'+mode.lower())
    run(['cmake','-S',ROOT/'labs'/family,'-B',folder,'-DCMAKE_C_COMPILER='+cc,'-DCMAKE_BUILD_TYPE='+('Debug' if mode=='Sanitize' else mode),'-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'/family),'-DCMASTERY_SANITIZE='+('ON' if mode=='Sanitize' else 'OFF')])
    run(['cmake','--build',folder,'-j4'])
    run(['ctest','--test-dir',folder,'--output-on-failure'],env={'ASAN_OPTIONS':'detect_stack_use_after_return=1'})
  stub=build/'stub';run(['cmake','-S',ROOT/'labs'/family,'-B',stub,'-DCMASTERY_ASSESSOR_ROOT=']);run(['cmake','--build',stub,'-j4'])
  marker='SD_CHECK:'
  for case in CASES[family]:run([stub/(family+'-tests'),case],1,marker)
  shared=build/'oracle.so';run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-shared','-fPIC','-O2','-I'+str(ROOT/'labs'/family/'include'),ROOT/'assessor'/family/(family+'.c'),'-o',shared])
  report['oracle']=json.loads(run([sys.executable,ROOT/'tools'/f'verify_{family}_fixtures.py','--library',shared]).stdout)
  composition=build/'composition'
  run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/service_depth/include','labs/service_depth/tests/composition.c','assessor/service_depth/service_depth.c','-o',composition])
  run([composition],marker='service composition:')
  report['tsan']=dict(status='not-applicable-to-serialized-anchor',scope='Serialized service cores; real kernel synchronization is a separate adapter obligation.')
  for cc in ['gcc','clang']:run([cc,'-std=c2x','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','-I'+str(ROOT/'labs/service_depth/include'),ROOT/'assessor/service_depth/service_depth.c'])
  target=build/'cortex-m4';target.mkdir(exist_ok=True)
  for source,name in [(ROOT/'assessor'/family/(family+'.c'),'core'),(ROOT/'labs/protocols/targets/runtime.c','runtime')]:
   run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(ROOT/'labs'/family/'include'),'-c',source,'-o',target/(name+'.o')])
  run(['arm-none-eabi-ld',target/'core.o',target/'runtime.o','--entry=model_entry','-Map='+str(target/'audit.map'),'-o',target/'audit.elf'])
  undefined=run(['arm-none-eabi-nm','-u',target/'audit.elf']).stdout
  if undefined.strip():raise RuntimeError('Unresolved target dependencies: '+undefined)
  symbols=run(['arm-none-eabi-nm',target/'audit.elf']).stdout
  report['target_audit']=dict(status='linked-no-unresolved-symbols',target='Cortex-M4 Thumb',runtime_helpers='Explicit byte-loop __aeabi_memcpy/memclr/memset and aligned aliases; no hosted runtime or allocator.',execution='not-run',symbols=symbols)
  adapter_receipt=ROOT/'reviews/v5/expansion/followthrough/service_adapter_campaign.json'
  run([sys.executable,ROOT/'tools/run_service_adapters.py','--report',adapter_receipt])
  run([sys.executable,ROOT/'tools/check_service_adapters_v5.py','--report',adapter_receipt.relative_to(ROOT).as_posix()])
  report['adapter_campaign']={'path':adapter_receipt.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(adapter_receipt.read_bytes()).hexdigest()}
  if any(hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=h for path,h in inspected.items()):raise RuntimeError('Inputs changed during campaign')
  report.update(status='passed',passed=True)
 except Exception as error:report['error']=str(error)
 dest=args.report or ROOT/'build'/family/'report.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(dict(family=family,passed=report['passed'],report=str(dest),error=report.get('error'))));return 0 if report['passed'] else 1

if __name__=="__main__":raise SystemExit(main())
