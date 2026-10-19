import argparse
import heapq
import itertools
import json
import random
import subprocess


def expected(c):
    m0,m1,d0,d1,deadline,notify,cancel,capacity,after,hedge,reset=c
    masks=[m0,m1];delays=[d0,d1];tokens=capacity;attempts=[None,None];events=[];serial=0
    winner=-1;reply_at=quiet_at=notification_at=0;considered=not hedge;quiet=False
    applied=replays=denied=dropped=dispatches=deliveries=acknowledged=resets=0;records=1
    notification=0;reset_done=False
    def plan(index,tick):
        nonlocal serial
        a=attempts[index]
        if a['reserved']==3:return
        raw=max(tick*1000+after,a['tcp_due'])
        if raw>=deadline*1000:return
        a['reserved']+=1;due=(raw+999)//1000
        serial+=1;heapq.heappush(events,(due,serial,index))
    def emit(index,tick):
        nonlocal tokens,dispatches,dropped,applied,replays,records,notification
        a=attempts[index];ordinal=a['sent'];a['sent']+=1;dispatches+=1;tokens-=1
        lost=bool(masks[index]&(1<<ordinal))
        if lost:dropped+=1
        else:
            if applied:replays+=1
            else:applied=1;notification=1
            records+=1
            if a['response'] is None:a['response']=tick+delays[index]
        if ordinal:a['rto']*=2
        a['tcp_due']=tick*1000+a['rto']
        plan(index,tick)
    def start(index,tick):
        attempts[index]={'sent':0,'reserved':0,'rto':1000,'tcp_due':tick*1000+1000,'response':None,'finished':False}
        emit(index,tick)
    start(0,0)
    if hedge:serial+=1;heapq.heappush(events,(2,serial,2))
    for tick in range(65):
        cancelled=bool(cancel and tick>=cancel);expired=tick>=deadline
        for index,a in enumerate(attempts):
            if a is None or a['finished']:continue
            if a['response']==tick or (a['response'] is None and tick==deadline+1):
                a['finished']=True
                if a['response'] is not None and winner<0 and not cancelled and not expired:
                    winner=index;reply_at=tick;considered=True
        while events and events[0][0]==tick:
            _,_,index=heapq.heappop(events)
            if index==2:
                considered=True
                if not cancelled and not expired and winner<0:
                    if tokens:start(1,tick)
                    else:denied+=1
            else:
                a=attempts[index]
                if a['finished'] or cancelled or expired or winner>=0:continue
                if not tokens:denied+=1
                else:emit(index,tick)
        if not quiet and considered and all(a is None or a['finished'] for a in attempts):quiet=True;quiet_at=tick
        if notification==1:notification=2
        if tick>=notify and notification==2:notification=3;deliveries+=1;notification_at=tick
        if notification==3:
            if not reset or reset_done:acknowledged+=1;notification=0
            elif quiet:reset_done=True;resets+=1;notification=1
        if quiet and not notification and tick>=notify:break
    started=sum(a is not None for a in attempts)
    return ','.join(map(str,[0,started,dispatches,dispatches-started,dropped,denied,applied,replays,deliveries,acknowledged,resets,reply_at,quiet_at,notification_at,tokens,winner,3*applied,records,acknowledged]))


def main():
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);args=p.parse_args()
    cases=[]
    for m0,m1,deadline in itertools.product(range(16),range(16),range(1,17)):
        cases.append((m0,m1,1+m0%8,1+m1%8,deadline,m0%8,m1%5,1+m0%8,1501 if m0%2 else 0,1,m1%2))
    randomizer=random.Random(75911)
    for _ in range(5000):cases.append((randomizer.randrange(16),randomizer.randrange(16),randomizer.randrange(1,25),randomizer.randrange(1,25),randomizer.randrange(1,17),randomizer.randrange(33),randomizer.randrange(17),randomizer.randrange(1,9),randomizer.choice([0,1501]),randomizer.randrange(2),randomizer.randrange(2)))
    result=subprocess.run([args.driver,'oracle'],input='\n'.join(' '.join(map(str,c)) for c in cases)+'\n',capture_output=True,text=True,timeout=120)
    assert result.returncode==0,result.stderr
    lines=result.stdout.splitlines();assert len(lines)==len(cases),(len(lines),len(cases))
    for index,(c,got) in enumerate(zip(cases,lines)):
        want=expected(c);assert got==want,(index,c,got,want)
    print(json.dumps({'structured_histories':4096,'mixed_histories':5000,'composition_cases':len(cases)}))


if __name__=='__main__':main()
