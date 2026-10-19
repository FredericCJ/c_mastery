"""Record reviewed evidence-only inputs; these are not learner reading registries."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
NAMES=['legacy_amp_followthrough','legacy_debugging_followthrough','legacy_kernel_boot','legacy_kernel_followthrough','legacy_mcu_followthrough','legacy_memory_followthrough','legacy_os_followthrough','legacy_performance_followthrough']
WRAPPERS=['legacy_numeric_followthrough','legacy_concurrency_followthrough','legacy_services_followthrough']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 rows={}
 for name in NAMES:
  p=ROOT/'research/v5/expansion'/name/'resources.json';v=json.loads(p.read_text(encoding='utf8'));assert isinstance(v.get('resources'),list) and v['resources']
  for r in v['resources']:assert r.get('id') and r.get('locators')
  rows[p.relative_to(ROOT).as_posix()]={'sha256':sha(p),'role':'author-evidence-only','reason':'Bounded capture, local-provider or retained-source evidence record; lacks full catalogue work/edition/inspection schema. Preserve it as package evidence; do not invent assignable source metadata.','ids':[r['id'] for r in v['resources']]}
 value={'schema_version':1,'evidence_only':rows,'pinned_wrappers':{f'research/v5/expansion/{name}/resources.json':('retained_registration' if name=='legacy_numeric_followthrough' else 'registered_passages') for name in WRAPPERS},'history_components':['history','history_*','history-*'],'policy':'Unknown malformed canonical resource inputs fail. Explicit evidence inputs are byte-pinned and not merged as reading records. Wrapper paths/hashes and every declared retained passage are checked. Scope and eligibility are never synthesized.'}
 p=ROOT/'research/v5/resource_merge_policy.json';assert not p.exists();p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8');print(json.dumps({'evidence_inputs':len(rows),'pinned_wrappers':len(WRAPPERS)}))
if __name__=='__main__':main()
