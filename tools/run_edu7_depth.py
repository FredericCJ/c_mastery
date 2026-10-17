"""Audited certificate source-bound host campaigns and separate Cortex-M4 link/dependency audit."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
CASES={'edu7_depth':['grant','offer','session']}
def main():
 family='edu7_depth'
 p=argparse.ArgumentParser();p.add_argument('--report',type=Path);args=p.parse_args();build=ROOT/'build'/family/'campaign';build.mkdir(parents=True,exist_ok=True)
 paths=[x for base in [ROOT/'labs'/family,ROOT/'assessor'/family,ROOT/'fixtures'/family] for x in base.rglob('*') if x.is_file()]
 paths += [ROOT/'tools'/n for n in [f'run_{family}.py',f'verify_{family}_fixtures.py']]
 paths += list((ROOT/'research/v5/expansion/edu7_depth').glob('*'))
 paths += [ROOT/'curriculum/projects'/('PR-T40-%02d.md'%n) for n in range(1,9)]
 paths += [ROOT/p for p in ['labs/protocols/targets/runtime.c','labs/communications/include/com_edu7.h','assessor/communications/edu7.c']]
 inspected={x.relative_to(ROOT).as_posix():hashlib.sha256(x.read_bytes()).hexdigest() for x in paths}
 from v5_common import identity
 report=dict(schema_version=5,family=family,status='failed',passed=False,input_sha256=identity()['sha256'],inspected_paths=inspected,commands=[],limits=['Author execution only; independent learner assessment not performed.','Target evidence is compilation/link/dependency inspection, never board execution or physical validation.','Full written alternative implementations and real imported component integration remain required.'])
 def run(cmd,expected=0,marker=None,env=None,optional=False):
  result=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=dict(os.environ,**(env or {})),text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=300)
  index=len(report['commands']);log=build/f'{index:03}.log';log.write_text(result.stdout,encoding='utf-8')
  ok=result.returncode==expected and (marker is None or marker in result.stdout)
  report['commands'].append(dict(command=list(map(str,cmd)),returncode=result.returncode,expected=expected,passed=ok,optional=optional,environment_overrides=env or {},log=log.relative_to(ROOT).as_posix(),stdout=result.stdout))
  if not ok and not optional:raise RuntimeError(f'Command {index} failed: {cmd}: {result.stdout[-1800:]}')
  return result
 try:
  gate=json.loads((ROOT/'research/v5/expansion/edu7_depth/research_gate.json').read_text(encoding='utf-8'))
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
  marker='E7_CHECK:'
  for case in CASES[family]:run([stub/(family+'-tests'),case],1,marker)
  shared=build/'oracle.so';run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-shared','-fPIC','-O2','-I'+str(ROOT/'labs'/family/'include'),ROOT/'assessor'/family/(family+'.c'),'-o',shared])
  report['oracle']=json.loads(run([sys.executable,ROOT/'tools'/f'verify_{family}_fixtures.py','--library',shared]).stdout)
  adapter=build/'retained-adapter'
  run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/edu7_depth/include','-Ilabs/communications/include','labs/edu7_depth/tests/retained_adapter.c','assessor/edu7_depth/edu7_depth.c','assessor/communications/edu7.c','-o',adapter])
  run([adapter],marker='retained codec composition:')
  report['tsan']=dict(status='not-applicable-to-serialized-anchor',scope='Single-owner EDU7 models; no interrupt/thread interleaving claim.')
  for cc in ['gcc','clang']:run([cc,'-std=c2x','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','-I'+str(ROOT/'labs/edu7_depth/include'),ROOT/'assessor/edu7_depth/edu7_depth.c'])
  target=build/'cortex-m4';target.mkdir(exist_ok=True)
  for source,name in [(ROOT/'assessor'/family/(family+'.c'),'core'),(ROOT/'labs/protocols/targets/runtime.c','runtime')]:
   run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(ROOT/'labs'/family/'include'),'-c',source,'-o',target/(name+'.o')])
  run(['arm-none-eabi-ld',target/'core.o',target/'runtime.o','--entry=model_entry','-Map='+str(target/'audit.map'),'-o',target/'audit.elf'])
  undefined=run(['arm-none-eabi-nm','-u',target/'audit.elf']).stdout
  if undefined.strip():raise RuntimeError('Unresolved target dependencies: '+undefined)
  symbols=run(['arm-none-eabi-nm',target/'audit.elf']).stdout
  report['target_audit']=dict(status='linked-no-unresolved-symbols',target='Cortex-M4 Thumb',runtime_helpers='Explicit byte-loop __aeabi_memcpy/memclr/memset and aligned aliases; no hosted runtime or allocator.',execution='not-run',symbols=symbols)
  if any(hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=h for path,h in inspected.items()):raise RuntimeError('Inputs changed during campaign')
  report.update(status='passed',passed=True)
 except Exception as error:report['error']=str(error)
 dest=args.report or ROOT/'build'/family/'report.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
 print(json.dumps(dict(family=family,passed=report['passed'],report=str(dest),error=report.get('error'))));return 0 if report['passed'] else 1

if __name__=="__main__":raise SystemExit(main())
