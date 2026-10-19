"""Exact rational expectations, changed radix, frozen-tolerance minimization."""
from fractions import Fraction as Q
from itertools import product,permutations
from pathlib import Path
import json,math

def fraction(x):return Q.from_float(x)
def gamma(k):
 u=Q(1,2**53);assert k*u<1
 return k*u/(1-k*u)
def power(radix,e):return Q(radix**e) if e>=0 else Q(1,radix**-e)
def rounded(x,radix,precision):
 if not x:return Q(0)
 sign=-1 if x<0 else 1;x=abs(x);e=0
 while x<power(radix,e):e-=1
 while x>=power(radix,e+1):e+=1
 step=power(radix,e-precision+1);v=x/step;n=v.numerator//v.denominator;r=v-n
 if r>Q(1,2) or r==Q(1,2) and n%2:n+=1
 return sign*n*step
def expansion(n,d,radix):
 remainder=n%d;seen={};digits=[]
 while remainder and remainder not in seen:
  seen[remainder]=len(digits);digit,remainder=divmod(remainder*radix,d);digits.append(digit)
 return dict(digits=digits,cycle_start=seen.get(remainder),terminating=not remainder)
def prepare(c):
 cases=[]
 alphabet=[-65536.0,-0.0625,0.0,0.0625,65536.0]
 for n in range(6):
  for x in product(alphabet,repeat=n):cases.append(dict(kind='exact-grid',a=list(x),b=[0.0625]*n))
 for x in permutations([1e16,1.0,-1e16]):cases.append(dict(kind='cancellation',a=list(x),b=[1.0]*3))
 for pairs in [1,16,256,2048]:
  a=[v for _ in range(pairs) for v in [1.0,-1.0+2**-50]]
  cases.append(dict(kind='long-alternating',a=a,b=[1.0]*len(a)))
 cases.append(dict(kind='dot-transfer',a=[1e16,1.0,-1e16],b=[1.0,1.0,1.0]))
 # Predicted bounds are written BEFORE running any tested executable.
 for row in cases:
  a=list(map(fraction,row['a']));b=list(map(fraction,row['b']));n=len(a)
  row['exact_sum']=str(sum(a,Q()));row['exact_dot']=str(sum((x*y for x,y in zip(a,b)),Q()))
  row['sum_bound']=str(gamma(max(0,n-1))*sum(map(abs,a),Q()))
  row['dot_bound']=str(gamma(2*n)*sum((abs(x*y) for x,y in zip(a,b)),Q()))
 c.write('rational-inputs.json',json.dumps(cases,indent=2)+'\n')
 inp=c.write('rational-inputs.txt',''.join(str(len(row['a']))+'\n'+''.join(a.hex()+' '+b.hex()+'\n' for a,b in zip(row['a'],row['b'])) for row in cases))
 return cases,inp
def check(c,cases,text):
 lines=[line for line in text.splitlines() if line and line[0].isdigit()]
 c.require(len(lines)==len(cases),'all independent rational cases consumed')
 maximum=Q()
 for i,(row,line) in enumerate(zip(cases,lines)):
  tokens=line.split();assert int(tokens[0])==i and len(tokens)==5
  values=[fraction(float.fromhex(x)) for x in tokens[1:]];exact=Q(row['exact_sum']);dot=Q(row['exact_dot'])
  assert abs(values[0]-exact)<=Q(row['sum_bound']),('naive bound',i)
  assert abs(values[3]-dot)<=Q(row['dot_bound']),('dot bound',i)
  if row['kind'] in ['exact-grid','long-alternating']:assert all(v==exact for v in values[:3]),('exact dyadic partition',i)
  if row['kind']=='cancellation':assert values[2]==1,('Neumaier permutation',i)
  maximum=max(maximum,abs(values[0]-exact))
 c.require(True,'exact-grid permutations long sequences and fresh dot bound',dict(cases=len(cases),max_naive_error=str(maximum)))
def independent(c):
 tenth=Q(1,10);binary=fraction(0.1);fifth=fraction(0.2);total=fraction(0.1+0.2)
 assert binary-tenth==Q(1,180143985094819840)
 conversion=(binary-tenth)+(fifth-Q(1,5));addition=total-binary-fifth
 assert total-Q(3,10)==conversion+addition and addition==Q(1,36028797018963968)
 radices=[]
 for base,prec in [(2,3),(2,8),(10,3),(10,7)]:
  entries=[]
  for e in [-4,0,4]:
   v=power(base,e);up=power(base,e-prec+1);down=power(base,e-prec)
   assert rounded(v+up,base,prec)==v+up and rounded(v-down,base,prec)==v-down
   assert rounded(v+up/2,base,prec)==v
   entries.append(dict(exponent=e,value=str(v),previous=str(v-down),next=str(v+up),lower_spacing=str(down),upper_spacing=str(up)))
  radices.append(dict(radix=base,precision=prec,neighbors=entries))
 assert not expansion(1,10,2)['terminating'] and expansion(1,10,10)['terminating'] and not expansion(1,3,10)['terminating']
 tolerance=Q(1,4);original=[0.0,1e16,0.0,1.0,-1e16,0.0]
 def discrepancy(a):
  result=0.0
  for v in a:result+=v
  return abs(fraction(result)-sum(map(fraction,a),Q()))>tolerance
 minimized=original[:];changed=True
 while changed:
  changed=False
  for i in range(len(minimized)):
   candidate=minimized[:i]+minimized[i+1:]
   if discrepancy(candidate):minimized=candidate;changed=True;break
 assert minimized==[1.0,-1e16] and all(not discrepancy(minimized[:i]+minimized[i+1:]) for i in range(2))
 # Fixed-point reference uses exact rational product, not implementation guard/rounding code.
 fixed=[]
 for a,b in [(-128,-128),(-1,8),(1,8),(127,127),(16,16),(3,-8)]:
  exact=Q(a*b,256);raw=exact*16;n=raw.numerator//raw.denominator;r=raw-n;even=n+(r>Q(1,2) or r==Q(1,2) and n%2!=0)
  clipped=max(-128,min(127,even));fixed.append(dict(a=a,b=b,exact=str(exact),rounded_raw=even,output=clipped,error=str(Q(clipped,16)-exact)))
 d=dict(tenth=dict(exact=str(tenth),binary=str(binary),conversion_error=str(binary-tenth),combined_input_error=str(conversion),addition_error=str(addition),total_error=str(total-Q(3,10))),expansions=[expansion(1,10,2),expansion(1,10,10),expansion(1,3,10)],changed_formats=radices,fixed_point=fixed,minimization=dict(original=original,result=minimized,frozen_tolerance=str(tolerance),one_deletion_minimal=True,classification='Naive algorithm does not meet this absolute contract; its forward-error bound is not violated. A missing compensation term in Neumaier is an implementation defect; naive-as-oracle is an invalid reference.'),dot_transfer=dict(exact='1',bound=str(gamma(6)*(2*10**16+1)),interpretation='Bound derived from three products plus additions, not copied sum tolerance; permits observed zero but does not meet absolute1/4 requirement.'))
 c.write('exact-models.json',json.dumps(d,indent=2)+'\n');c.require(True,'exact conversion radix boundary minimization and fixed point derivations',dict(formats=4,partitions=12,fixed_cases=6,tolerance=str(tolerance)))
