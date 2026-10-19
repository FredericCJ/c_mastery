"""Integer mathematics oracle independent of the C wire/triangle algorithm."""
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
rows=[];expected=[]
for v in list(range(-4097,4097))+[-2147483648,2147483647]:
 rows.append(f'e {v}');expected.append(f'0 {(v%8192)//256} {v%256}' if -4096<=v<=4095 else '1 165 90')
for v in range(65536):
 rows.append(f'd {v}');expected.append(f'0 {v if v<4096 else v-8192}' if v<8192 else '1 99')
for v in list(range(65537))+[4294967295]:
 rows.append(f't {v}');expected.append(f'0 {v*(v+1)//2}' if v<=65535 else '1 99')
r=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True,check=True);actual=r.stdout.splitlines();assert len(actual)==len(expected)
for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(i,rows[i],x,y)
trace=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest()
print(json.dumps(dict(passed=True,encodings=65536,values=8196,triangles=65538,cases=len(rows),trace_sha256=trace)))
