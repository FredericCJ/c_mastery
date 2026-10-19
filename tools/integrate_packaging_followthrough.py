"""Add T22 implementation support without editing accepted project/unit evidence."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/api_depth/library').rglob('*') if p.is_file() and '__pycache__' not in p.parts]+['labs/api_depth/include/api_depth.h']
 register='research/v5/expansion/legacy_packaging/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources']
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T22-*.json')):
  d=json.loads(path.read_text(encoding='utf8'));d['supplied_artifacts']=sorted(set(d['supplied_artifacts']+public));contract='labs/api_depth/library/CONTRACTS.md'
  if contract not in d['shared_contracts']:d['shared_contracts'].append(contract)
  for r in resources:d['source_records']=[x for x in d['source_records'] if x['id']!=r['id']]+[dict(id=r['id'],path=register,passages=r['locators'])]
  d['execution_support']=dict(kind='Actual library adapter, relocatable packages, foreign consumers, docs, semantic mutants and original local bisection/release experiments',scope='All30 original criteria and30 exact delayed author tasks; independent learner/review and physical evidence remain distinct.',tests=['python3 tools/run_packaging_followthrough.py'],fresh_execution='See current source-bound research/v5/expansion/legacy_packaging/validation.json',author_validation='research/v5/expansion/legacy_packaging/validation.json',successor_bindings='research/v5/expansion/legacy_packaging/completion_dispositions.json',provider_contracts=['labs/api_depth/include/api_depth.h'],reference_gap_resolutions='Bounded Doxygen/CMake/ctypes/Git/Hanson3.2 and selected package notices freshly inspected; exact C17, fresh Modern C10.1, physical board and distribution permission remain limited.')
  d['status']['execution']='See source-bound T22 author receipt; independent learner/review/physical evidence not inferred';path.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
 print('T22 additive support attached: four projects, unchanged criterion/variant IDs')
if __name__=='__main__':main()
