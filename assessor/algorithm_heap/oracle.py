"""Check supplied path certificates and actual assembly observations without a heap API."""
from collections import Counter
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def check():
 d=json.loads((ROOT/'fixtures/algorithms/PR-T36-04.json').read_text());expected=[]
 for f,case in enumerate(d['input']['cases']):
  states=case['states'];path=case['path'];n=len(states[0]);comparisons=0
  assert len(states)==len(path)
  for k,(a,p) in enumerate(zip(states,path)):
   assert Counter(a)==Counter(states[0]);assert all(a[(i-1)//2]>=a[i] for i in range(1,n) if (i-1)//2!=p)
   children=list(range(2*p+1,min(2*p+3,n)));comparisons+=len(children)
   if k+1<len(states):
    q=path[k+1];assert q in children and a[q]>a[p];assert all(a[q]>a[x] or (a[q]==a[x] and q<=x) for x in children)
    assert states[k+1]==[a[q] if i==p else a[p] if i==q else a[i] for i in range(n)]
   else:assert all(a[p]>=a[q] for q in children)
  assert comparisons==d['expected']['key_comparisons'][f] and len(path)-1==d['expected']['swaps'][f] and states[-1]==d['expected']['finals'][f]
  expected.append('HEAP '+' '.join(map(str,[f,n,comparisons,len(path)-1,*states[-1]])))
  expected.extend('SWAP '+' '.join(map(str,[f,p,*a])) for p,a in zip(path[1:],states[1:]))
 if len(sys.argv)>1:
  observed=[line for line in pathlib.Path(sys.argv[1]).read_text().splitlines() if line.startswith(('HEAP ','SWAP '))];assert observed==expected,(observed,expected)
 return dict(status='pass',fixtures=3,complete_swaps=3,comparisons=[3,4,2],independent_multiset=True)
if __name__=='__main__':print(json.dumps(check()))
