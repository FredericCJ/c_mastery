"""Independent hashlib tree/path and selective-repair request oracle."""
import argparse,hashlib,json,random,subprocess
from collections import Counter
H=lambda b:hashlib.sha256(b).digest()
def raw(leaves):
 if not leaves:return H(b'')
 if len(leaves)==1:return H(b'\x00'+leaves[0])
 k=1<<((len(leaves)-1).bit_length()-1);return H(b'\x01'+raw(leaves[:k])+raw(leaves[k:]))
def root(domain,leaves):return H(b'\x02'+domain.to_bytes(4,'big')+len(leaves).to_bytes(4,'big')+raw(leaves))
def path(leaves,index):
 if len(leaves)==1:return []
 k=1<<((len(leaves)-1).bit_length()-1)
 return path(leaves[:k],index)+[raw(leaves[k:])] if index<k else path(leaves[k:],index-k)+[raw(leaves[:k])]
def verify(leaf,index,count,proof):
 fn=index;sn=count-1;r=H(b'\x00'+leaf)
 for p in proof:
  assert sn
  if fn&1 or fn==sn:
   r=H(b'\x01'+p+r)
   if not fn&1:
    while fn and not fn&1:fn>>=1;sn>>=1
  else:r=H(b'\x01'+r+p)
  fn>>=1;sn>>=1
 assert sn==0
 return r

def proof_cases():
 rng=random.Random(0x767007);rows=[]
 for domain in [0,1,0xffffffff]:
  for count in range(17):
   for width in [0,1,4,31,32]:
    leaves=[rng.randbytes(width) for _ in range(count)]
    for index in range(max(count,1)):rows.append((domain,leaves,index))
 for _ in range(256):
  count=rng.randrange(17);rows.append((rng.randrange(1<<32),[rng.randbytes(rng.randrange(33)) for _ in range(count)],rng.randrange(count) if count else 0))
 lines=[];expected=[];verified=0
 for domain,leaves,index in rows:
  values=[domain,len(leaves),index]
  for l in leaves:values += [len(l),*l]
  lines.append(' '.join(map(str,values)));proof=path(leaves,index) if leaves else []
  if leaves:assert verify(leaves[index],index,len(leaves),proof)==raw(leaves);verified+=1
  expected.append(root(domain,leaves).hex()+' '+str(len(proof))+''.join(' '+h.hex() for h in proof))
 return lines,expected,verified

def requests(a,b,full):
 if not full and raw(a)==raw(b):return []
 if len(b)==1:return [1]
 k=1<<((len(b)-1).bit_length()-1);return [0]+requests(a[:k],b[:k],full)+requests(a[k:],b[k:],full)
def repair_cases():
 rng=random.Random(0x767107);rows=[]
 for n in range(9):
  for mask in range(1<<n):
   for full in [0,1,2]:rows.append((n,mask,1+(mask*7+n)%108,full,0,0))
 for full in [0,1,2]:
  for fail in range(1,16):rows.append((8,255,1+fail,full,fail,0))
 for _ in range(512):
  n=rng.randrange(1,17);rows.append((n,rng.randrange(1<<n),rng.randrange(1,109),rng.randrange(3),rng.choice([0,0,0,1,2,3,4,5]),rng.choice([0,0,0,3])))
 lines=[];expected=[];counts=Counter();totalrequests=0
 for n,mask,chunk,full,fail,mode in rows:
  remote=[(i+1).to_bytes(4,'big') for i in range(n)];local=[bytes([9 if mask&(1<<i) else 0])+remote[i][1:] for i in range(n)]
  visit=([1]*n if full==2 else requests(local,remote,full)) if n else []
  status=0
  if visit and (fail==1 or full==2 and fail and fail<=len(visit)):status=5
  elif visit and mode==3:status=3
  elif fail and fail<=len(visit):status=5
  target=local if status else remote
  metrics=[0,0,0,0] if status else [len(visit),sum(visit),len(visit)*27,len(visit)*108]
  lines.append(' '.join(map(str,(n,mask,chunk,full,fail,mode))));expected.append(f'{status} '+root(7,target).hex()+' '+' '.join(map(str,metrics)));counts[status]+=1;totalrequests+=metrics[0]
 return lines,expected,counts,totalrequests

def execute(driver,mode,lines,expected):
 p=subprocess.run([driver,mode],input='\n'.join(lines)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr;observed=p.stdout.splitlines();assert len(observed)==len(expected)
 for i,(x,y) in enumerate(zip(observed,expected)):assert x==y,(mode,i,lines[i],x,y)
 return hashlib.sha256(p.stdout.encode()).hexdigest()
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);a=ap.parse_args();p,pe,pv=proof_cases();r,re,rc,rq=repair_cases();ph=execute(a.driver,'oracle',p,pe);rh=execute(a.driver,'repair_oracle',r,re);print(json.dumps(dict(passed=True,proof_rows=len(p),independently_verified_paths=pv,proof_output_sha256=ph,repair_rows=len(r),repair_status_counts=dict(rc),successful_requests=rq,repair_output_sha256=rh,limits=['Hashlib agreement and finite paths do not prove collision resistance.','Synthetic synchronous framed transport; no deployed CT service, persistence or live network is involved.'])))
