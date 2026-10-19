"""Read-only discovery report for canonical resource shapes and duplicate identities."""
from collections import defaultdict,Counter
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
def main():
 paths=list((ROOT/'research/v5').glob('*resources.json'))+list((ROOT/'research/v5/expansion').rglob('*resources.json'))+list((ROOT/'research/v5/modern').glob('*resources.json'))
 rows=defaultdict(list);skipped=[];shapes=[]
 for p in sorted(paths):
  relative=p.relative_to(ROOT).as_posix()
  if any(x=='history' or x.startswith('history_') or x.startswith('history-') for x in p.relative_to(ROOT).parts):skipped.append(relative);continue
  v=json.loads(p.read_text(encoding='utf8'));entries=v if isinstance(v,list) else v.get('resources')
  shapes.append(dict(path=relative,keys=sorted(v) if isinstance(v,dict) else 'list',rows=None if entries is None else len(entries),entry_keys=sorted({tuple(sorted(r)) for r in entries}) if isinstance(entries,list) else []))
  if isinstance(entries,list):
   for r in entries:
    if 'id' in r:rows[r['id']].append(dict(path=relative,record=r))
 duplicates=[]
 for ident,group in rows.items():
  if len(group)<2:continue
  keys=set().union(*(r['record'] for r in group));different=[k for k in keys if any(r['record'].get(k)!=group[0]['record'].get(k) for r in group[1:])]
  duplicates.append(dict(id=ident,paths=[r['path'] for r in group],different_fields=sorted(different),records=[r['record'] for r in group]))
 value=dict(paths=len(paths),excluded_history=skipped,shapes=shapes,duplicate_ids=duplicates);dest=ROOT/'research/v5/expansion/resource_merge_repair/discovery.json';dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps(dict(paths=len(paths),excluded_history=len(skipped),duplicate_ids=[{k:v for k,v in row.items() if k!='records'} for row in duplicates],nonstandard=[r for r in shapes if r['rows'] is None or any('id' not in keys or 'inspection' not in keys for keys in r['entry_keys'])]),indent=2))
if __name__=='__main__':main()
