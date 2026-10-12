"""Linux verification of the 32 v4 units and their explicit negative controls.

Run in WSL from the repository root. Assessor dependencies are explicitly
supplied even while testing integration stubs, isolating the unsolved unit.
"""
import argparse,datetime,hashlib,json,pathlib,subprocess,sys
p=argparse.ArgumentParser();p.add_argument('--compiler',default='gcc');p.add_argument('--profile',choices=['debug','release','asan'],default='debug');p.add_argument('--family',action='append');p.add_argument('--learner-only',action='store_true');p.add_argument('--report',action='store_true');p.add_argument('--receipts-dir',default='reviews/v4');a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
families=a.family or ['algorithms','communications','foundations','firmware_io','integration']
report={'schema_version':1,'kind':'v4-executable-unit-verification','compiler':a.compiler,'profile':a.profile,'generated_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'32 new unit gates, explicit mutations, two P1 architectures and Cortex-M4/M7 compilation. No physical hardware evidence.','runs':[]}
for family in families:
 for assessor in ([False] if a.learner_only else [True,False]):
  tag=f'v4-{family}-{a.compiler}-{a.profile}-'+('assessor' if assessor else 'learner')
  build=root/'build'/tag;build.mkdir(parents=True,exist_ok=True)
  args=['cmake','-S',str(root/'labs'/family),'-B',str(build),'-DCMAKE_C_COMPILER='+a.compiler,'-DCMAKE_BUILD_TYPE='+('Release' if a.profile=='release' else 'Debug')]
  if assessor:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(root/'assessor'/family)]
  if a.profile=='asan':args+=['-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer','-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined']
  if family=='integration':
   for dep in ['foundations','algorithms','firmware_io','runtime','storage']:
    args+=['-DCMASTERY_'+dep.upper()+'_ROOT='+str(root/'assessor'/dep),'-DCMASTERY_'+dep.upper()+'_INCLUDE='+str(root/'labs'/dep/'include')]
  log=[]
  for cmd in [args,['cmake','--build',str(build),'-j','6']]:
   r=subprocess.run(cmd,capture_output=True,text=True);log+=[r.stdout,r.stderr]
   if r.returncode:(build/'verification.log').write_text('\n'.join(log));print('\n'.join(log)[-14000:]);raise SystemExit(tag+' build failed')
  data=json.loads((root/'labs'/family/'family.json').read_text())
  names={t for u in data['units'] for t in u['tests']}
  inventory=json.loads(subprocess.check_output(['ctest','--test-dir',str(build),'--show-only=json-v1'],text=True))['tests']
  if assessor:names|=set(data.get('negative_controls',[]))&{t['name'] for t in inventory if '.negative.' in t['name']}
  if assessor and family=='firmware_io':names.add('firmware_io.cortex_compile')
  if assessor and family=='integration':names.add('integration.architecture_lookahead')
  selected=[t for t in inventory if t['name'] in names]
  missing=names-{t['name'] for t in selected}
  if missing:raise SystemExit('Missing gates: '+str(missing))
  outcomes=[]
  for test in selected:
   result=subprocess.run(test['command'],cwd=build,capture_output=True,text=True,timeout=60);expected=0 if assessor else 1
   log.append(test['name']+' expected='+str(expected)+' actual='+str(result.returncode)+'\n'+result.stdout+result.stderr)
   if result.returncode!=expected:
    (build/'verification.log').write_text('\n'.join(log));print(log[-1]);raise SystemExit(tag+' semantic verification failed')
   outcomes.append({'test':test['name'],'expected_returncode':expected,'actual_returncode':result.returncode,'status':'passed' if assessor else 'learner-scaffold-rejected'})
  (build/'verification.log').write_text('\n'.join(log))
  print(tag+': '+str(len(selected))+' expected results',flush=True)
  report['runs'].append({'family':family,'mode':'assessor' if assessor else 'learner-with-explicit-reference-dependencies' if family=='integration' else 'learner','log':str((build/'verification.log').relative_to(root)),'configure_command':args,'outcomes':outcomes})
if a.report:
 paths=set()
 for family in families:
  for directory in [root/'labs'/family,root/'assessor'/family]:
   paths.update(p for p in directory.rglob('*') if p.is_file() and not any(part in {'build','__pycache__'} for part in p.relative_to(root).parts))
 if 'integration' in families:
  for dep in ['runtime','storage']:
   paths.update((root/'assessor'/dep).glob('*.c'));paths.update((root/'labs'/dep/'include').glob('*.h'))
 paths.update([pathlib.Path(__file__),root/'tools/v4_lab_assets.py'])
 report['inspected_paths']=[{'path':p.relative_to(root).as_posix(),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(paths)]
 destination=root/a.receipts_dir/f'labs32-{a.compiler}-{a.profile}.json';destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(json.dumps(report,indent=2)+'\n')
 print('Evidence report: '+str(destination.relative_to(root)))
