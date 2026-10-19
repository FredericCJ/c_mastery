"""Independent finite ownership histories under two zero-size policies."""
import argparse,hashlib,itertools,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();rows=[];expected=[]
 for policy in [0,1]:
  for length in range(6):
   for history in itertools.product('abcfzdw',repeat=length):
    values=[];calls=0;releases=0;status=0
    for op in history:
     if op in 'abc':
      n={'a':1,'b':2,'c':4}[op];values=(values+[0]*n)[:n];calls+=1;status=0
     elif op=='f':calls+=1;status=2
     elif op=='z':status=1
     elif op=='d':releases+=bool(values);values=[];status=0
     elif op=='w':
      if values:values[0]=23
      status=0
    rows.append(str(policy)+' '+(''.join(history) or '-'));expected.append(f'{status} {len(values)} {sum(values)} {calls} {releases} 0')
 proc=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True);assert proc.returncode==0,proc.stderr;assert proc.stdout.splitlines()==expected
 trace=hashlib.sha256(('\n'.join(x+' => '+y for x,y in zip(rows,expected))+'\n').encode()).hexdigest()
 print(json.dumps(dict(passed=True,histories=len(rows),maximum_length=5,zero_policies=2,trace_sha256=trace)))
if __name__=='__main__':main()
