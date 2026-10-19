"""Independent logical effect/event/ack history model, not a translation of C rings."""
import argparse
import itertools
import json
import random
import subprocess


def expected(capacity,word):
    now=total=ack=seen=0;epoch=records=1;entries={};events=[];channel=[];answers=[]
    for letter in word:
        status=0
        if letter in 'ab':
            identity=1 if letter=='a' else 2;delta=identity
            old=entries.get(identity)
            if old and now-old[0]<2:records+=1
            elif seen&(1<<identity):status=6
            elif len(events)==4:status=2
            else:
                total+=delta;entries[identity]=(now,total);records+=1;seen|=1<<identity
                events.append([records,1])
        elif letter=='t':now+=1
        elif letter=='f':
            waiting=next((e for e in events if e[1]==1),None)
            if not waiting:status=3
            elif len(channel)==capacity:status=2
            else:waiting[1]=2;channel.append(waiting[0])
        elif letter=='d':
            if not channel:status=3
            else:
                sequence=channel.pop(0);next(e for e in events if e[0]==sequence)[1]=3
        elif letter=='k':
            if not events or events[0][1]!=3:status=6
            else:ack=events.pop(0)[0]
        elif letter=='r':
            channel=[]
            for e in events:e[1]=1
        elif letter=='e':
            if events:status=12
            else:epoch+=1;records+=1;entries={};seen=0
        else:raise ValueError(letter)
        pending=sum(e[1]==1 for e in events)
        answers.append(f'{status},{total},{epoch},{len(events)},{pending},{ack},{records},{seen}:'+''.join(f'{a}.{b}/' for a,b in events)+';')
    return ''.join(answers)


def main():
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);args=p.parse_args()
    cases=[];answers=[]
    for capacity in [1,2]:
        for length in range(1,6):
            for symbols in itertools.product('abtfdkre',repeat=length):
                word=''.join(symbols);cases.append(f'{capacity} {word}');answers.append(expected(capacity,word))
    structured=len(cases);randomizer=random.Random(75009)
    for _ in range(5000):
        capacity=randomizer.choice([1,2]);word=''.join(randomizer.choices('abtfdkre',weights=[4,4,2,4,4,4,2,1],k=32))
        cases.append(f'{capacity} {word}');answers.append(expected(capacity,word))
    result=subprocess.run([args.driver,'oracle'],input='\n'.join(cases)+'\n',capture_output=True,text=True,timeout=180)
    assert result.returncode==0,result.stdout[-1000:]+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(answers),(len(actual),len(answers))
    for index,(got,want) in enumerate(zip(actual,answers)):
        assert got==want,(index,cases[index],got,want)
    print(json.dumps({'structured_histories':structured,'mixed_histories':5000,'cases':len(cases),
                      'scope':'Atomic durable-effect/event and ACK history model; byte-tear/native-file tests are separate C checks. No exactly-once external consumer or infinite recovery proof.'}))


if __name__=='__main__':main()
