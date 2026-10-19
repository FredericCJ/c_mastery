"""Independent retained-observation recomputation and reserved-capacity event model."""
import argparse
import itertools
import json
from pathlib import Path
import random
import subprocess

def model(config,word):
    window,minimum,numerator,denominator=config
    health=[dict(phase=0,epoch=1,until=0,observations=[]) for _ in range(2)]
    slots=[None]*4;generations=[0]*4;used=[0,0];last=[None,None];now=0;closed=False;output=[]
    def complete(token,outcome):
        if token is None:return 6
        slot,gen=token;p=slots[slot]
        if p is None or generations[slot]!=gen:return 6
        tenant=slot//2;used[tenant]-=p['cost'];slots[slot]=None;h=health[tenant]
        if p['epoch']!=h['epoch'] or outcome<0:return 0
        def trip():h.update(phase=1,epoch=h['epoch']+1,until=now+4)
        if h['phase']==2:
            if outcome:h['phase']=0;h['observations']=[]
            else:trip()
        else:
            h['observations']=(h['observations']+[outcome==0])[-window:]
            count=len(h['observations']);failures=sum(h['observations'])
            if count>=minimum and failures*denominator>=count*numerator:trip()
        return 0
    for char in word:
        tenant=int(char.isupper());op=char.lower();h=health[tenant];status=0
        if op in 'ab':
            cost=1 if op=='a' else 2
            if closed:status=16
            elif h['phase']==1 and now<h['until']:status=11
            elif used[tenant]+cost>2:status=12
            elif h['phase'] and any(p is not None and p['epoch']==h['epoch'] for p in slots[2*tenant:2*tenant+2]):status=12
            else:
                candidates=[i for i in range(2*tenant,2*tenant+2) if slots[i] is None]
                if not candidates:status=2
                else:
                    i=candidates[0];generations[i]+=1;slots[i]=dict(cost=cost,epoch=h['epoch'],cancelled=False)
                    used[tenant]+=cost;last[tenant]=(i,generations[i])
                    if h['phase']==1:h['phase']=2
        elif op in 'fsn':
            candidates=[i for i in range(2*tenant,2*tenant+2) if slots[i] is not None]
            token=(candidates[0],generations[candidates[0]]) if candidates else last[tenant]
            status=complete(token,{'f':0,'s':1,'n':-1}[op])
        elif op=='d':status=complete(last[tenant],1)
        elif op=='c':
            token=last[tenant]
            if token is None or slots[token[0]] is None or generations[token[0]]!=token[1]:status=6
            else:slots[token[0]]['cancelled']=True
        elif op=='t':now+=4
        elif op=='x':
            closed=True
            for p in slots:
                if p is not None:p['cancelled']=True
        fields=[status,*used]
        for item in health:fields += [item['phase'],item['epoch'],len(item['observations']),sum(item['observations']),item['until']]
        output.append(':'.join(map(str,fields))+';')
    return ''.join(output)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    configs=[(4,4,1,2),(4,4,3,4),(8,3,2,3),(1,1,1,1)]
    words=[''.join(chars) for n in range(1,6) for chars in itertools.product('abAfsntx',repeat=n)]
    words+=[''.join('a'+outcome+'t' for outcome in chars) for n in range(1,8) for chars in itertools.product('fsn',repeat=n)]
    cases=[];expected=[]
    for config,word in itertools.product(configs,words):
        cases.append(' '.join(map(str,config))+' '+word);expected.append(model(config,word))
    structured=len(cases);rng=random.Random(75006)
    for _ in range(5000):
        config=rng.choice(configs);word=''.join(rng.choice('abABfsnFSNcdCDtx') for _ in range(rng.randrange(12,61)))
        cases.append(' '.join(map(str,config))+' '+word);expected.append(model(config,word))
    result=subprocess.run([str(Path(args.driver)),'oracle'],input='\n'.join(cases)+'\n',text=True,capture_output=True,timeout=180)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(got,want) in enumerate(zip(actual,expected)):assert got==want,(i,cases[i],got,want)
    print(json.dumps({'structured_histories':structured,'mixed_tenant_histories':5000,'configurations':configs,
                      'expected':'Retained Python lists recomputed for every result, separate per-tenant reserved cost, live-generation permit ledger.',
                      'limits':'Bounded sequential states under external serialization; no physical backend response or arbitrary schedule theorem.'}));return 0

if __name__=='__main__':raise SystemExit(main())
