"""Independent queue/ownership-event model; no C representation or ring indices reused."""
import argparse
import hashlib
import itertools
import json
import subprocess
def model(word):
    queue=[];removed=0;steps=0;drops=0;trace=0;rows=[]
    def destroy(value):
        nonlocal drops,trace
        drops+=1
        if value is not None:trace=(trace*257+value)%(1<<64)
    for op in word:
        status=0;result=99
        if op=='P':
            if not queue:status=3
            else:result=queue.pop(0);removed+=1;destroy(result)
        elif op=='C':
            for value in queue:destroy(value);removed+=1
            queue=[]
        elif len(queue)==4:status=2
        elif op=='F':steps+=1;destroy(None);status=4
        elif op=='G':steps+=2;destroy(1);status=4
        else:steps+=2;queue.append(2 if op=='B' else 1)
        rows.append([status,removed%4,len(queue),len(queue),steps,drops,result,trace])
    return rows
def run(driver):
    words=[''.join(t) for n in range(6) for t in itertools.product('ABFGPC',repeat=n)]
    words += ['ABABPAPBPAPBPAPBPAPBPC','AAAAFFFFGGGGPPPPCCCC','ABPG'*8,'GFPABC'*5]
    p=subprocess.run([driver,'histories'],input='\n'.join(words)+'\n',text=True,capture_output=True,timeout=90)
    assert p.returncode==0,(p.returncode,p.stderr)
    observed=[json.loads(line) for line in p.stdout.splitlines()]
    expected=[model(w) for w in words for _ in range(3)]
    assert observed==expected,next(((words[i//3],i%3,a,b) for i,(a,b) in enumerate(zip(observed,expected)) if a!=b),('line count',len(observed),len(expected)))
    return dict(passed=True,histories=len(words),implementations=3,transitions=sum(map(len,words))*3,trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
