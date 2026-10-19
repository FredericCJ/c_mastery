"""Independent explicit-position enumeration; checks actual emitted result packets."""
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def matches(t,p):
 found=[];cost=0
 for start in range(len(t)-len(p)+1):
  for j,value in enumerate(p):
   cost+=1
   if t[start+j]!=value:break
  else:found.append(start)
 return found,cost
def rows():
 for n in range(6):
  for bits in range(1<<n):
   for m in range(4):
    for pattern in range(1<<m):
     yield (bits,n,pattern,m),matches([(bits>>i)&1 for i in range(n)],[(pattern>>i)&1 for i in range(m)])
def main():
 fixture=json.loads((ROOT/'fixtures/algorithms/PR-T36-06.json').read_text(encoding='utf8'))
 # Separate literal expected relations from the C/provider algorithm.
 assert matches([0,97,0,97,0],[97,0])==([1,3],6)
 assert matches([0,97,0,97,0],[0,97,0])==([0,2],7)
 assert matches([0,97,0,97,0],[])==([0,1,2,3,4,5],0)
 def lookup(slots,key):
  path=[]
  for i in range(len(slots)):
   at=(key+i)%len(slots);path.append(at);state,value=slots[at]
   if state=='empty':return path,None
   if state=='occupied' and value==key:return path,at
  return path,None
 f=fixture['input'];e=fixture['expected'];queries=[lookup(f['slots'],k) for k in f['lookup_keys']]
 assert [x[0] for x in queries]==e['probe_indices'] and [x[1] for x in queries]==e['lookup_results']
 assert len(lookup(f['full_slots'],f['full_absent_key'])[0])==e['full_absent_probes']
 results=[matches(f['text'],p) for p in f['patterns']]
 assert [x[0] for x in results]==e['match_offsets'] and [x[1] for x in results]==e['byte_comparisons']
 actual=0;kind=sys.argv[2] if len(sys.argv)>2 else None
 if len(sys.argv)>1:
  lines=pathlib.Path(sys.argv[1]).read_text().splitlines()
  if kind=='matching':
   expected={k:(len(v[0]),v[1],*v[0]) for k,v in rows()};observed={}
   for line in lines:
    if line.startswith('MATCH '):
     values=list(map(int,line.split()[1:]));key=tuple(values[:4]);assert key not in observed;observed[key]=tuple(values[4:])
   assert observed==expected;actual=len(observed)
  elif kind=='stream':
   expected={(*k,split):(len(v[0]),*v[0]) for k,v in rows() for split in range(k[1]+1)};observed={}
   for line in lines:
    if line.startswith('STREAM '):
     values=list(map(int,line.split()[1:]));key=tuple(values[:5]);assert key not in observed;observed[key]=tuple(values[5:])
   assert observed==expected;actual=len(observed)
  elif kind=='lookup':
   observed=[tuple(map(int,line.split()[1:])) for line in lines if line.startswith('LOOKUP ')]
   assert observed==[(0,22,-1,6,6,7,0,1,2,3),(0,16,0,1,0),(0,2,2,1,2),(2,0,-1,8,0,1,4,1,0,1,4,1)];actual=len(observed)
  else:raise AssertionError('unknown actual packet kind')
 print(json.dumps(dict(status='pass',fixture_relations=3,actual_packets=actual,kind=kind)))
if __name__=='__main__':main()
