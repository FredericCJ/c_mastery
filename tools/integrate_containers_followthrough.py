"""Attach T14 successor scaffolds without rewriting baseline acceptance."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/containers_depth').glob('*') if p.is_file())
 for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T14-*.json')):
  data=json.loads(path.read_text(encoding='utf8'));data['supplied_artifacts']=sorted(set(data['supplied_artifacts']+public))
  contract='labs/containers_depth/CONTRACTS.md'
  if contract not in data['shared_contracts']:data['shared_contracts'].append(contract)
  data['execution_support']={'kind':'additive source-bound original container implementations, immutable CSR/BFS composition and finite transfer models','scope':'Only exact ordinary/changed-condition mechanisms in research/v5/expansion/legacy_containers/completion_dispositions.json; physical/concurrent/persistent and independent learner evidence remain separate.','tests':['python3 tools/run_containers_followthrough.py'],'fresh_execution':'See current source-bound research/v5/expansion/legacy_containers/validation.json','author_validation':'research/v5/expansion/legacy_containers/validation.json','successor_bindings':'research/v5/expansion/legacy_containers/completion_dispositions.json'}
  data['status']['execution']='See source-bound T14 author execution receipt; independent learner assessment is not supplied'
  path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
 print('Attached T14 containers successor to nine original assessment supplements')
if __name__=='__main__':main()
