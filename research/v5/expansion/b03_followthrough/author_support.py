"""Pure idempotent additive hook for the five original exercise dictionaries."""
import copy
IDS={f'X-T{n}-002' for n in (23,24,25,28,30)}
def attach(record):
 if record.get('id') not in IDS:return record
 result=copy.deepcopy(record)
 result['b03_followthrough_author_support']={
  'version':1,'support':'research/v5/expansion/b03_followthrough/support.json',
  'mapping':'research/v5/expansion/b03_followthrough/evidence_map.json',
  'scaffold':'labs/b03_followthrough','contract':'labs/b03_followthrough/CONTRACTS.md',
  'assessor':'assessor/b03_followthrough/ANALYSIS.md',
  'scope':'Exact original criteria plus ordinary and delayed changed conditions; author execution only.',
  'independent_learner':'not-assessed','external_review':'not-run','physical_observation':'not-claimed'}
 return result
