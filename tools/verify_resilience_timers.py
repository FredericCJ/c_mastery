"""Sorted due-event model, independent of all three C timer representations."""
import argparse
import itertools
import json
from pathlib import Path
import subprocess

def model(word):
    wall=now=observed=sequence=0;live={};generations=[0]*16;identity=None;output=[]
    for op in word:
        status=0;events=[]
        if op in 'abc':
            if len(live)==16:status=2
            else:
                slot=next(i for i in range(16) if i not in live)
                generations[slot]+=1;identity=(slot,generations[slot])
                live[slot]=(wall+{'a':1,'b':9,'c':63}[op],sequence,generations[slot])
                sequence+=1;observed=wall
        elif op=='x':
            if identity is None or identity[0] not in live or live[identity[0]][2]!=identity[1]:status=6
            else:del live[identity[0]]
        elif op=='p':wall+=100
        else:
            wall+=1 if op=='t' else 100;now=observed=wall
            expired=sorted((due,seq,slot,gen) for slot,(due,seq,gen) in live.items() if due<=now)
            for due,seq,slot,gen in expired:
                events.append(f'{slot}:{gen}:{due}:{seq}:{observed-due},');del live[slot]
        output.append(f'{status},{now},{observed},{len(live)}['+''.join(events)+']; ')
    return ''.join(output)

def execute(executable,mode,cases,expected):
    result=subprocess.run([str(executable),mode],input='\n'.join(cases)+'\n',text=True,capture_output=True,timeout=90)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(mode,len(actual),len(expected))
    for i,(got,want) in enumerate(zip(actual,expected)):
        assert got==want,(mode,i,cases[i],got,want)
    return len(cases)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    cases=[];expected=[]
    for length in range(1,6):
        for chars in itertools.product('abcxtjp',repeat=length):
            word=''.join(chars);want=model(word)
            for kind in range(3):cases.append(f'{kind} {word}');expected.append(want)
    histories=execute(Path(args.driver),'oracle',cases,expected)
    cases=[];expected=[]
    for kind,first,second,cancel in itertools.product(range(3),range(1,17),range(1,17),range(17)):
        cases.append(f'{kind} {first} {second} {cancel}')
        due=[(second,1)]
        if cancel>first:due.append((first,0))
        expected.append(('6|' if cancel>first else '0|')+''.join(f'{time}:{slot},' for time,slot in sorted(due)))
    pairs=execute(Path(args.driver),'pairs',cases,expected)
    print(json.dumps({'histories':histories,'pairs':pairs,'implementations':['ordered-list','binary-heap','two-level-wheel'],
                      'model':'independent sorted absolute due events; finite bounds, not an all-input proof'}))
    return 0

if __name__=='__main__':raise SystemExit(main())
