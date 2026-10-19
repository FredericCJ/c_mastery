"""Exhaustive finite set-intersection contract, independent of C range predicates."""
import json,pathlib,subprocess,sys
prefix=json.loads(sys.argv[3]) if len(sys.argv)>3 else []
p=subprocess.run([*prefix,sys.argv[1]],capture_output=True,text=True,timeout=90)
pathlib.Path(sys.argv[2]).write_text(json.dumps(dict(command=[*prefix,sys.argv[1]],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr),indent=2)+'\n')
assert p.returncode==0,p.stderr
expected=[]
for length in range(9):
    for dst in range(length+2):
        for src in range(length+2):
            for n in range(length+2):
                for inplace in range(2):
                    before=list(range(8));after=before[:]
                    domain=set(range(length));source=set(range(src,src+n));destination=set(range(dst,dst+n))
                    valid=src<=length and dst<=length and source<=domain and destination<=domain
                    valid=valid and (n==0 or not(source&destination) or (inplace and dst==src))
                    if valid:
                        for i in range(n):after[dst+i]=before[src+i]^0x55
                    expected.append([length,dst,src,n,inplace,0 if valid else -1,*after])
actual=[list(map(int,line.split(','))) for line in p.stdout.splitlines()]
assert actual==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('count',len(actual),len(expected)))
print(json.dumps(dict(status='pass',range_cases=len(expected),limits='Finite arrays0..8 and exact contract partition only')))
