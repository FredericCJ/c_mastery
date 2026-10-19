"""Independent unbounded integer oracle for ordinary and changed result types."""
import argparse,hashlib,json,subprocess,itertools
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();rows=[];expected=[];counts={}
def add(op,x,y,result):rows.append(f'{op} {x} {y}');expected.append(result);counts[op]=counts.get(op,0)+1
signed=[-2147483648,-2147483647,-65536,-65535,-1,0,1,65535,65536,2147483646,2147483647]
for x,y in itertools.chain(itertools.product(range(-128,128),repeat=2),itertools.product(signed,repeat=2)):
 v=x+y;add('a',x,y,f'0 {v}' if -2147483648<=v<=2147483647 else '1 77')
for x,y in itertools.product(signed+[i for i in range(-128,129)],[0,1,127,255,65534,65535,65536,65537,2147483647,2147483648,4294967295]):
 v=x+y;add('m',x,y,f'0 {v}' if 0<=v<=65535 else '1 77')
for x in list(range(65536))+[1<<i for i in range(32)]+[4294967295]:
 width=x.bit_length();trailing=32 if not x else (x&-x).bit_length()-1;add('b',x,0,f'0 {32-width} {trailing} {width}')
u=sorted(set([0,1,2,3,18446744073709551615]+[1<<i for i in range(64)]+[(1<<i)-1 for i in range(1,65)]))
for x,y in itertools.product(u,repeat=2):add('d',x,y,f'0 {x//y}' if y else '1 77')
for x,y in itertools.product(range(33),range(3)):add('s',x,y,('0 '+str(int((x&(31 if y else 15))==(31 if y else 15)))) if x<=31 and y<=1 else '1 77')
r=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True,check=True);actual=r.stdout.splitlines();assert len(actual)==len(expected)
for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(i,rows[i],x,y)
trace=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest();print(json.dumps(dict(passed=True,cases=len(rows),counts=counts,trace_sha256=trace)))
