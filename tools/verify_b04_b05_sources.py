"""Reject stale bounded inspection gates; never infer reading from a URL."""
import argparse
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('batch',choices=['b04','b05','b04/systems']);a=p.parse_args()
 folder=ROOT/'research/v5/expansion'/a.batch;gate=json.loads((folder/'research_gate.json').read_text())
 register=json.loads((folder/'resources.json').read_text())
 shared=register.get('canonical_external_sources',[])
 shared_paths={row['path'] for row in shared}
 if shared_paths and (a.batch!='b05' or shared_paths!={'research/v5/expansion/b04/resources.json'}):raise ValueError('Unexpected shared source register')
 if gate['status']!='passage-inspected':raise ValueError('No passage gate')
 for name,digest in gate['research_input_sha256'].items():
  path=(ROOT/name).resolve()
  if not path.is_relative_to(ROOT/'research/v5/expansion'/a.batch) and name not in shared_paths:raise ValueError('Escaping gate input')
  if hashlib.sha256(path.read_bytes()).hexdigest()!=digest:raise ValueError('Stale gate: '+name)
 records=list(register['resources'])
 for row in shared:
  if row['path'] not in gate['research_input_sha256']:raise ValueError('Shared source register is not pinned')
  matches=[r for r in json.loads((ROOT/row['path']).read_text())['resources'] if r['id']==row['id']]
  if len(matches)!=1:raise ValueError('Shared source ID is not unique')
  records.extend(matches)
 for record in records:
  inspection=record['inspection']
  if not all(inspection.get(k) for k in ['date','method','scope','limits','document_sha256']):raise ValueError('Missing bounded inspection evidence')
  if record.get('path') and hashlib.sha256((ROOT/record['path']).read_bytes()).hexdigest()!=inspection['document_sha256']:raise ValueError('Original source changed after inspection')
 print(json.dumps({'batch':a.batch,'passed':True,'scope':gate['scope'],'limits':'Hash consistency validates the stated record, not independent review or complete corpus inspection.'}))
if __name__=='__main__':main()
