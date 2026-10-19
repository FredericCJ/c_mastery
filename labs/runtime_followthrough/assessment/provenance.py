"""Replay exact trusted compiler/VM binaries; hashes alone are not derivation."""
from pathlib import Path
import argparse,hashlib,json,subprocess,tempfile

def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def call(cmd):
 r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30)
 return dict(command=list(map(str,cmd)),returncode=r.returncode,stdout=r.stdout,stderr=r.stderr)
def validate(d):
 errors=[]
 try:
  for p,h in d['inputs'].items():
   if sha(p)!=h:errors.append('stale input '+p)
  if errors:return errors
  required={d[k] for k in ['source','image','compiler','vm','build_record']}
  if not required<=set(d['inputs']):return ['missing chain node']
  build=json.loads(Path(d['build_record']).read_text())
  if not build['passed'] or not build['commands'] or any(r['returncode'] for r in build['commands']):return ['failed build']
  for p,h in build['inputs'].items():
   if sha(p)!=h:errors.append('stale build input '+p)
  if errors:return errors
  if not all(d[k] in build['outputs'] and sha(d[k])==build['outputs'][d[k]] for k in ['compiler','vm']):return ['build output binding']
  if d['compile']['command']!=[d['compiler'],d['source'],d['image']] or d['compile']['returncode'] or d['compile']['stderr']:return ['compiler command binding']
  if d['run']['command']!=[d['vm'],d['image'],str(d['adapter'])] or d['run']['returncode'] or d['run']['stderr']:return ['VM command binding']
  if json.loads(d['run']['stdout'])!=d['result']:return ['result transcript mismatch']
  with tempfile.TemporaryDirectory(prefix='cmastery-chain-') as temp:
   image=Path(temp)/'recompiled.cvm';compiled=call([d['compiler'],d['source'],image])
   if compiled['returncode'] or compiled['stdout']!=d['compile']['stdout'] or compiled['stderr']:return ['recompilation verdict']
   if image.read_bytes()!=Path(d['image']).read_bytes():return ['source/image derivation mismatch']
   replay=call([d['vm'],image,str(d['adapter'])])
   if replay['returncode'] or replay['stderr'] or json.loads(replay['stdout'])!=d['result']:return ['VM replay mismatch']
 except (KeyError,ValueError,OSError,subprocess.SubprocessError) as e:errors.append(type(e).__name__+': '+str(e))
 return errors

def record(source,compiler,vm,build,out,adapter):
 source,compiler,vm,build,out=[p.resolve() for p in [source,compiler,vm,build,out]]
 out.mkdir(parents=True,exist_ok=False);saved=out/'source.cvm';saved.write_bytes(source.read_bytes());image=out/'image.cvm'
 compile_row=call([compiler,saved,image]);assert compile_row['returncode']==0,compile_row
 run=call([vm,image,str(adapter)]);assert run['returncode']==0,run
 d=dict(schema_version=1,source=str(saved.resolve()),image=str(image.resolve()),compiler=str(compiler.resolve()),vm=str(vm.resolve()),build_record=str(build.resolve()),adapter=adapter,compile=compile_row,run=run,result=json.loads(run['stdout']),limits=['Author replay of trusted binaries and captured build inputs; no independent reader claim.','Hashes identify artifacts; compiler re-execution checks source/image derivation; neither proves compiler correctness or hardware behavior.'])
 d['inputs']={str(p.resolve()):sha(p) for p in [saved,image,compiler,vm,build]};assert not validate(d);(out/'chain.json').write_text(json.dumps(d,indent=2)+'\n');return d
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--record',type=Path);p.add_argument('--source',type=Path);p.add_argument('--compiler',type=Path);p.add_argument('--vm',type=Path);p.add_argument('--build-record',type=Path);p.add_argument('--output',type=Path);p.add_argument('--adapter',type=int,choices=[1,2],default=1);a=p.parse_args()
 if a.record:
  errors=validate(json.loads(a.record.read_text()));print(json.dumps(dict(passed=not errors,errors=errors)));raise SystemExit(bool(errors))
 if not all([a.source,a.compiler,a.vm,a.build_record,a.output]):p.error('record creation requires source,compiler,vm,build-record,output')
 d=record(a.source.resolve(),a.compiler.resolve(),a.vm.resolve(),a.build_record.resolve(),a.output.resolve(),a.adapter);print(json.dumps(dict(passed=True,result=d['result'],image_sha256=sha(d['image']))))
