"""Attach current T17 mechanisms without rewriting historical baseline records."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/numerical_depth').glob('*') if p.is_file())+['labs/checkpoint_depth/include/checkpoint_numeric.h']
 register='research/v5/expansion/legacy_numerics/resources.json';resources={r['id']:r for r in json.loads((ROOT/register).read_text(encoding='utf8'))['resources']}
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T17-*.json')):
  data=json.loads(path.read_text(encoding='utf8'));data['supplied_artifacts']=sorted(set(data['supplied_artifacts']+public));contract='labs/numerical_depth/CONTRACTS.md'
  if contract not in data['shared_contracts']:data['shared_contracts'].append(contract)
  for ident in resources:
   item={'id':ident,'path':register,'passages':resources[ident]['locators']};data['source_records']=[old for old in data['source_records'] if old['id']!=ident]+[item]
  data['execution_support']={'kind':'actual numerical mechanisms, frozen FIR/IIR producer composition and explicitly named exact transfer derivations','scope':'30 ordinary and30 original delayed obligations remain separately mapped; author validation is distinct from learner, independent review and physical evidence.','tests':['python3 tools/run_numerical_followthrough.py'],'fresh_execution':'See source-bound research/v5/expansion/legacy_numerics/validation.json','author_validation':'research/v5/expansion/legacy_numerics/validation.json','successor_bindings':'research/v5/expansion/legacy_numerics/completion_dispositions.json','provider_contracts':['labs/checkpoint_depth/include/checkpoint_numeric.h'],'reference_gap_resolutions':'Exact held Higham edition selected passages actually inspected; bounded Goldberg and official versioned compiler passages read; published ISO/IEEE text not invented.'}
  data['status']['execution']='See source-bound T17 author receipt; independent learner remains separate';path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
 print('Attached T17 successor to7 original projects; retained30 ordinary/30 delayed identities')
if __name__=='__main__':main()
