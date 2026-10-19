"""Independent integer formatting and byte-protocol oracle."""
import argparse,hashlib,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();digest=hashlib.sha256()
 def run(mode,data):
  r=subprocess.run([a.driver,mode],input=data,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;return r.stdout.splitlines()
 cases=[(tag,value,cap) for tag in [0,2] for value in range(-9999,10000) for cap in range(7)]
 rows=run('numeric',''.join(f'{t} {v} {c}\n' for t,v,c in cases));assert len(rows)==len(cases)
 for (tag,value,cap),row in zip(cases,rows):
  expected=str(value).encode('ascii');ok=len(expected)<=cap;out=(expected+b'\xa5'*(8-len(expected))) if ok else b'\xa5'*8
  result=f'{0 if ok else 3} {len(expected) if ok else 777} {out.hex()}';assert row==result,(tag,value,cap,row,result);digest.update((row+'\n').encode())
 wire=[(version,tag,value,0) for version in [1,2] for tag in [0,2] for value in range(-9999,10000)]
 wire += [(version,1,0,n) for version in [1,2] for n in range(257)]+[(2,3,b,0) for b in [0,1]]
 rows=run('wire_oracle',''.join(f'{v} {t} {n} {l}\n' for v,t,n,l in wire));assert len(rows)==len(wire)
 for (version,tag,value,length),row in zip(wire,rows):
  payload=bytes((i*37+11)%256 for i in range(length)) if tag==1 else bytes([value]) if tag==3 else value.to_bytes(4,'big',signed=True)
  octets=bytes([version,tag])+len(payload).to_bytes(2,'big')+payload
  oldstatus=0 if version==1 else 1;written=(length if tag==1 else len(str(value))) if version==1 else 777
  expected=f'{version} {tag} {value if tag in [0,2] else 0} {length if tag==1 else 0} {value if tag==3 else 0} {oldstatus} {written} {octets.hex()}'
  assert row==expected,(row,expected);digest.update((row+'\n').encode())
 print(json.dumps(dict(passed=True,numeric_cases=len(cases),integers=19999,tags=2,capacities=7,wire_cases=len(wire),text_lengths=257,versions=2,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
