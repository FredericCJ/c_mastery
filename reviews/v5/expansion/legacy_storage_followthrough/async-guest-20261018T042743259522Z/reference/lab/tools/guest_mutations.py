"""Assessor-only mutation controls: compile copies; require actual guest failure."""
import argparse,json,os,shutil,subprocess,sys
from pathlib import Path
import guest

def main():
 p=argparse.ArgumentParser();p.add_argument('--lab',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--build',type=Path,required=True);a=p.parse_args()
 a.build.mkdir(parents=True,exist_ok=True);os.environ['CMASTERY_CPUS']='4';records=[]
 specs=[('tlb_ack','smp.c','__atomic_store_n(&shoot_ack[c],__atomic_load_n(&shoot_generation,__ATOMIC_ACQUIRE),__ATOMIC_RELEASE);','/* mutation: omit remote acknowledgement */','tlb'),('msix_mask','kernel.c','entry[2]=43;barrier();entry[3]=0;','entry[2]=43;barrier();entry[3]=1;','async')]
 for name,file,before,after,stage in specs:
  root=a.build/name;source=root/'source';shutil.copytree(a.source,source,dirs_exist_ok=True)
  path=source/file;text=path.read_text();assert text.count(before)==1;path.write_text(text.replace(before,after))
  output=root/'guest';guest.build(a.lab.resolve(),source.resolve(),output.resolve())
  cmd=[sys.executable,str(a.lab/'tools/guest.py'),'stage','--stage',stage,'--lab',str(a.lab),'--source',str(source),'--build',str(output)]
  result=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
  evidence=json.loads((output/('stage-'+stage+'.json')).read_text());log=(output/('stage-'+stage+'.log')).read_text()
  # Missing marker alone is not enough: require guest failure or the configured
  # host deadline after the preceding real guest milestone, not external interruption.
  killed=result.returncode!=0 and not evidence['passed'] and ('FAIL' in log or (name=='tlb_ack' and evidence.get('deadline_expired') and 'PASS smp-affinity-migration' in log))
  records.append({'mutation':name,'detected':killed,'command':cmd,'exit':result.returncode,'detector':'host deadline' if evidence.get('deadline_expired') else 'guest failure','log':str(output/('stage-'+stage+'.log')),'kernel_sha256':json.loads((output/'build.json').read_text())['kernel_sha256']})
  print(name,'detected',killed,flush=True)
  if not killed:print(result.stdout,result.stderr,log)
 report={'passed':all(x['detected'] for x in records),'records':records};(a.build/'mutations.json').write_text(json.dumps(report,indent=2)+'\n')
 if not report['passed']:raise SystemExit(1)
if __name__=='__main__':main()
