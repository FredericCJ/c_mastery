"""Independent list partition and exact destructor-identity model."""
import argparse,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args();commands=[];expected=[]
for n in range(9):
 for mask in range(1<<n):
  for fail in range(n+1):
   commands.append(f'{n} {mask} {fail}')
   original=list(range(1,n+1));kept=[i for i in original if not mask>>(i-1)&1];removed=[i for i in original if mask>>(i-1)&1]
   prefix=[2,fail,n,n]+original if fail else [0,n,n,len(kept)]+kept+removed
   suffix=[n,0,(1<<n)-1] if fail else [len(kept),mask,(1<<n)-1]
   expected.append(' '.join(map(str,prefix))+' | '+' '.join(map(str,suffix)))
if a.driver:
 result=subprocess.run([a.driver],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=60);assert result.returncode==0,result.stderr
 actual=result.stdout.splitlines();assert len(actual)==len(expected)
 for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(commands[i],x,y)
print(json.dumps(dict(passed=True,cases=len(expected),scope='Every keep mask and every predicate-failure ordinal for length0..8; finite exact ownership/list model, not all C callbacks.')))
