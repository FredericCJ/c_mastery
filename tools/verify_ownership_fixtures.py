"""Independent sets/state oracle; never treats native undefined behavior as data."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import subprocess

ROOT=Path(__file__).resolve().parents[1]

def borrow_pair(row):
 extent,a,n,aw,b,m,bw=row
 valid=lambda start,count,write: write in (0,1) and 0<=start<=extent and 0<=count<=extent-start
 first=0 if valid(a,n,aw) else 1
 second=0 if valid(b,m,bw) else 1
 if first==second==0 and (aw or bw) and set(range(a,a+n)) & set(range(b,b+m)):second=4
 return [first,second]

def derive(case):
 kind=case['kind'];v=case['input']
 if kind=='frontier':
  acquired=min(v['fail_at']-1,3)
  return dict(status='NOMEM' if v['fail_at']<=3 else 'OK',releases=acquired,live_after_cleanup=0)
 if kind=='scope':
  return [digit for when,digit in reversed(v['entries']) if when=='always' or (when=='success')==v['success']]
 if kind=='move':return dict(source=[],destination=v['source'],release_count=len(v['source']))
 if kind=='borrow':return borrow_pair(v)
 if kind=='cell':return dict(status='BUSY',value=v['value'],old_output=v['sentinel'])
 if kind=='shared':
  strong,weak=1,1;destroyed=released=0;trace=[]
  for op in v:
   status='OK'
   if op=='retain':strong+=1
   elif op=='downgrade':weak+=1
   elif op=='drop':
    strong-=1
    if strong==0:destroyed+=1;weak-=1;released+=weak==0
   elif op=='upgrade':
    if strong==0:status='DEAD'
    else:strong+=1
   elif op=='weak_drop':weak-=1;released+=weak==0
   trace.append([strong,weak,destroyed,released,status])
  return trace
 if kind=='cow':
  changed=list(v['original']);changed[v['index']]=v['value']
  return dict(failed_owner=v['original'],old_owner=v['original'],new_owner=changed)
 if kind=='pin':return ['OK','OK','BUSY','OK','OK']
 if kind=='allocator':return dict(attempts=2,peak=1,releases=1,live=0,poison=-(2**31))
 if kind=='facade':
  state=[1]*8+[99]*8;modulus=2**v['bits'];off=v['offset'];n=v['count']
  if (off+n)%modulus<=8:
   for i in range(n):state[((off+i)%modulus)%16]=v['value']
  return dict(flawed=state,repaired_status='INVALID',repaired=[1]*8+[99]*8)
 raise ValueError(kind)

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--driver');args=parser.parse_args()
 original=ROOT/'labs/algorithms/include/v4_algo.h';copy=ROOT/'labs/ownership/include/imports/v4_algo.h'
 if original.read_bytes()!=copy.read_bytes():raise SystemExit('Imported allocator contract differs from accepted header')
 data=json.loads((ROOT/'fixtures/ownership/cases.json').read_text(encoding='utf-8'))
 for row in data['cases']:
  if derive(row)!=row['expected']:raise SystemExit('Independent fixture disagreement: '+row['id'])
 cases=[]
 for extent in range(6):
  ranges=[(start,count) for start in range(extent+2) for count in range(extent+2)]
  for (a,n),(b,m),aw,bw in itertools.product(ranges,ranges,(0,1),(0,1)):
   cases.append((extent,a,n,aw,b,m,bw))
 if args.driver:
  text=''.join(' '.join(map(str,row))+'\n' for row in cases)
  p=subprocess.run([args.driver],input=text,capture_output=True,text=True,timeout=60)
  if p.returncode:raise SystemExit(p.stdout+p.stderr)
  output=p.stdout.splitlines()
  if len(output)!=len(cases):raise SystemExit('Wrong number of oracle responses')
  for row,line in zip(cases,output):
   if list(map(int,line.split()))!=borrow_pair(row):raise SystemExit('Borrow-set disagreement: '+str(row))
 print(json.dumps(dict(status='passed',fixture_cases=len(data['cases']),differential_cases=len(cases) if args.driver else 0,allocator_header_sha256=hashlib.sha256(copy.read_bytes()).hexdigest(),scope='Finite set intersection and independently derived state traces; no lifetime/race proof')))

if __name__=='__main__':main()
