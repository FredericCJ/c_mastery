"""Set-based interval and explicit forward-ring generation oracle, separate from C."""
import sys,json
rows=0
for line in open(sys.argv[1],encoding='utf8'):
 p=line.split();v=list(map(int,p[1:]));rows+=1
 if p[0]=='R':
  a,n,b,m,contains,overlap=v;outer=set(range(a,a+n));inner=set(range(b,b+m))
  assert contains==int(inner<=outer and (bool(inner) or a<=b<=a+n)),line
  assert overlap==int(bool(outer&inner)),line
 elif p[0]=='G':
  x,y,status,which=v;forward=[(y+k)%256 for k in range(1,128)]
  expected=5 if x==y or x==(y+128)%256 else 0
  assert status==expected and which==(9 if expected else (0 if x in forward else 1)),line
 elif p[0]=='H':
  bits,status=v;assert status==(0 if bits==239 else 1),line
 elif p[0]=='M':
  at,length,status,*memory=v;first=set(range(2,5));second=set(range(at,at+length));zero=set(range(16,20));kept=set(range(24,28))
  valid=at+length<=32 and not(second&(first|zero|kept));expected=[165]*32
  if valid:
   for dest,source in zip(range(2,5),range(3)):expected[dest]=3*source+1
   for dest,source in zip(range(at,at+length),range(17,17+length)):expected[dest]=3*source+1
   for dest in zero:expected[dest]=0
  assert status==(0 if valid else 1) and memory==expected,line
assert rows==16*9*24*9+65536+256+32*9,rows
print(json.dumps(dict(passed=True,rows=rows,scope='Finite independent interval/generation model; no retained RAM persistence or hardware claim')))
