"""Narrow VM source-schema normalization; preserve history before any metadata edit."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PACKAGES=['representation','verifier','execution','collection','budgets','services','targets','assessment']
BASE='research/v5/expansion/vm_source_metadata'

def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def write(p,v):
 q=ROOT/p;q.parent.mkdir(parents=True,exist_ok=True);q.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def resources():return ['research/v5/expansion/legacy_vm_'+n+'/resources.json' for n in PACKAGES]

def inspect():
 rows=[]
 for path in resources():
  for r in read(path)['resources']:
   ins=r['inspection'];assert all(ins.get(k) for k in ['date','method','limits']) and r['locators']
   for p,h in ins['source_sha256'].items():assert sha(p)==h,('captured or inspected source changed',r['id'],p)
   rows.append(dict(registry=path,id=r['id'],eligible=r.get('assignable') is not False,locators=r['locators'],source_files=len(ins['source_sha256']),previous_scope=ins.get('scope')))
 return rows

def preserve():
 manifest=BASE+'/history_01/manifest.json'
 if (ROOT/manifest).exists():return
 files=set(resources())|{'research/v5/expansion/legacy_vm_representation/author_support.py','research/v5/expansion/legacy_diagnostics/validation.json'}
 for name in PACKAGES:
  folder=ROOT/'research/v5/expansion'/('legacy_vm_'+name)
  files.update(p.relative_to(ROOT).as_posix() for p in folder.iterdir() if p.is_file() and p.suffix in {'.json','.md','.py'})
  review=ROOT/'reviews/v5/expansion'/('legacy_vm_'+name)
  files.update(p.relative_to(ROOT).as_posix() for p in review.glob('*.json'))
 files.update(p.relative_to(ROOT).as_posix() for p in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T46-*.json'))
 records={}
 for p in sorted(files):
  dest=BASE+'/history_01/'+p;target=ROOT/dest;assert not target.exists();target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((ROOT/p).read_bytes());records[p]=dict(archive=dest,sha256=sha(p))
 write(manifest,records)

def main():
 p=argparse.ArgumentParser();p.add_argument('--apply',action='store_true');a=p.parse_args();rows=inspect()
 if a.apply:
  preserve()
  for path in resources():
   data=read(path)
   for r in data['resources']:
    ins=r['inspection'];assert ins.get('scope',r['locators'])==r['locators'];ins['scope']=copy.deepcopy(r['locators'])
   write(path,data)
  write(BASE+'/normalization.json',dict(passed=True,kind='schema-only-recorded-inspection-scope-normalization',entries=rows,policy='Scope is exactly the already recorded inspected locators; dates, methods, limits, captured bytes and eligibility unchanged. No new reading or execution claimed.',source_sha256={p:sha(p) for p in resources()}))
 print(json.dumps(dict(entries=len(rows),public=sum(r['eligible'] for r in rows),author_only=sum(not r['eligible'] for r in rows),missing_scope=sum(r['previous_scope'] is None for r in rows),applied=a.apply)))
if __name__=='__main__':main()
