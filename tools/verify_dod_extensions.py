"""Integer/sequence oracle independent of the C layout and lane operations."""
import argparse
import itertools
import json
from pathlib import Path
import runpy
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def expected():
 data=json.loads((ROOT/'fixtures/dod/extensions.json').read_text(encoding='utf-8'))
 rows=[]
 for lo1,lo2,hi1,hi2 in itertools.product(data['lane_edges'],repeat=4):
  rows.append(dict(kind='swar16',a=lo1+hi1*65536,b=lo2+hi2*65536,
                   sum=min(65535,lo1+lo2)+65536*min(65535,hi1+hi2)))
 for n in range(9):
  for cold in range(2):
   rows.append(dict(kind='tiles',count=n,cold=cold,rows=[[i-3,i%3-1,i+40 if cold else 0] for i in range(n)]))
 return rows
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--driver');args=parser.parse_args()
 runpy.run_path(str(ROOT/'labs/dod/tests/verify_fixtures.py'))
 want=expected()
 assert len(want)==1314
 if args.driver:
  r=subprocess.run([args.driver,'oracle'],check=True,capture_output=True,text=True,timeout=20)
  got=[json.loads(line) for line in r.stdout.splitlines()]
  if got!=want:raise ValueError('C observations differ from independent lane/tile model')
  expected_map=[]
  for a,b in itertools.product(range(32),repeat=2):
   entries={a:10,b:20};del entries[a]
   expected_map.append(dict(kind='delete',a=a,b=b,entries=[[k,v] for k,v in sorted(entries.items())]))
  expected_edits=[]
  for choice in range(243):
   script=choice;values=[]
   for step in range(5):
    op=script%3;script//=3;at=step%(len(values)+1)
    if op<2:values[at:at]=[97,0][:op+1]
    elif at<len(values):del values[at]
    expected_edits.append(dict(kind='edit',script=choice,step=step,bytes=values.copy()))
  for program,expected_rows in [('dod-persistent',expected_map),('dod-text',expected_edits)]:
   result=subprocess.run([str(Path(args.driver).parent/program),'oracle'],check=True,capture_output=True,text=True,timeout=30)
   if [json.loads(line) for line in result.stdout.splitlines()]!=expected_rows:raise ValueError(program+' differs from independent dictionary/sequence model')
  spec=json.loads((ROOT/'fixtures/dod/buffers.json').read_text(encoding='utf8'))
  expected_buffers=[]
  def signed(value):return value if value<32768 else value-65536
  for lo1,lo2,hi1,hi2 in itertools.product(spec['signed_edges'],repeat=4):
   a=lo1+hi1*65536;b=lo2+hi2*65536
   low=max(-32768,min(32767,signed(lo1)+signed(lo2)))&65535
   high=max(-32768,min(32767,signed(hi1)+signed(hi2)))&65535
   expected_buffers.append(f'S {a} {b} {low+65536*high}')
  for cut in spec['failure_cuts']:
   length=32 if cut==0 else 16;live=2*length;peak=96 if cut==0 else 32*cut
   attempts=4 if cut!=1 else 3;releases=4 if cut==0 else cut+1
   expected_buffers.append(f'B {cut} {0 if cut==0 else 5} {length} {length} {live} {peak} {attempts} 0 {releases}')
  observed=subprocess.run([str(Path(args.driver).parent/'dod-buffers'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if observed!=expected_buffers:raise ValueError('Signed lane or quota rollback differs from independent model')
  spec=json.loads((ROOT/'fixtures/dod/dictionary.json').read_text(encoding='utf8'))
  keys=[(k[0]+32 if 65<=k[0]<=90 else k[0],k[1]) for k in spec['keys']]
  expected_dictionary=[]
  for mode,a,b in itertools.product(range(2),range(8),range(8)):
   versions=[{}, {keys[a]:10}];v=versions[1].copy();v[keys[b]]=20;versions.append(v);v=v.copy();del v[keys[a]];versions.append(v)
   for version,entries in enumerate(versions):
    for q,key in enumerate(keys):expected_dictionary.append(f'M {mode} {a} {b} {version} {q} {entries.get(key,-999)}')
  for n in range(1,10):expected_dictionary.append(f'C {n} {0 if n<=8 else 2} {n-1} {17 if n<=8 else 0} {17 if n<=8 else 0} {17*min(n,8)}')
  actual=subprocess.run([str(Path(args.driver).parent/'dod-dictionary'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if actual[:-1]!=expected_dictionary or not actual[-1].startswith('L '):raise ValueError('Dictionary snapshots or collision work differ from independent model')
  entity_rows=[]
  for script in range(729):
   code=script;slots=[[0,1,0] for _ in range(4)];latest=None;first=None;version=1
   for step in range(6):
    op=code%3;code//=3
    if op==0:
     free=next((i for i,s in enumerate(slots) if not s[0] and s[1]<=3),None)
     if free is None or sum(s[2]==1 for s in slots)==2:status=2
     else:
      slots[free][0]=1;slots[free][2]=1;latest=(free,slots[free][1]);first=first or latest;version+=1;status=0
    else:
     handle=latest if op==1 else first
     if handle is None or not slots[handle[0]][0] or slots[handle[0]][1]!=handle[1]:status=4
     elif op==1:slots[handle[0]][0]=0;slots[handle[0]][1]+=1;slots[handle[0]][2]=0;version+=1;status=0
     elif slots[handle[0]][2]!=2 and sum(s[2]==2 for s in slots)==2:status=2
     else:slots[handle[0]][2]=2;version+=1;status=0
    entity_rows.append(' '.join(map(str,['E',script,step,status,version,*[x for s in slots for x in s]])))
  actual=subprocess.run([str(Path(args.driver).parent/'dod-entities'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if actual!=entity_rows:raise ValueError('Recycled entity traces differ from independent slot-generation dictionary')
  rope_rows=[]
  def fib(n):
   a,b=0,1
   for _ in range(n):a,b=b,a+b
   return min(a,129)
  for n in range(1,17):
   for rebuilt in range(2):
    leaves=(1 if n<=8 else 2) if rebuilt else n;depth=(0 if n<=8 else 1) if rebuilt else n-1;minimum=fib(depth+2)
    rope_rows.append(f'P {n} {rebuilt} {depth} {n} {2*leaves-1} {leaves} {n} {8*leaves-n} {minimum} {int(n>=minimum)}')
  actual=subprocess.run([str(Path(args.driver).parent/'dod-rope-stats'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if actual!=rope_rows:raise ValueError('Rope balance or fragmentation observations differ from independent tree model')
  ordered_rows=[' '.join(map(str,['O',removed,24,4,*[x for x in range(4) if x!=removed],9])) for removed in range(4)]
  actual=subprocess.run([str(Path(args.driver).parent/'dod-ordered'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if actual!=ordered_rows:raise ValueError('Linked insertion order or operation counters differ from independent model')
  history_rows=[]
  for code in range(729):
   script=code;versions=[[0]*8]
   for step in range(3):
    op=script%9;script//=9;current=versions[-1].copy();current[[0,3,7][op//3]]=[0,1,-1][op%3];versions.append(current)
    for version,values in enumerate(versions):history_rows.append(' '.join(map(str,['T',code,step,version,*values])))
  for code in range(512):
   script=code;generations={i:1 for i in range(4)};live={};latest={}
   for step in range(3):
    op=script%8;script//=8;identity=op%4
    if op<4:
     slot=next((i for i in range(4) if i not in live and generations[i]<=3),None)
     if slot is None:status=2
     else:live[slot]=identity;latest[identity]=(slot,generations[slot]);status=0
    else:
     token=latest.get(identity)
     if token is None or token[0] not in live or generations[token[0]]!=token[1]:status=4
     else:del live[token[0]];generations[token[0]]+=1;status=0
    fields=[v for i in range(4) for v in (generations[i],int(i in live),live.get(i,0))]
    history_rows.append(' '.join(map(str,['S',code,step,status,len(live),*fields])))
  actual=subprocess.run([str(Path(args.driver).parent/'dod-tests'),'oracle'],check=True,capture_output=True,text=True).stdout.splitlines()
  if actual!=history_rows:raise ValueError('Persistent histories or sparse lifetimes differ from independent snapshots/dictionaries')
 print(json.dumps(dict(passed=True,oracle_rows=len(want),dictionary_cases=1024,edit_prefixes=1215,signed_lane_cases=2401,quota_cases=3,custom_dictionary_cases=4105,entity_prefixes=4374,rope_statistics=32,ordered_cases=4,history_rows=8097,c_executed=bool(args.driver),limits='Finite lane, tile, dictionary, edit and quota cases; ownership/query invariants have separate C traces. No universal performance claim.')))
if __name__=='__main__':main()
