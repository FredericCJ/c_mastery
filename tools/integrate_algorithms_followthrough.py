"""Attach actual T15 successor inputs and inspected sources without baseline edits."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/algorithm_depth').glob('*') if p.is_file())
 public+=['labs/containers_depth/containers.h','labs/machine_image/freestanding_depth/free.h']
 register='research/v5/expansion/legacy_algorithms/resources.json';resources={r['id']:r for r in json.loads((ROOT/register).read_text(encoding='utf8'))['resources']}
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T15-*.json')):
  data=json.loads(path.read_text(encoding='utf8'));data['supplied_artifacts']=sorted(set(data['supplied_artifacts']+public));contract='labs/algorithm_depth/CONTRACTS.md'
  if contract not in data['shared_contracts']:data['shared_contracts'].append(contract)
  number=int(path.stem[-2:]);selected=['R-ALGORITHMS-CLRS'] if number<=6 else ['R-ALGORITHMS-N1570'] if number in (7,8) else ['R-ALGORITHMS-SCANNING','R-ALGORITHMS-PARSING'] if number==9 else []
  for ident in selected:
   item={'id':ident,'path':register,'passages':resources[ident]['locators']}
   data['source_records']=[old for old in data['source_records'] if old['id']!=ident]+[item]
  data['execution_support']={'kind':'additive actual T15 algorithm mechanisms, T14/T11 provider composition and explicit transfer models','scope':'Exact ordinary and original delayed obligations in research/v5/expansion/legacy_algorithms/completion_dispositions.json. Independent learner, published-ISO access and physical measurements remain separately qualified.','tests':['python3 tools/run_algorithms_followthrough.py'],'fresh_execution':'See current source-bound research/v5/expansion/legacy_algorithms/validation.json','author_validation':'research/v5/expansion/legacy_algorithms/validation.json','successor_bindings':'research/v5/expansion/legacy_algorithms/completion_dispositions.json','provider_contracts':['labs/containers_depth/CONTRACTS.md','labs/machine_image/freestanding_depth/CONTRACTS.md'],'reference_gap_resolutions':'Bounded CLRS and original-author parser source passages now inspected under research/v5/expansion/legacy_algorithms/resources.json; historical source notes are preserved.'}
  data['status']['execution']='See source-bound T15 author execution receipt; independent learner assessment not supplied'
  path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
 print('Attached T15 successor to ten original project supplements; retained45 ordinary and45 delayed identities')
if __name__=='__main__':main()
