"""Apply only explicitly inspected source-access successor dispositions."""
from common import ROOT,read_json,dump,sha
SOURCE='research/v5/expansion/followthrough/source_access_successor.json'
def main():
 record=read_json(ROOT/SOURCE);state=read_json(ROOT/'curriculum/v5/expansion/implementation_updates.json');index={r['id']:r for r in state['updates']}
 for source in record['sources']:
  from pathlib import Path
  p=Path(source['private_source']);expected=source.get('sha256',source.get('retrieval',{}).get('sha256'))
  if not p.is_file() or sha(p)!=expected:raise ValueError('Source identity changed: '+source['id'])
  index[source['id']]=dict(id=source['id'],original_source=source['original_source'],closure_rationale=source['technical_consequence'],dimensions={'teaching':dict(state=source['current_status'],evidence=[SOURCE,source['closure_evidence']]),'implementation':dict(state='not-applicable-to-source-access-only-obligation',evidence=[SOURCE]),'author_validation':dict(state='actual-named-passage-inspection-and-byte-identity',evidence=[SOURCE]),'external_review':dict(state='not-inferred',evidence=[]),'independent_learner':dict(state='not-inferred',evidence=[]),'physical_observation':dict(state='not-applicable-to-source-access',evidence=[])},required_environment='Authorized exact named source access; no private source redistribution.',evidence_limits=[source['limitations']])
 state['updates']=list(index.values());dump(ROOT/'curriculum/v5/expansion/implementation_updates.json',state)
 ledgerpath=ROOT/'curriculum/v5/expansion/completion_ledger.json';ledger=read_json(ledgerpath)
 if SOURCE not in ledger['required_gap_registers']:ledger['required_gap_registers'].append(SOURCE)
 dump(ledgerpath,ledger);print(dict(source_obligations=len(record['sources']),total=len(index),historical_records_rewritten=False))
if __name__=='__main__':main()
