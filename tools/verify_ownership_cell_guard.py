"""Independent small guard-admission and callback outcome table."""
import argparse
import hashlib
import itertools
import json
import subprocess
def run(driver):
    inputs=[];answers=[]
    for readers,writer,fail,close in itertools.product(range(7),range(2),range(2),range(2)):
        n=min(readers,1 if writer else 4)
        admission=0 if readers==n else 4 if writer else 2
        calls=int(n==0);status=4 if n else 6 if close else 1 if fail else 0
        answer=[admission,n,status,9 if status==0 else 71,calls,0,int(close and calls)]
        inputs.append(f'{readers} {writer} {fail} {close}');answers.append(' '.join(map(str,answer)))
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,cases=len(inputs),reader_limits=[0,6],writer_modes=2,callback_failure_modes=2,callback_close_modes=2,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Sequential admission oracle only; actual pthread and TSan observations are separate and not an exhaustive interleaving proof.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
