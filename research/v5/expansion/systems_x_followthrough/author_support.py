"""Pure additive metadata hook; original contracts/rubrics/variants stay untouched."""
import copy
IDS={f'X-T{i}-001' for i in range(43,56)}|{'X-T51-002','X-T55-002','X-T55-003'}
def attach(record):
 if record.get('id') not in IDS:return record
 result=copy.deepcopy(record);result['systems_x_followthrough_author_support']={'version':1,'support':'research/v5/expansion/systems_x_followthrough/support.json','mapping':'research/v5/expansion/systems_x_followthrough/evidence_map.json','scaffold':'labs/systems_x_followthrough','contract':'labs/systems_x_followthrough/README.md','assessor':'assessor/systems_x_followthrough/ANALYSIS.md','scope':'Exact original criteria and changed conditions, separate author/native/model/emulator scopes.','external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-run'};return result
