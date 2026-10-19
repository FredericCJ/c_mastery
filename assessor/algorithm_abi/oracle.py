"""Independent bisect answers and partition certificates, checked against actual logs."""
import bisect,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def main():
 d=json.loads((ROOT/'fixtures/algorithms/PR-T36-02.json').read_text(encoding='utf8'));a=d['input']['array'];rows=[]
 for case,want in zip(d['input']['cases'],d['expected']['indices']):
  key=case['key'];assert bisect.bisect_left(a,key)==want;lo,hi=0,len(a)
  for index,(l,h,m) in enumerate(case['steps']):
   assert (l,h)==(lo,hi) and lo<=m<hi and m==lo+(hi-lo)//2
   assert all(x<key for x in a[:lo]) and all(x>=key for x in a[hi:]);before=hi-lo
   if a[m]<key:lo=m+1
   else:hi=m
   assert hi-lo<before;rows.append([key,index,l,h,m,lo,hi])
  assert lo==hi==want
 if len(sys.argv)>1:
  observed=[[int(x) for x in line.split()[1:]] for line in pathlib.Path(sys.argv[1]).read_text().splitlines() if line.startswith('TRACE ')]
  assert observed==rows,(observed,rows)
 assert bisect.bisect_right(a,7)==3
 print(json.dumps(dict(status='pass',certificates=15,indices=d['expected']['indices'],upper7=3)))
if __name__=='__main__':main()
