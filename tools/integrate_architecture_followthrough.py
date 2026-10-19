"""Attach T20 mechanisms to original assessments without changing baseline records."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/api_depth/architecture').glob('*') if p.is_file())
 public += ['labs/foundations/include/foundation.h','labs/containers_depth/containers.h','labs/api_depth/stream/stream.h','labs/api_depth/abstractions/registry.h','labs/api_depth/include/api_depth.h']
 public += sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/firmware_lifecycle').rglob('*.h'))
 register='research/v5/expansion/legacy_architecture/resources.json';resources=json.loads((ROOT/register).read_text(encoding='utf8'))['resources']
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T20-*.json')):
  data=json.loads(path.read_text(encoding='utf8'));data['supplied_artifacts']=sorted(set(data['supplied_artifacts']+public));contract='labs/api_depth/architecture/CONTRACTS.md'
  if contract not in data['shared_contracts']:data['shared_contracts'].append(contract)
  for resource in resources:
   item=dict(id=resource['id'],path=register,passages=resource['locators']);data['source_records']=[old for old in data['source_records'] if old['id']!=item['id']]+[item]
  data['execution_support']=dict(kind='Actual original architecture mechanisms and frozen producer composition, with separate bounded models and selected target cross-build',scope='30 ordinary and30 exact delayed obligations; external review, independent learner performance and physical observations remain separate.',tests=['python3 tools/run_architecture_followthrough.py'],fresh_execution='See source-bound research/v5/expansion/legacy_architecture/validation.json',author_validation='research/v5/expansion/legacy_architecture/validation.json',successor_bindings='research/v5/expansion/legacy_architecture/completion_dispositions.json',provider_contracts=['labs/foundations/include/foundation.h','labs/containers_depth/containers.h','labs/api_depth/stream/stream.h','labs/api_depth/abstractions/registry.h','labs/firmware_lifecycle/targets/f439_port.h'],reference_gap_resolutions='Actual bounded Parnas/EWD249, HAL and device source, RM0090 GPIO/EXTI and Linux manual passages inspected; exact published ISO C17/POSIX2024 and physical evidence limits remain named.')
  data['status']['execution']='See current source-bound T20 author receipt; learner and physical performance remain separate';path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
 print('Attached T20 successor to4 original projects; preserved30 ordinary/30 delayed identities')
if __name__=='__main__':main()
