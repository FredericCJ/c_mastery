"""Independent finite buffered-channel model; no reference implementation import."""
import argparse
from collections import deque
import itertools
import subprocess


def expected(word):
    queues=[deque(),deque()];closed=[False,False];stop=False;start=0;which=99;value=101;rows=[]
    for index,op in enumerate(word):
        if op in '01':
            k=int(op)
            status=4 if stop else 3 if closed[k] else 2 if len(queues[k])==2 else 0
            if status==0:queues[k].append((10 if k==0 else 20)+index)
        elif op in '23':
            mask=3 if op=='2' else 1;status=4 if stop else 1
            if not stop:
                for k in (start,1-start):
                    if mask&(1<<k) and queues[k]:
                        which=k;value=queues[k].popleft();start=1-k;status=0;break
                if status==1 and all(closed[k] for k in range(2) if mask&(1<<k)):status=3
        elif op in '45':closed[int(op)-4]=True;status=0
        else:stop=True;status=0
        rows.append(f'{status},{which},{value};')
    return ''.join(rows)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    words=[''.join(x) for n in range(6) for x in itertools.product('0123456',repeat=n)]
    result=subprocess.run([args.driver],input='\n'.join(words)+'\n',capture_output=True,text=True,timeout=60)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(words)
    for word,row in zip(words,actual):assert row==expected(word),(word,row,expected(word))
    print(f'Independent registration model passed {len(words)} words through length5; threaded progress not inferred')


if __name__=='__main__':main()
