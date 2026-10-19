"""Independent flat integer/list/substring models; one fresh process per input corpus."""
import itertools,json,pathlib,random,subprocess,sys,hashlib
MOD=65521
def cases():
 inputs=[];expected=[];counts={k:0 for k in 'FSIX'}
 def add(k,text,lines):inputs.append(text);expected.extend(lines);counts[k]+=1
 vectors=[v for n in range(6) for v in itertools.product(range(3),repeat=n)]
 for original in vectors:
  v=list(original);n=len(v);ops=[];out=['0']
  def observe():
   for j in range(n+1):ops.append(f'p {j}');out.append(f'0 {sum(v[:j])} 999')
   for rank in range(sum(v)+1):
    ops.append(f's {rank}');pos=next((j for j in range(n) if sum(v[:j+1])>rank),None);out.append('2 999 999' if pos is None else f'0 999 {pos}')
  observe()
  for i,d in [(0,3),(n,-1),(0,-4),(0,-9223372036854775808),(n-1,7)]:
   ops.append(f'a {i if i>=0 else n} {d}');valid=0<=i<n and 0<=v[i]+d and sum(v)+d<=2**32-1;out.append(f'{0 if valid else 2} 999 999')
   if valid:v[i]+=d
  for rounding in [0,1]:ops.append(f'r {rounding}');out.append('0 999 999');v=[(x+rounding)//2 for x in v];observe()
  add('F','F '+str(n)+' '+' '.join(map(str,original))+' '+str(len(ops))+'\n'+'\n'.join(ops),out)
 for original in vectors:
  v=list(original);n=len(v);ops=[];out=['0']
  for a,b,l,r in [(2,3,0,n),(5,7,0,n),(0,11,n//2,n),(3,1,0,n//2)]:
   ops.append(f'a {l} {r} {a} {b}');out.append('0 999 999');v[l:r]=[(a*x+b)%MOD for x in v[l:r]]
   for lo in range(n+1):
    for hi in range(lo,n+1):
     h=0
     for x in v[lo:hi]:h=(h*257+x)%MOD
     ops.append(f'q {lo} {hi}');out.append(f'0 {h} {hi-lo}')
  ops.append(f'a 0 {n+1} 1 0');out.append('2 999 999')
  add('S','S '+str(n)+' '+' '.join(map(str,original))+' '+str(len(ops))+'\n'+'\n'.join(ops),out)
 rng=random.Random(7281)
 for closed in [0,1]:
  for trial in range(80):
   values={};ops=[];out=['0']
   for i in range(100):
    lo=rng.randrange(-12,13);hi=lo+rng.randrange(0,9);id=rng.randrange(20)
    if i%3==0:
     ops.append(f'a {lo} {hi} {id}');status=2 if not closed and hi==lo else 4 if (lo,id) in values else 3 if len(values)==64 else 0
     if status==0:values[lo,id]=hi
     out.append(str(status))
    elif i%3==1:
     ops.append(f'd {lo} {id}');out.append('0' if (lo,id) in values else '5');values.pop((lo,id),None)
    else:
     ops.append(f'q {lo} {hi}');matches=[]
     if closed or hi>lo:matches=[(l,h,j) for (l,j),h in sorted(values.items()) if (l<=hi and lo<=h if closed else l<hi and lo<h)]
     status=0 if closed or hi>lo else 2;out.append(' '.join(map(str,[status,len(matches),*[z for x in matches for z in x]])))
   add('I',f'I {closed} {len(ops)}\n'+'\n'.join(ops),out)
 strings=[bytes(v) for n in range(7) for v in itertools.product([0,1,255],repeat=n)]+[b'a'*64,bytes(range(64)),bytes([255,0])*32,b'banana']
 patterns=[bytes(v) for n in range(4) for v in itertools.product([0,1,255],repeat=n)]
 for s in strings:
  n=len(s);order=sorted(range(n+1),key=lambda i:s[i:]);lcp=[0]
  for a,b in zip(order,order[1:]):
   k=0
   while a+k<n and b+k<n and s[a+k]==s[b+k]:k+=1
   lcp.append(k)
  out=[' '.join(map(str,[0,*order,*lcp]))]
  ps=patterns+[s[:min(n,5)],s[-min(n,4):]]
  for p in ps:
   lo=next((i for i,pos in enumerate(order) if s[pos:pos+len(p)]>=p),n+1);hi=next((i for i,pos in enumerate(order) if s[pos:pos+len(p)]>p),n+1);out.append(f'0 {lo} {hi}')
  add('X','X '+str(n)+' '+' '.join(map(str,s))+' '+str(len(ps))+'\n'+'\n'.join(str(len(p))+' '+' '.join(map(str,p)) for p in ps),out)
 return '\n'.join(inputs)+'\n','\n'.join(expected)+'\n',counts
def main():
 import argparse
 parser=argparse.ArgumentParser();parser.add_argument('--driver');args=parser.parse_args()
 source,want,counts=cases()
 if not args.driver:
  assert counts and all(x>0 for x in counts.values()) and len(source)>10000 and len(want.splitlines())>10000
  print(json.dumps(dict(passed=True,scope='Independent model fixture construction only; no C execution',cases=counts,observations=len(want.splitlines()),input_sha256=hashlib.sha256(source.encode()).hexdigest(),expected_sha256=hashlib.sha256(want.encode()).hexdigest())))
  return
 r=subprocess.run([args.driver],input=source,text=True,capture_output=True,timeout=120)
 if r.returncode or r.stdout!=want:
  a=r.stdout.splitlines();b=want.splitlines();first=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)));raise AssertionError((r.returncode,first,a[first:first+2],b[first:first+2],r.stderr[-1000:]))
 print(json.dumps(dict(passed=True,cases=counts,observations=len(want.splitlines()),input_sha256=hashlib.sha256(source.encode()).hexdigest(),output_sha256=hashlib.sha256(r.stdout.encode()).hexdigest(),limits='Independent finite models; no full-domain or physical/learner claim.')))
if __name__=='__main__':main()
