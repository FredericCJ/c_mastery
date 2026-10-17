"""Independent finite deque and supervisor fixture models; optional compiled differential driver."""
import argparse
from collections import deque
import itertools
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def deque_trace(word):
    q=deque();serial=0;out=[]
    for op in word:
        if op=='+':
            serial+=1
            if len(q)==8:out.append('s2')
            else:q.append(serial);out.append('s0')
        elif not q:out.append('s1')
        else:out.append('v'+str(q.pop() if op=='p' else q.popleft()))
    return ','.join(out)+(',' if out else '')
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');args=p.parse_args()
    data=json.loads((ROOT/'fixtures/messaging/cases.json').read_text(encoding='utf-8'))
    for row in data['cases']:
        if row['kind']=='stealing':actual=deque_trace(row['input'])
        elif row['kind']=='supervision':
            strategy,failed=row['input'];selected=[failed] if strategy=='one' else list(range(0 if strategy=='all' else failed,4))
            actual=[-i-1 for i in selected[::-1]]+[i+1 for i in selected]
        else:continue
        if actual!=row['expected']:raise AssertionError(row['id'])
    words=[''.join(w) for n in range(7) for w in itertools.product('+ps',repeat=n)]
    words+=['+'*10+'s'*10,'+'*8+'p'*4+'+'*5+'s'*9]
    if args.driver:
        run=subprocess.run([args.driver],input='\n'.join(words)+'\n',text=True,capture_output=True,check=True)
        observed=run.stdout.splitlines();expected=[deque_trace(w) for w in words]
        if observed!=expected:raise AssertionError('compiled deque disagrees with independent Python deque model')
    print(json.dumps(dict(fixtures=len(data['cases']),differential_cases=len(words) if args.driver else 0,scope='Sequential histories and bounded supervisor selection, not all concurrent schedules')))
if __name__=='__main__':main()
