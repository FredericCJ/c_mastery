"""Independent list/prefix model for the original three-store protocol."""
import argparse
import json
import random
import subprocess


def cases():
    commands=[];expected=[]
    source=[];inbox=[];ack=0
    def step(op,*args):
        nonlocal source,inbox,ack
        r=0;reply=0
        if op=='reset':source=[];inbox=[];ack=0
        elif op=='append':
            kind,key,value=args
            if kind not in (1,2) or not 0<=key<=31:r=1
            elif len(source)==16:r=2
            else:source.append((key,kind,value))
        elif op=='receive':
            uuid,schema,position,key,kind,value=args;reply=99
            if not 1<=position<=16 or key>31 or kind not in (1,2):r=1
            elif (uuid,schema)!=(19,7):r=7
            elif position<=len(inbox):
                if inbox[position-1]!=(key,kind,value):r=103
                else:r=102;reply=sum(e[2] for e in inbox[:position])
            elif position!=len(inbox)+1:r=101
            else:inbox.append((key,kind,value));reply=sum(e[2] for e in inbox)
        elif op=='ack':
            uuid,position,reply=args
            if ack==len(source):r=100
            elif (uuid,position,reply)!=(19,ack+1,sum(e[2] for e in source[:ack+1])):r=103
            else:ack+=1
        elif op!='reopen':raise ValueError(op)
        commands.append(' '.join(map(str,(op,*args))))
        expected.append([r,len(source),ack,len(inbox),sum(e[2] for e in inbox),reply])
    rng=random.Random(7602)
    for history in range(64):
        step('reset')
        count=history%17
        for i in range(count):step('append',1+i%2,rng.randrange(32),rng.choice([-(1<<31),(1<<31)-1,-7,0,12]))
        step('append',0,1,0)
        if count==16:step('append',1,0,0)
        for i in range(200):
            choice=rng.randrange(6)
            if choice<3:
                pos=rng.choice([0,1,16,17,len(inbox)+1,rng.randrange(1,17)])
                event=source[pos-1] if 1<=pos<=len(source) else (0,1,0)
                key,kind,value=event
                if choice==1:value=0 if value else 1
                step('receive',20 if i%17==0 else 19,8 if i%23==0 else 7,pos,key,kind,value)
            elif choice==3:
                pos=ack+1;total=sum(e[2] for e in source[:pos])
                step('ack',20 if i%11==0 else 19,pos+(i%13==0),total+(i%7==0))
            elif choice==4:step('reopen',rng.randrange(3))
            else:
                pos=1+rng.randrange(16)
                step('receive',19,7,pos,32,1,0)
    return commands,expected


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);args=ap.parse_args()
    commands,expected=cases()
    p=subprocess.run([args.driver,'oracle'],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=300)
    if p.returncode:raise SystemExit(p.stderr or p.stdout)
    actual=[list(map(int,line.split())) for line in p.stdout.splitlines()]
    if len(actual)!=len(expected):raise SystemExit(f'row count {len(actual)} != {len(expected)}')
    for i,(a,e) in enumerate(zip(actual,expected)):
        if a!=e:raise SystemExit(f'row {i} {commands[i]}: actual {a} expected {e}')
    print(json.dumps({'passed':True,'histories':64,'transition_rows':len(expected),'oracle':'Python integer/list model; no C reference reuse','limits':'Finite serialized API histories; no arbitrary network Byzantine or persistence-subset proof.'}))


if __name__=='__main__':main()
