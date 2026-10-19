"""Independent finite cancellation/completion transition oracle, not schedule proof."""
import argparse
import itertools
import subprocess


def expected(word):
    cancelled=False;claimed=False;calls=0;drops=0;status=5;results=[]
    for letter in word:
        if letter=='c':
            results.append(3 if claimed else 0);cancelled=True
        else:
            drops+=1
            if claimed:results.append(6)
            else:
                claimed=True;calls+=1;status=4 if cancelled else 0 if letter=='a' else 10
                results.append(0)
    return ''.join(str(s)+',' for s in results)+f'|{calls},{drops},1,{status}'


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    words=[''.join(w) for n in range(1,8) for w in itertools.product('abc',repeat=n)]
    run=subprocess.run([args.driver,'oracle'],input='\n'.join(words)+'\n',text=True,capture_output=True,timeout=60,check=True)
    observed=run.stdout.splitlines();wanted=[expected(w) for w in words]
    if observed!=wanted:
        for word,a,b in zip(words,observed,wanted):
            if a!=b:raise AssertionError((word,a,b))
        raise AssertionError(('count',len(observed),len(wanted)))
    print(f'Independent completion winner oracle passed {len(words)} words')


if __name__=='__main__':main()
