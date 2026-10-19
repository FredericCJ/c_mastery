"""Independent phase/balance/intent model for the original two-step workflow."""
import argparse,hashlib,json,struct,subprocess
def run(driver):
 rows=[]
 for decline in range(2):
  for amount in [1,4,10]:
   steps=[(2,0,10-amount,20)]
   steps+=([(3,0,10-amount,20),(5,2,10,20)] if decline else [(4,1,10-amount,20+amount)])
   for step,(phase,result,left,right) in enumerate(steps):
    wire=bytes([1,phase,decline])+struct.pack('<i',amount)
    rows.append([decline,amount,step,phase,result,left,right,wire.hex()])
 p=subprocess.run([driver,'oracle'],text=True,capture_output=True,timeout=400);assert p.returncode==0,p.stdout+p.stderr
 actual=[]
 for line in p.stdout.splitlines():
  parts=line.split(',');actual.append([*[int(v) for v in parts[:-1]],parts[-1]])
 assert actual==rows,(actual,rows)
 return dict(passed=True,phase_rows=len(rows),workflows=6,trace_sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest(),limits=['Independent phase/balance/intent oracle; persistence-cut execution is separately observed by the C harness.','Three exclusive synthetic images; compensation is a separate effect, not rollback or a general distributed transaction.'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
