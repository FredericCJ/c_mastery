"""Merge explicitly reviewed author dispositions after checking current inputs."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json
ROOT=Path(__file__).resolve().parents[4]
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def write(p,v):(ROOT/p).write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
sources=['research/v5/expansion/legacy_posix_'+name+'/support.json' for name in ('followthrough','mapping','namespace','io','terminal','retry','workload')]
sources+=['research/v5/expansion/legacy_concurrency_lifecycle/support.json','research/v5/modern/protocols_completion_dispositions.json','research/v5/modern/coroutines_completion_dispositions.json']
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--source',action='append');parser.add_argument('--guard',action='append',default=[],help='Explicit source=guard path for shared campaign directories');parser.add_argument('--report',default='reviews/v5/expansion/followthrough/ready_support_merge.json');args=parser.parse_args()
guards=dict(pair.split('=',1) for pair in args.guard)
if args.source:sources=args.source
updates=read('curriculum/v5/expansion/implementation_updates.json');merged={r['id']:r for r in updates['updates']};evidence=[];count=0
for path in sources:
 d=read(path);pins=d.get('source_sha256',d.get('input_sha256',{}));assert pins,path
 for name,digest in pins.items():assert sha(name)==digest,('Stale disposition input',path,name)
 explicit_guard=path in guards
 if '/legacy_' in path or explicit_guard:
  name=path.split('/')[-2];guard_name=Path(path).stem+'_validation.json';guard_path=guards.get(path,'reviews/v5/expansion/'+name+'/'+guard_name);guard=read(guard_path);assert guard['passed'],name
  for name,digest in guard.get('input_sha256',{}).items():assert sha(name)==digest,('Stale guard input',guard_path,name)
 else:
  for report in d['current_reports']:
   r=read(report);assert r.get('passed') is True or r.get('status')=='passed' or (r.get('status')=='pass' and r.get('commands') and all(c.get('passed') is True for c in r['commands'])),(path,report)
 for row in d['updates']:
  assert set(row['dimensions'])=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},row['id']
  merged[row['id']]=row;count+=1
 evidence.append(dict(path=path,sha256=sha(path),rows=len(d['updates']),current_input_count=len(pins),guard=dict(path=guard_path,sha256=sha(guard_path)) if '/legacy_' in path or explicit_guard else None))
updates['updates']=list(merged.values());write('curriculum/v5/expansion/implementation_updates.json',updates)
write(args.report,dict(observed_at=datetime.now(timezone.utc).isoformat(),scope='Current source identities and reported gates checked; parallel authorship is not independent review',merged_rows=count,total_updates=len(merged),sources=evidence))
print(dict(merged_rows=count,total_updates=len(merged)))
