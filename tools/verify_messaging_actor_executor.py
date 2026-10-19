"""Independent message/incarnation state machine and owner accounting."""
import argparse
from collections import deque
import itertools
import subprocess


def model(word):
    queue=deque();generation=1;alive=True;closed=False;cancel=False
    calls=dead=drops=0;delivery=99;result=[]
    for op in word:
        if op in 'abf':
            status=4 if cancel else 3 if closed else 2 if len(queue)==2 else 0
            if status==0:queue.append((2 if op=='b' else 1,op=='f'))
            else:drops+=1
        elif op=='p':
            status=4 if cancel else 0 if queue else 3 if closed else 1
            if status==0:
                gen,fail=queue.popleft();drops+=1
                if gen!=generation:delivery=7;dead+=1
                elif not alive:delivery=3;dead+=1
                else:
                    calls+=1;delivery=10 if fail else 0
                    if fail:alive=False
        elif op=='r':
            status=6 if alive else 0
            if not alive:generation+=1;alive=True
        elif op=='c':closed=True;status=0
        else:cancel=True;status=0
        result.append(f'{status}:{delivery}:{generation}:{calls}:{dead}:{drops}:{int(alive)},')
    return ''.join(result)+f'D{drops+len(queue)}'


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    words=[''.join(w) for n in range(6) for w in itertools.product('abfprcx',repeat=n)]
    words+=['fprbpaap','aafpprbpx','aaaaacppppp','fpfprbpbp']
    result=subprocess.run([args.driver,'oracle'],input='\n'.join(words)+'\n',capture_output=True,text=True,timeout=90)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(words)
    for word,row in zip(words,actual):assert row==model(word),(word,row,model(word))
    print(f'Independent actor/executor model passed {len(words)} ownership/incarnation words; native schedule limits remain explicit')


if __name__=='__main__':main()
