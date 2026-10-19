"""Independent registry association and exact generic-result oracle."""
import argparse,hashlib,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();rows=[]
for i in range(16):
 for x in range(4):rows.append(f'D {i} {x} '+(f'0 {i+x}' if i in [1,4,7] else '-1 77'))
expected=[(1,4,0.0,0),(2,0,3.5,0),(1,11,0.0,0),(1,3,0.0,0),(3,7,0.0,0),(4,14,0.0,0),(1,4,0.0,1),(1,4,0.0,1),(5,10,0.0,0)]
rows += [f'G {i} {s} {v} {r:.1f} {n}' for i,(s,v,r,n) in enumerate(expected)]
r=subprocess.run([a.driver,'oracle'],text=True,capture_output=True);assert r.returncode==0 and r.stdout.splitlines()==rows,r.stdout
print(json.dumps(dict(passed=True,cases=len(rows),trace_sha256=hashlib.sha256(('\n'.join(rows)+'\n').encode()).hexdigest())))
