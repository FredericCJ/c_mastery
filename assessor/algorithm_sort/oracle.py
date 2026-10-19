"""Independent rank/inversion observations; no algorithm solution is exported."""
import collections,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 fixture=json.loads((ROOT/'fixtures/algorithms/PR-T36-03.json').read_text(encoding='utf8'));original=[(x,ord(tag)-65) for x,tag in fixture['input']['records']];expected=[]
 for prefix in range(2,6):expected.append((prefix,sorted(original[:prefix],key=lambda x:x[0])+original[prefix:]))
 metrics=[]
 for n in range(33):
  for d in range(4):
   a=[7 if d==0 else i if d==1 else n-i if d==2 else 2**32-1 if i%5==0 else 2**31 if i%5==1 else i%3 for i in range(n)]
   inversions=sum(a[j]>a[i] for i in range(n) for j in range(i));comparisons=sum(sum(a[j]>a[i] for j in range(i))+(min(a[:i])<=a[i]) for i in range(1,n));metrics.append([n,d,comparisons,inversions,max(0,n-1),max(0,n-1)])
 assert sum(original[j][0]>original[i][0] for i in range(5) for j in range(i))==6
 assert sorted(original,key=lambda x:x[0])==[(1,1),(1,4),(2,3),(3,0),(3,2)]
 if len(sys.argv)>1:
  text=pathlib.Path(sys.argv[1]).read_text();observed=[]
  if sys.argv[2]=='prefix':
   for line in text.splitlines():
    if line.startswith('PREFIX '):
     fields=line.split();observed.append((int(fields[1]),[tuple(map(int,x.split(':'))) for x in fields[2:]]))
   assert observed==expected
   assert all(collections.Counter(x)==collections.Counter(original) for _,x in observed)
  else:
   observed=[[int(x) for x in line.split()[1:]] for line in text.splitlines() if line.startswith('METRIC ')];assert observed==metrics
 print(json.dumps(dict(status='pass',prefixes=4,metrics=132,fixture_comparisons=9,fixture_shifts=6,fixture_placements=4,sorted5_comparisons=4,reverse5_comparisons=10)))
if __name__=='__main__':main()
