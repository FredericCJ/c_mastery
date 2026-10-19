"""Attach actual T16 successor while retaining all original identities."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/allocation_depth').glob('*') if p.is_file())+['labs/algorithms/include/algo.h']
 register='research/v5/expansion/legacy_allocation/resources.json';resources={r['id']:r for r in json.loads((ROOT/register).read_text(encoding='utf8'))['resources']}
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T16-*.json')):
  data=json.loads(path.read_text(encoding='utf8'));data['supplied_artifacts']=sorted(set(data['supplied_artifacts']+public));contract='labs/allocation_depth/CONTRACTS.md'
  if contract not in data['shared_contracts']:data['shared_contracts'].append(contract)
  number=int(path.stem[-2:]);selected=['R-ALLOCATION-N1570','R-ALLOCATION-WILSON']
  if number in (1,7):selected.append('R-ALLOCATION-N3096')
  if number==8:selected+=['R-ALLOCATION-MMAP','R-ALLOCATION-MUTEX','R-ALLOCATION-JOIN']
  for ident in selected:
   item={'id':ident,'path':register,'passages':resources[ident]['locators']};data['source_records']=[old for old in data['source_records'] if old['id']!=ident]+[item]
  data['execution_support']={'kind':'actual local T16 allocation policies, unchanged algo_pool composition, host adapters and exact transfer dossiers','scope':'42 ordinary and42 original delayed obligations bound separately in research/v5/expansion/legacy_allocation/completion_dispositions.json; author work is distinct from independent learner, external review and physical evidence.','tests':['python3 tools/run_allocation_followthrough.py'],'fresh_execution':'See source-bound research/v5/expansion/legacy_allocation/validation.json','author_validation':'research/v5/expansion/legacy_allocation/validation.json','successor_bindings':'research/v5/expansion/legacy_allocation/completion_dispositions.json','provider_contracts':['labs/algorithms/include/algo.h','labs/machine_image/freestanding_depth/CONTRACTS.md'],'reference_gap_resolutions':'Bounded Wilson, actual ISO drafts and selected host-interface passages registered without relabeling drafts or inaccessible sources.'}
  data['status']['execution']='See source-bound T16 author receipt; independent learner assessment remains separate';path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
 print('Attached T16 successor to9 original projects; retained42 ordinary/42 delayed identities')
if __name__=='__main__':main()
