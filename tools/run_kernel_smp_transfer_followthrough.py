"""T48.14 delayed transfer to the separate, frozen T56 process/thread provider."""
import argparse,hashlib,json,os,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);commands=[];runs=[]
 lab=ROOT/'labs/kernel_smp';source=ROOT/'assessor/kernel_smp';tool=lab/'tools/guest.py';env=dict(os.environ)
 env.update({key:str(ROOT/path) for key,path in {'CMASTERY_RUNTIME_ROOT':'assessor/runtime','CMASTERY_RUNTIME_INCLUDE':'labs/runtime/include','CMASTERY_RUNTIME_ADAPTERS':'labs/runtime/adapters','CMASTERY_STORAGE_ROOT':'assessor/storage','CMASTERY_STORAGE_INCLUDE':'labs/storage/include'}.items()})
 def run(label,cmd,timeout=180):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,env=env,timeout=timeout);p=work/(label+'.txt');p.write_text(r.stdout+r.stderr);commands.append({'label':label,'command':cmd,'exit_code':r.returncode,'expected_exit_code':0,'output':p.relative_to(ROOT).as_posix()});assert r.returncode==0,(label,r.stdout[-6000:],r.stderr[-2000:])
 try:
  built=work/'built';run('build',[sys.executable,tool,'build','--lab',lab,'--source',source,'--build',built]);build=json.loads((built/'build.json').read_text());assert build['runtime_linked'] and build['storage_linked']
  for cpus in (1,2,4):
   folder=work/('cpus'+str(cpus));folder.mkdir()
   for p in built.iterdir():
    if p.is_file() and p.suffix in ('.elf','.iso','.json'):shutil.copy2(p,folder/p.name)
   env['CMASTERY_CPUS']=str(cpus);run('cpus'+str(cpus),[sys.executable,tool,'test','--require','storage','--lab',lab,'--source',source,'--build',folder]);record=json.loads((folder/'execution.json').read_text());assert record['passed'];runs.append({'cpus':cpus,'receipt':(folder/'execution.json').relative_to(ROOT).as_posix(),'sha256':sha(folder/'execution.json')})
  # Missing acknowledgement and masked MSI-X are mechanism-specific controls.
  run('mutations',[sys.executable,ROOT/'tools/kernel_smp_mutations_followthrough.py','--build',work/'mutations'],240);mutation=json.loads((work/'mutations/mutations.json').read_text());assert mutation['passed'] and len(mutation['records'])==2
  inspected={Path(p).relative_to(ROOT).as_posix():h for p,h in build['source_sha256'].items()}
  for path in (Path(__file__),ROOT/'tools/kernel_smp_mutations_followthrough.py'):inspected[path.relative_to(ROOT).as_posix()]=sha(path)
  assert all(sha(ROOT/p)==h for p,h in inspected.items())
 except Exception as e:passed=False;error=repr(e);inspected=locals().get('inspected',{})
 else:passed=True;error=None
 result={'passed':passed,'error':error,'commands':commands,'runs':runs,'inspected_paths':inspected,'producer_sha256':sha(Path(__file__)),'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Separate frozen CMK2 provider, not an SMP upgrade of the T48 successor. Actual 1/2/4-vCPU TCG and explicit baseline CVM/store linkage; author transfer support only, no independent delayed learner or physical claim.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'runs':len(runs)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);raise SystemExit(main(p.parse_args().work))
