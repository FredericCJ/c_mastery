"""Independent mathematical mask/checked-transform oracle; no assessor import."""
import argparse,hashlib,itertools,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 metrics=json.loads(subprocess.check_output([a.driver,'metrics'],text=True));lo,hi=metrics['int_min'],metrics['int_max'];rows=[];expected=[]
 for required,observed in itertools.product(range(18),repeat=2):
  rows.append(f'm {required} {observed}')
  expected.append(f'1 {int(all(not (required>>i)&1 or (observed>>i)&1 for i in range(4)))}' if required<16 and observed<16 else '0 9')
 values=sorted({lo,lo+1,-((-lo)//2)-1,-((-lo)//2),-1,0,1,hi//2,hi//2+1,hi-1,hi})
 for v in itertools.product(values,repeat=3):
  rows.append('t '+' '.join(map(str,v)));doubled=[2*x for x in v]
  expected.append('1 '+' '.join(map(str,doubled)) if all(lo<=x<=hi for x in doubled) else '0 91 92 93')
 proc=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True);assert proc.returncode==0,proc.stderr
 assert proc.stdout.splitlines()==expected
 trace=hashlib.sha256(('\n'.join(x+' => '+y for x,y in zip(rows,expected))+'\n').encode()).hexdigest()
 print(json.dumps(dict(passed=True,cases=len(rows),mask_cases=324,transform_cases=len(rows)-324,trace_sha256=trace,int_min=lo,int_max=hi)))
if __name__=='__main__':main()
