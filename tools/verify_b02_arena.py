"""Independent mathematical offset admission and nested-scope stack model."""
import argparse,hashlib,itertools,json,subprocess

def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 cases=[k for n in range(7) for k in itertools.product(range(5),repeat=n)]
 data=''.join(str(len(k))+' '+(''.join(map(str,k)) or '-')+'\n' for k in cases)
 run=subprocess.run([a.driver,'oracle'],input=data,text=True,capture_output=True,timeout=90);assert run.returncode==0,run.stderr
 lines=run.stdout.splitlines();assert len(lines)==len(cases);digest=hashlib.sha256();transitions=0
 for ops,line in zip(cases,lines):
  cursor=count=0;next_id=1;stack=[];expected=[len(ops)]
  for op in ops:
   rc=0;transitions+=1
   if op==0:
    if len(stack)==4 or next_id>15:rc=3
    else:stack.append((cursor,count,next_id));next_id+=1
   elif op==1:
    cursor=((cursor+1)//2)*2+1;count+=1
   elif op==4:
    if not stack:rc=5
    elif len(stack)==1:rc=1
    else:stack.pop()
   elif not stack or op==3 and len(stack)!=1:rc=5
   else:cursor,count,_=stack.pop()
   expected.extend([rc,cursor,len(stack),next_id,count])
  assert list(map(int,line.split()))==expected,(ops,line,expected)
  digest.update((' '.join(map(str,expected))+'\n').encode())
 maximum=int(subprocess.run([a.driver,'limits'],text=True,capture_output=True,check=True).stdout);assert maximum in [2**32-1,2**64-1]
 extents=[(c,n,a) for c in range(65) for n in [*range(68),maximum] for a in [0,1,2,3,4,8,16]]
 run=subprocess.run([a.driver,'extents'],input=''.join(f'{c} {n} {a}\n' for c,n,a in extents),text=True,capture_output=True,timeout=90);assert run.returncode==0,run.stderr
 rows=run.stdout.splitlines();assert len(rows)==len(extents)
 for (c,n,a),row in zip(extents,rows):
  if a not in [1,2,4,8]:expected=[1,77,c]
  else:
   offset=((c+a-1)//a)*a
   expected=[0,offset,offset+n] if offset+n<=64 else [3,77,c]
  assert list(map(int,row.split()))==expected,(c,n,a,row,expected)
 print(json.dumps(dict(passed=True,histories=len(cases),transitions=transitions,extent_cases=len(extents),size_max=maximum,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
