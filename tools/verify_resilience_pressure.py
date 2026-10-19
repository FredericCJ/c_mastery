"""List/ownership and explicit receiver-credit models for the pressure successor."""
import argparse
import itertools
import json
from pathlib import Path
import subprocess

def execute(executable,mode,cases,expected):
    result=subprocess.run([str(executable),mode],input='\n'.join(cases)+'\n',text=True,capture_output=True,timeout=180)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(mode,len(actual),len(expected))
    for i,(got,want) in enumerate(zip(actual,expected)):
        assert got==want,(mode,i,cases[i],got,want)
    return len(cases)

def policy_model(policy,word):
    queue=[];closed=expired=cancel=False;dropped=0;out=[]
    for op in word:
        status=0;observed=99
        if op in 'ab':
            value=1 if op=='a' else 9
            if cancel:status=4;dropped+=1
            elif expired:status=5;dropped+=1
            elif closed:status=16;dropped+=1
            elif len(queue)<4:queue.append(value)
            elif policy==0:status=8;dropped+=1 # caller disposes the retained public-test value
            elif policy==1:status=2;dropped+=1
            elif policy==2:status=9;dropped+=1
            elif policy==3:queue=queue[1:]+[value];dropped+=1
            else:status=10;queue[-1]=max(queue[-1],value);dropped+=1
        elif op=='p':
            if queue:observed=queue.pop(0);dropped+=1
            else:status=16 if closed else 3
        elif op=='t':expired=True
        elif op=='x':cancel=True
        elif op=='c':closed=True
        out.append(f'{status}:{observed}:{dropped}['+''.join(f'{v},' for v in queue)+']; ')
    return ''.join(out)

def pump_model(capacity,word):
    slots=[None]*4;generation=[0]*4;queue=[];channel=[];leased=[]
    producers=[None,None];tickets=[None,None];serial=[0,0]
    next_producer=0;closed=channel_closed=False;now=0;consumed=returned=dropped=sequence=0;last=None;output=[]
    def progress():
        nonlocal dropped,next_producer,consumed
        start=next_producer
        for j in range(2):
            k=(start+j)%2;p=producers[k]
            if not p or p['status']!=8:continue
            if now>=2:p['status']=5;dropped+=1
            elif closed:p['status']=16;dropped+=1
            elif any(s is None for s in slots):
                slot=slots.index(None);generation[slot]+=1
                slots[slot]=dict(value=p['value'],state='queued');queue.append(slot)
                consumed+=1;next_producer=(k+1)%2;p['status']=0
    def poll(k):
        p=producers[k]
        if p is None or tickets[k]!=p['serial']:return 6,99
        if p['status']==8:return 8,99
        result=p['status'];producers[k]=None;return 0,result
    for op in word:
        status=0;observed=99
        if op in 'ab':
            k=0 if op=='a' else 1;sequence+=1
            if closed:status=16;dropped+=1
            elif producers[k] is not None:status=12;dropped+=1
            else:
                serial[k]+=1;tickets[k]=serial[k]
                producers[k]=dict(serial=serial[k],status=8,value=sequence)
                progress();status=producers[k]['status']
                if status!=8:assert poll(k)==(0,status)
        elif op=='f':
            if not queue:status=3
            elif len(channel)==capacity:status=2
            elif channel_closed:status=16
            else:
                slot=queue.pop(0);channel.append(slot);slots[slot]['state']='transport'
            if closed and not queue:channel_closed=True
        elif op=='r':
            if channel:
                slot=channel.pop(0);slots[slot]['state']='leased';leased.append((slot,generation[slot]));observed=slots[slot]['value']
            else:status=16 if channel_closed else 3
        elif op=='x':
            if leased:last=leased.pop(0)
            if last is None:status=6
            else:
                slot,gen=last
                if slots[slot] is None or slots[slot]['state']!='leased' or generation[slot]!=gen:status=6
                else:slots[slot]=None;returned+=1;dropped+=1;progress()
        elif op=='c':
            closed=True;progress()
            if not queue:channel_closed=True
        elif op=='t':now=2;progress()
        elif op=='p':
            a,b=poll(0),poll(1);status=a[0]*100+b[0];observed=a[1]*100+b[1]
        counts={state:sum(s is not None and s['state']==state for s in slots) for state in ['queued','transport','leased']}
        pending=sum(p is not None and p['status']==8 for p in producers)
        terminal=sum(p is not None and p['status']!=8 for p in producers)
        credits=sum(s is None for s in slots)
        output.append(f"{status}:{observed}:{counts['queued']}:{counts['transport']}:{counts['leased']}:{pending}:{terminal}:{credits}:{consumed}:{returned}:{dropped};")
    return ''.join(output)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args();exe=Path(args.driver)
    cases=[];expected=[]
    for n in range(1,6):
        for letters in itertools.product('abptxc',repeat=n):
            word=''.join(letters)
            for policy in range(5):cases.append(f'{policy} {word}');expected.append(policy_model(policy,word))
    policies=execute(exe,'policy',cases,expected)
    workload='aaabababpppp';workloads=[]
    for policy in range(5):
        observation=policy_model(policy,workload)
        execute(exe,'policy',[f'{policy} {workload}'],[observation])
        records=observation.split('; ')[:-1];states=[int(row.split(':',1)[0]) for row in records[:8]]
        workloads.append({'policy':policy,'offered':8,'accepted':states.count(0),
                          'rejected':states.count(2),'dropped':states.count(9),'coalesced':states.count(10),
                          'pending_caller_disposed':states.count(8),'displaced':4 if policy==3 else 0,
                          'peak_queued':4,'maximum_adapter_pending':0,'schedule':workload})
    cases=[];expected=[]
    words=[''.join(chars) for n in range(1,6) for chars in itertools.product('abfrxcpt',repeat=n)]
    words+=['aaaababfrxpxfrx','aaaababtpcfrxfrxfrxfrx','aaaabfrfrfrfrxxxxxx','aaaabcfrrxfrxfrxfrx']
    for word in words:
        for capacity in [1,4]:cases.append(f'{capacity} {word}');expected.append(pump_model(capacity,word))
    pumps=execute(exe,'pump',cases,expected)
    print(json.dumps({'policy_histories':policies,'pump_histories':pumps,'policy_workloads':workloads,
                      'models':'Independent lists, owner counts and per-slot returned-credit identities; bounded histories plus explicit full-capacity traces.',
                      'limits':'Native two-waiter test is separate; finite traces do not prove all schedules or weak-memory behavior.'}));return 0

if __name__=='__main__':raise SystemExit(main())
