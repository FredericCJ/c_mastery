"""Independent Fraction oracle, including exact units and changed Q11 width."""
import argparse,hashlib,itertools,json,subprocess
from fractions import Fraction

def rounded(value,even=False):
 lower=value.numerator//value.denominator
 return min([lower,lower+1],key=lambda c:(abs(value-c),c%2 if even else -abs(c)))
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 def run(mode,cases):
  data=''.join(str(len(c))+' '+ ' '.join(f'{x} {y}' for x,y in c)+'\n' for c in cases)
  r=subprocess.run([a.driver,mode],input=data,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;rows=r.stdout.splitlines();assert len(rows)==len(cases);return rows
 cases=[[(x,y)] for x in range(-128,128) for y in range(-128,128)]
 alphabet=[-128,-64,-1,0,1,64,127];cases+=[[(a,b),(c,d)] for a,b,c,d in itertools.product(alphabet,repeat=4)]
 cases += [[],[(-128,-128)]*1024,[(-128,127)]*1024,[(127,127)]*512+[(-127,127)]*512,[(127,127),(127,127),(-128,127)]]
 digest=hashlib.sha256();clipped=ties=0
 for c,row in zip(cases,run('oracle',cases)):
  total=sum(x*y for x,y in c);exact=Fraction(total,128);away=rounded(exact);even=rounded(exact,True);result=max(-128,min(127,away));expected=[result,max(-128,min(127,even)),total,away,result*128-total,int(result!=away)]
  assert list(map(int,row.split()))==expected,(c[:8],row,expected)
  if result==away:assert abs(Fraction(result,128)-Fraction(total,16384))<=Fraction(1,256)
  clipped+=result!=away;ties+=exact.denominator==2;digest.update((' '.join(map(str,expected))+'\n').encode())
 values=[-32768,-32767,-2048,-1024,-1,0,1,1024,2048,32767]
 wide=[[(x,y)] for x,y in itertools.product(values,repeat=2)]
 wide+=[[(values[(j*3+i)%10],values[(j*7+2*i)%10]) for j in range(i%65)] for i in range(257)]
 wide += [[(-32768,-32768)]*65536,[(-32768,32767)]*65536,[(32767,32767)]*32768+[(-32767,32767)]*32768,[(-32768,-32768)]*4+[(-32768,32767)]*4]
 for c,row in zip(wide,run('wide',wide)):
  total=sum(x*y for x,y in c);assert abs(total)<=2**46;expected=max(-32768,min(32767,rounded(Fraction(total,2048))));assert int(row)==expected,(c[:8],row,expected);digest.update(f'{total} {expected}\n'.encode())
 def raw(mode,data):
  r=subprocess.run([a.driver,mode],input=data,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;return r.stdout.splitlines()
 def small(n,d,even):
  rounded_raw=rounded(Fraction(n,d),even);out=max(-128,min(127,rounded_raw));answer=[n,d,rounded_raw,out*d-n,out,int(out!=rounded_raw)]
  if not answer[-1]:assert abs(Fraction(out,16)-Fraction(n,16*d))<=Fraction(1,32)
  return answer
 pairs=list(itertools.product(range(-128,128),repeat=2));rows=raw('small_oracle',''.join(f'{a} {b}\n' for a,b in pairs));assert len(rows)==len(pairs)
 for (x,y),row in zip(pairs,rows):
  expected=[]
  for even in [False,True]:expected+=small(x*y,16,even)+small(2*x+y,2,even)
  assert list(map(int,row.split()))==expected,(x,y,row,expected);digest.update((' '.join(map(str,expected))+'\n').encode())
 assert min(x*y for x,y in pairs)==-16256 and max(x*y for x,y in pairs)==16384
 values=[-128,-127,-64,-8,-3,-1,0,1,3,8,64,127];triples=[(a,b,c,mode) for a,b,c in itertools.product(values,repeat=3) for mode in range(2)]
 rows=raw('cascade_oracle',''.join(f'{a} {b} {c} {mode}\n' for a,b,c,mode in triples));assert len(rows)==len(triples);bounded=0
 for (x,y,z,mode),row in zip(triples,rows):
  first=small(x*y,16,bool(mode));second=small(first[4]*z,16,bool(mode));error=second[4]*256-x*y*z;bound=8*abs(z)+128;valid=int(not first[5] and not second[5]);expected=first+second+[error,bound,valid]
  assert list(map(int,row.split()))==expected,(x,y,z,mode,row,expected)
  if valid:assert abs(error)<=bound;bounded+=1
  digest.update((' '.join(map(str,expected))+'\n').encode())
 print(json.dumps(dict(passed=True,q7_cases=len(cases),single_pairs=65536,two_term_cases=2401,ties=ties,clipped=clipped,q11_cases=len(wide),q11_max_terms=65536,q11_sum_bound=2**46,scale16_pairs=len(pairs),scale16_policies=2,mixed_scale_pairs=len(pairs),cascade_cases=len(triples),unclipped_cascades=bounded,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
