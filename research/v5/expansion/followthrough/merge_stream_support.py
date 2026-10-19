"""Merge only the independently tracked current stream successor dispositions."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[4]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
path='research/v5/expansion/legacy_stream_followthrough/support.json';d=read(path)
for p,h in d['source_sha256'].items():assert sha(p)==h,('Stale stream support',p)
guard_path='reviews/v5/expansion/legacy_stream_followthrough/variants_support_validation_current.json';guard=read(guard_path);assert guard['passed'] and len(guard['checks'])==21
for p,h in guard['inspected_paths'].items():assert sha(p)==h,('Stale stream guard',p)
state_path=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=json.loads(state_path.read_text());index={r['id']:r for r in state['updates']}
for row in d['updates']:index[row['id']]=row
state['updates']=list(index.values());state_path.write_text(json.dumps(state,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
report=dict(passed=True,rows=len(d['updates']),total_updates=len(index),source_sha256={path:sha(path),guard_path:sha(guard_path)},scope='Author support and exact input checks; no independent learner or review claim')
(ROOT/'reviews/v5/expansion/followthrough/stream_support_merge.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(report)
