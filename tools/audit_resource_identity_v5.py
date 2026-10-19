"""Read-only preflight of existing merged registry against canonical additions."""
import json
from pathlib import Path
from resource_inputs_v5 import discover
from integrate_v5 import merge_resource_record
from v5_common import baseline
ROOT=Path(__file__).resolve().parents[1]
def main():
 incoming,discovery=discover(ROOT);prior={r['id']:r for r in json.loads((ROOT/'references/resources.json').read_text())['resources']};accepted=baseline()['resources'];conflicts=[]
 for r in incoming:
  try:
   if r.get('reuse'):
    if r['id'] not in prior:raise ValueError('Unknown reused resource')
   else:merge_resource_record(prior.get(r['id'],{}),r,r['id'] in accepted)
  except ValueError as e:conflicts.append(dict(id=r['id'],accepted=r['id'] in accepted,error=str(e),previous=prior.get(r['id']),incoming=r,provenance=discovery['registration_provenance'][r['id']]))
 p=ROOT/'research/v5/expansion/resource_merge_repair/registry_identity_conflicts.json';p.write_text(json.dumps(conflicts,indent=2)+'\n',encoding='utf8')
 print(json.dumps([dict(id=r['id'],accepted=r['accepted'],differing_identity={k:[r['previous'].get(k),r['incoming'].get(k)] for k in ['title','authors','edition','year','url'] if r['previous'].get(k)!=r['incoming'].get(k)}) for r in conflicts],indent=2))
if __name__=='__main__':main()
