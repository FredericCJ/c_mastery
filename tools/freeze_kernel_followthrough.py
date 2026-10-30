"""Compose already executed source-bound T48 mechanisms without regenerating them."""
import argparse,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OPEN=['Independent external review not run.','Minimum-seven-day independent learner assessment not observed.','Physical CPU,DMA,timing and power-loss observations not run.','Exact published C17 wording and full current hardware errata not newly inspected.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(matrix):
 from check_kernel_followthrough import validate_campaign
 paths={'contracts':'reviews/v5/expansion/legacy_kernel_followthrough/contracts_attempt4/result.json','device':'reviews/v5/expansion/legacy_kernel_followthrough/device_attempt4/result.json','matrix':matrix,'runtime':'reviews/v5/expansion/legacy_kernel_followthrough/runtime_frozen.json','smp':'reviews/v5/expansion/legacy_kernel_followthrough/smp_transfer_attempt4/result.json','services':'reviews/v5/expansion/legacy_vm_services/current_validation.json'}
 pins={};components={}
 for name,path in paths.items():
  p=ROOT/path;data=json.loads(p.read_text());assert data['passed'];components[name]={'path':path,'sha256':sha(p)}
  for source,h in data.get('inspected_paths',data.get('input_sha256',{})).items():
   assert source not in pins or pins[source]==h,('conflicting source',source);pins[source]=h
 for path in ('tools/freeze_kernel_followthrough.py','tools/check_kernel_followthrough.py','research/v5/expansion/legacy_kernel_followthrough/resources.json','research/v5/expansion/legacy_kernel_followthrough/source_access.json','research/v5/expansion/legacy_kernel_followthrough/research_gate.json'):
  pins[path]=sha(ROOT/path)
 v={'passed':True,'error':None,'components':components,'inspected_paths':pins,'coverage':{'original_criteria':39,'original_delayed_variants':39,'guest_profiles':87,'runtime_profiles':12,'smp_cpu_counts':[1,2,4],'host_contract_checks':102,'host_device_checks':79},'allowed_guest_exit_codes':[33,35],'open_gates':OPEN,'scope':'Exact T48.02–14 author support. T48.01 frozen separately. Models,actual one-CPU guests,separate CMK2 transfer,source authority and external observations retain their own scopes.'};validate_campaign(v);dest=ROOT/'reviews/v5/expansion/legacy_kernel_followthrough/campaign.json';dest.write_text(json.dumps(v,indent=2)+'\n');print(json.dumps({'passed':True,'source_pins':len(pins),'campaign_sha256':sha(dest)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--matrix',required=True);main(p.parse_args().matrix)
