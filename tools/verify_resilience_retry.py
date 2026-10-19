"""Independent bounded arithmetic/event model for the actual retry transport adapter."""
import argparse
import collections
import itertools
import json
from pathlib import Path
import subprocess

MAX=(1<<64)-1
WORD=(1<<32)-1

def draw(state,span):
    # Arithmetic over residues; unlike C this calculation cannot overflow accidentally.
    for used in range(1,65):
        state=(5*state+1)%(1<<32)
        value=state^(state>>16)
        value=(value*0x9e3779b1)%(1<<32)
        value^=value>>13
        if value>=((1<<32)%span):return 0,value%span,state,used
    return 8,99,state,64

def execute(executable,mode,cases,expected):
    result=subprocess.run([str(executable),mode],input='\n'.join(cases)+'\n',text=True,capture_output=True,timeout=120)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(mode,len(actual),len(expected))
    for i,(got,want) in enumerate(zip(actual,expected)):
        assert got==want,(mode,i,cases[i],got,want)
    return len(cases)

def random_cases():
    cases=[];expected=[]
    for seed,span in itertools.product([0,1,2,17,65535,65536,0xdeadbeef,WORD],
                                       list(range(1,34))+[65535,65536,65537,1<<31,(1<<31)+1,WORD]):
        state=seed;total=0;out=[]
        for _ in range(65):
            status,value,state,used=draw(state,span);total+=used
            out.append(f'{status}:{value}:{state}:{total},')
        cases.append(f'{seed} {span} 65');expected.append(''.join(out))
    return cases,expected

def plan_cases():
    cases=[];expected=[]
    for seed,policy,attempt,now,after,budget in itertools.product(
            [0,1,17,WORD],range(3),range(4),[0,999,1000,1001,9999],
            [0,1,9,9000,MAX],[0,1000,1001,1008,10000]):
        state=seed;used=0;status=0;due=99;pending=0;committed=attempt
        if now>=budget:status=5
        elif attempt==3:status=2
        elif after>MAX-now:status=7
        else:
            window=min(8,2<<attempt);value=window
            if policy!=1:
                lower=window//2 if policy==2 else 0
                status,value,state,used=draw(state,window-lower+1);value+=lower
            if not status:
                candidate=max(now+value,now+after,1000)
                if candidate>=budget:status=5
                else:due=candidate;pending=1;committed+=1
        cases.append(' '.join(map(str,[seed,policy,attempt,now,after,budget])))
        expected.append(f'{status} {due} {committed} {pending} {state} {used}')
    return cases,expected

def history_cases():
    cases=[];expected=[]
    for budget,first,second,third,fourth in itertools.product(range(21),range(4),range(6),range(10),range(2)):
        attempts=now=0;out=99;records=[];draws=[first,second,third,fourth]
        for value in draws:
            if now>=budget:status=5
            elif attempts==3:status=2
            elif value>min(8,2<<attempts):status=1
            elif now+value>=budget:status=5
            else:status=0;attempts+=1;now+=value;out=now
            records.append(f'{status}:{attempts}:{out}:{now},')
        cases.append(' '.join(map(str,[budget,*draws])));expected.append(''.join(records))
    return cases,expected

def population(policy,correlated,early,cutoff):
    # Each event is ordered by absolute time then client identity, as the harness specifies.
    clients=[dict(state=0 if correlated else i,attempt=0,rto=1000,due=1000,
                  next=0 if early else 1000,stage=1) for i in range(32)]
    tokens=96;sent=32;delivered=0;events=[];records=[];completion=[]
    while any(c['stage'] for c in clients):
        now,i=min((c['next'],i) for i,c in enumerate(clients) if c['stage'])
        c=clients[i]
        if c['stage']==1:
            window=min(8,2<<c['attempt']);value=window
            if policy!=1:
                lower=window//2 if policy==2 else 0
                status,value,c['state'],_=draw(c['state'],window-lower+1)
                assert status==0;value+=lower
            c['next']=max(now+value,c['due']);c['attempt']+=1;c['stage']=2
        else:
            status=0 if now>=cutoff else 8
            tokens-=1;sent+=1;events.append(now);records.append(f'{i}:{now}:{status},')
            c['rto']*=2;c['due']=now+c['rto']
            if status==0:delivered+=1;completion.append(now);c['stage']=0
            elif c['attempt']==3:c['stage']=0
            else:c['stage']=1;c['next']=now if early else c['due']
    line=''.join(records)+f'S{sent}:D{delivered}:T{tokens}:A32:F32'
    return line,dict(policy=policy,correlated=bool(correlated),plan_before_rto=bool(early),
                     outage_until=cutoff,transmissions=sent,deliveries=delivered,
                     peak_retries_per_tick=max(collections.Counter(events).values()),
                     first_completion=min(completion) if completion else None,
                     last_completion=max(completion) if completion else None)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    exe=Path(args.driver);summary={}
    for mode,factory in [('random',random_cases),('plan',plan_cases),('history',history_cases)]:
        cases,expected=factory();summary[mode]=execute(exe,mode,cases,expected)
    cases=[];expected=[];experiments=[]
    for policy,correlated,early,cutoff in itertools.product(range(3),range(2),range(2),[1,1001,3005,7010,8000]):
        cases.append(f'{policy} {correlated} {early} {cutoff}')
        result,observation=population(policy,correlated,early,cutoff)
        expected.append(result);experiments.append(observation)
    summary['population']=execute(exe,'population',cases,expected)
    # Explicit counterexamples to unconditional claims that jitter spreads a population.
    assert all(r['peak_retries_per_tick']==32 for r in experiments if r['correlated'] or r['plan_before_rto'])
    summary['experiments']=experiments
    summary['limits']='Deterministic event/loss experiments, no entropy, fairness, all-history proof or deployment latency claim.'
    print(json.dumps(summary));return 0

if __name__=='__main__':raise SystemExit(main())
