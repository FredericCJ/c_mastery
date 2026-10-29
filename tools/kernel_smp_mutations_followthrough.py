"""Paired copied-provider causal controls; frozen CMK2 sources are never edited.

The legacy TLB detector required a later affinity milestone although earlier
process teardown can already perform a shootdown. Instrument the actual remote
wait and its completion in BOTH copies. A deadline without reaching that wait
remains inconclusive; it is never counted as detecting the missing ack.
"""
import argparse,json,os,shutil,subprocess,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'labs/kernel_smp/tools'))
import guest
from c_source import find_unique,replace_unique

ACK='__atomic_store_n(&shoot_ack[c],__atomic_load_n(&shoot_generation,__ATOMIC_ACQUIRE),__ATOMIC_RELEASE);'
WAIT='for(unsigned c=0;c<topology.count;c++)if(c!=me&&(mask&(1u<<c)))for(unsigned n=0;__atomic_load_n(&shoot_ack[c],__ATOMIC_ACQUIRE)!=gen;n++){if(n==100000000)fail("TLB shootdown deadline");pause_cpu();}'
INSTRUMENT='if(mask&~(1u<<me))text("KF_SMP:ACK_WAIT_REMOTE\\n");\n '+WAIT+'\n if(mask&~(1u<<me))text("KF_SMP:ACK_COMPLETED\\n");'

def main(build):
 build=build.resolve();build.mkdir(parents=True,exist_ok=False);lab=ROOT/'labs/kernel_smp';provider=ROOT/'assessor/kernel_smp';os.environ['CMASTERY_CPUS']='4';records=[]
 def one(name,stage,mutation):
  folder=build/name;source=folder/'source';shutil.copytree(provider,source)
  smp=source/'smp.c';original=smp.read_text();find_unique(original,WAIT);find_unique(original,ACK)
  # Identical instrumentation in the positive control and ack mutant.
  if name in ('tlb_control','tlb_ack'):original=replace_unique(original,WAIT,INSTRUMENT)
  if mutation=='tlb_ack':original=replace_unique(original,ACK,'/* mutation: omit remote acknowledgement */')
  smp.write_text(original)
  if mutation=='msix_mask':
   kernel=source/'kernel.c';text=kernel.read_text();anchor='entry[2]=43;barrier();entry[3]=0;';kernel.write_text(replace_unique(text,anchor,'entry[2]=43;barrier();entry[3]=1;'))
  output=folder/'guest';guest.build(lab,source,output)
  command=[sys.executable,str(lab/'tools/guest.py'),'stage','--stage',stage,'--lab',str(lab),'--source',str(source),'--build',str(output)]
  result=subprocess.run(command,capture_output=True,text=True,timeout=30)
  evidence=json.loads((output/('stage-'+stage+'.json')).read_text());log=(output/('stage-'+stage+'.log')).read_text()
  waiting='KF_SMP:ACK_WAIT_REMOTE' in log;completed='KF_SMP:ACK_COMPLETED' in log
  if mutation is None:
   passed=result.returncode==0 and evidence['passed'] and waiting and completed
  elif mutation=='tlb_ack':
   passed=result.returncode!=0 and not evidence['passed'] and waiting and not completed and (evidence.get('deadline_expired') or 'FAIL SMP: TLB shootdown deadline' in log)
  else:passed=result.returncode!=0 and not evidence['passed'] and 'FAIL' in log
  row={'mutation':mutation,'name':name,'detected':passed,'command':command,'exit':result.returncode,'wait_entered':waiting,'wait_completed':completed,'detector':'host deadline after actual remote wait' if evidence.get('deadline_expired') else 'guest stage result','log':str(output/('stage-'+stage+'.log')),'receipt':str(output/('stage-'+stage+'.json')),'kernel_sha256':json.loads((output/'build.json').read_text())['kernel_sha256']}
  print(name,passed,flush=True)
  if not passed:print(log,flush=True)
  return row
 control=one('tlb_control','tlb',None)
 records.append(one('tlb_ack','tlb','tlb_ack'));records.append(one('msix_mask','async','msix_mask'))
 report={'passed':control['detected'] and all(x['detected'] for x in records),'positive_control':control,'records':records,'scope':'Copied actual provider; paired wait instrumentation changes diagnostic timing. Missing acknowledgement is detected only after entering an actual remote wait; no host deadline before the causal precondition is accepted.'};(build/'mutations.json').write_text(json.dumps(report,indent=2)+'\n');return int(not report['passed'])

if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);raise SystemExit(main(p.parse_args().build))
