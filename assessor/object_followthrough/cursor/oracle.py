"""Integer interval model, independent of the tested C predicates and pointer implementation."""
import json,pathlib,subprocess,sys
prefix=json.loads(sys.argv[3]) if len(sys.argv)>3 else []
p=subprocess.run([*prefix,sys.argv[1]],capture_output=True,text=True,timeout=60)
pathlib.Path(sys.argv[2]).write_text(json.dumps(dict(command=[*prefix,sys.argv[1]],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr),indent=2)+'\n')
assert p.returncode==0,p.stderr
expected=[]
for n in range(9):
    for position in range(n+1):
        remaining=list(range(position,n))
        for count in range(11):
            ok=int(len(remaining)>=count)
            expected.append(['take',n,position,count,ok,position+count if ok else position,position if ok else 8])
            previous=list(range(position));ok=int(len(previous)>=count)
            expected.append(['reverse',n,position,count,ok,position-count if ok else position,position-count if ok else 8])
        for offset in range(11):
            for count in range(11):
                # Enumerate possible half-open subintervals, including the final empty one.
                legal={(start,end) for start in range(position,n+1) for end in range(start,n+1)}
                ok=int((position+offset,position+offset+count) in legal)
                expected.append(['sub',n,position,offset,count,ok,position+offset if ok else 0,count if ok else n,0 if ok else position])
actual=[[q[0],*map(int,q[1:])] for q in (line.split(',') for line in p.stdout.splitlines())]
assert actual==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('count',len(actual),len(expected)))
print(json.dumps(dict(status='pass',interval_cases=len(expected),limits='Finite actual arrays0..8; no invented huge array/pointer or universal proof')))
