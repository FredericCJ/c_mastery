"""Independent exact input arithmetic; target results remain observed binary64."""
import decimal,fractions,itertools,json,math,random,statistics,subprocess,sys
F=fractions.Fraction
cases=[];descriptors=[]
def add(line,kind,data):cases.append(line);descriptors.append((kind,data))
def hx(x):return float(x).hex()
vectors=[]
for p in itertools.permutations([2**53,1,-2**53]):vectors.append(('permutation',list(map(float,p))))
vectors += [('cancellation',[1e16,1,-1e16]),('zero',[1.,-1.]),('small',[1.]+[2**-54]*2048),('magnitude-sort',[1.,2**54,2**55,-3*2**54]),('descending',[-3*2**54,2**55,2**54,1.])]
rng=random.Random(170210)
for group in ['ordinary','cancellation','extreme','nearzero']:
 for sample in range(128):
  if group=='ordinary':v=[rng.randint(-4096,4096)/128 for _ in range(32)]
  elif group=='cancellation':
   z=math.ldexp(float(rng.randint(1,4096)),rng.randint(20,50));v=[z,1.,-z]+[rng.choice([-1.,1.])*2**-10 for _ in range(13)]
  elif group=='extreme':v=[math.ldexp(rng.choice([-1.,1.])*rng.random(),rng.randint(-330,330)) for _ in range(32)]
  else:v=[1.,-1.,math.ldexp(rng.choice([-1.,1.]),-rng.randint(20,100))]
  vectors.append((group,v))
for index,(group,v) in enumerate(vectors):
 v=list(map(float,v))
 for m in range(4):add('s '+str(m)+' '+str(len(v))+' '+' '.join(hx(x) for x in v),'sum',(index,group,m,v))
for knots in [[F(0),F(1,2),F(1)],[F(0),F(1,8),F(1)]]:
 for i in range(101):
  q=F(i,100);data=' '.join(hx(x)+' '+hx(x*x) for x in knots);add('i 3 1 '+hx(q)+' '+data,'interp',(knots,F(float(q))))
for rate,h,n in [(1,F(1,2),4),(1,3,4),(2,F(1,4),4),(1,2,4),(1,F(1,2),2),(1,F(1,4),4),(1,F(1,8),8)]:add('e '+hx(1)+' '+hx(h)+' '+hx(rate)+' '+str(n),'euler',(F(rate),F(h),n))
for n in [2,3]:
 for seed in range(128):
  a=[[F(rng.randint(-8,8)) for _ in range(n)] for _ in range(n)]
  for i in range(n):a[i][i]=sum(abs(x) for j,x in enumerate(a[i]) if j!=i)+1
  x=[F(rng.randint(-16,16)) for _ in range(n)];b=[sum(a[i][j]*x[j] for j in range(n)) for i in range(n)]
  add('g '+str(n)+' '+hx(1e-12)+' '+' '.join(hx(z) for row in a for z in row)+' '+' '.join(hx(z) for z in b),'solve',(a,b,x))
r=subprocess.run([sys.argv[1]],input='\n'.join(cases)+'\n',stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True);lines=r.stdout.splitlines();assert len(lines)==len(cases)
metrics={};replay=[];grid={};counts={};max_forward=0
for line,(kind,data) in zip(lines,descriptors):
 fields=line.split();assert int(fields[0])==0,(kind,data,line);out=[float.fromhex(x) for x in fields[1:]];counts[kind]=counts.get(kind,0)+1
 if kind=='sum':
  index,group,m,v=data;exact=sum(map(F,v),F());absolute=sum(map(lambda x:abs(F(x)),v),F());error=abs(F(out[0])-exact);key=group+'.'+str(m);metrics.setdefault(key,[]).append((float(error),None if exact==0 else float(error/abs(exact)),index))
  # A separate decimal accumulation at two precisions must agree with the exact rational input sum.
  for precision in [900,1100]:
   with decimal.localcontext() as ctx:
    ctx.prec=precision;value=sum((decimal.Decimal.from_float(x) for x in v),decimal.Decimal(0));assert F(value)==exact
  if index<11:replay.append(dict(index=index,group=group,method=m,inputs=[x.hex() for x in v],observed=out[0].hex(),exact=str(exact),condition=None if exact==0 else str(absolute/abs(exact)),absolute_error=str(error)))
  if group=='ordinary':assert error==0
  if group=='zero':assert out[0]==0
 elif kind=='interp':
  xs,q=data;lo=0 if q<=xs[1] else 1;t=(q-xs[lo])/(xs[lo+1]-xs[lo]);exact=(1-t)*xs[lo]**2+t*xs[lo+1]**2;assert abs(F(out[0])-exact)<F(1,10**14);key=str(xs[1]);error=abs(F(out[0])-q*q);grid.setdefault(key,[]).append((float(error),float(q)))
 elif kind=='euler':
  rate,h,n=data;assert all(F(z)==(1-rate*h)**i for i,z in enumerate(out))
 else:
  a,b,x=data;err=max(abs(out[i]-float(x[i])) for i in range(len(x)));assert err<1e-12;max_forward=max(max_forward,err)
summary={}
for key,rows in metrics.items():
 errors=sorted(x[0] for x in rows);maximum=max(rows,key=lambda x:x[0]);summary[key]=dict(count=len(rows),absolute_p50=errors[len(errors)//2],absolute_p95=errors[min(len(errors)-1,95*len(errors)//100)],absolute_max=maximum[0],worst_vector=maximum[2],relative_unavailable=sum(x[1] is None for x in rows))
print(json.dumps(dict(passed=True,seed=170210,cases=len(cases),counts=counts,sum_error_strata=summary,replay=replay,interpolation_grids={k:dict(max_error=max(v)[0],locations=[q for e,q in v if e==max(v)[0]]) for k,v in grid.items()},solve_max_forward=max_forward,oracle='Fraction on represented binary64 inputs; independent Decimal precision900/1100; generated diagonally dominant solves have independently chosen exact solutions')))
