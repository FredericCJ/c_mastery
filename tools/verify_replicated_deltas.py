"""Independent tagged sets and three-replica causal-interval/restart model."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import random
import subprocess
def empty():return dict(adds=[set() for _ in range(4)],removed=[set() for _ in range(4)],next=[0]*3)
def join(a,b):return dict(adds=[x|y for x,y in zip(a['adds'],b['adds'])],removed=[x|y for x,y in zip(a['removed'],b['removed'])],next=[max(x,y) for x,y in zip(a['next'],b['next'])])
def below(a,b):return join(a,b)==b
def node():return dict(epoch=1,sequence=0,state=empty(),history=[],acked={},sent={},received={},past={0:empty()})
def publish(n,d):
    n['history'].append((n['sequence'],copy.deepcopy(d)));n['history']=n['history'][-4:]
    n['state']=join(n['state'],d);n['sequence']+=1;n['past'][n['sequence']]=copy.deepcopy(n['state'])
def encode(n,own):
    b=bytearray(96);b[:6]=b'RPD1\x01'+bytes([own]);b[8:16]=(9).to_bytes(8,'big');b[16:24]=n['epoch'].to_bytes(8,'big');b[24:32]=n['sequence'].to_bytes(8,'big')
    s=n['state']
    for half,base in enumerate([32,56]):
        for field,offset in [('adds',0),('removed',8)]:
            for v in range(4):
                mask=sum(1<<(q+8*(w%2)) for w,q in s[field][v] if (w<2)==(half==0))
                b[base+offset+2*v:base+offset+2*v+2]=mask.to_bytes(2,'big')
        b[base+16]=s['next'][0 if half==0 else 2];b[base+17]=s['next'][1] if half==0 else 0
    return (b+hashlib.sha256(b).digest()).hex()
def verify(driver):
    rng=random.Random(7605);commands=[];want=[];statuses={};causal_checks=0;histories=32
    for history in range(histories):
        nodes=[node() for _ in range(3)];packets={};acks={}
        def emit(op,a,b,c,status,fields=None):
            commands.append(f'{op} {a} {b} {c}\n');want.append(' '.join(map(str,[status,*(fields or [0]*5)]))+' '+' '.join(encode(n,i) for i,n in enumerate(nodes)))
            statuses[str(status)]=statuses.get(str(status),0)+1
        emit('I',0,0,0,0)
        for step in range(96):
            op=rng.choice('LLLLSSSSDDDKKO');a=rng.randrange(3);b=c=0;status=0;fields=None;n=nodes[a]
            if op=='D' and not packets:op='S'
            if op=='K' and not acks:op='S'
            if op=='L':
                b=rng.randrange(2);c=rng.randrange(4);d=empty()
                if b:
                    if n['state']['next'][a]==8:status=2
                    else:
                        dot=(a,n['state']['next'][a]);d['adds'][c].add(dot);d['next'][a]=dot[1]+1
                else:d['removed'][c]=n['state']['adds'][c].copy();d['next']=n['state']['next'][:]
                if not status:publish(n,d)
            elif op=='O':n['epoch']+=1;n['history']=[];n['acked']={};n['sent']={};n['received']={}
            elif op=='S':
                b=rng.choice([i for i in range(3) if i!=a]);c=rng.randrange(12);epoch=nodes[b]['epoch'];acked=n['acked'].get(b,(0,0))
                if acked==(epoch,n['sequence']):status=11
                else:
                    full=acked[0]!=epoch or not n['history'] or acked[1]<n['history'][0][0];start=0 if full else acked[1];body=copy.deepcopy(n['state']) if full else empty()
                    if not full:
                        for sequence,d in n['history']:
                            if sequence>=start:body=join(body,d)
                    packets[c]=dict(sender=a,target=b,se=n['epoch'],te=epoch,start=start,end=n['sequence'],full=int(full),body=body,base=copy.deepcopy(n['past'][start]))
                    n['sent'][b]=(epoch,n['sequence']);fields=[int(full),start,n['sequence'],n['epoch'],epoch]
            elif op=='D':
                b=rng.choice(list(packets));p=packets[b];a=p['target'];n=nodes[a];seen=n['received'].get(p['sender'],(0,0))
                if p['te']!=n['epoch'] or p['se']<seen[0]:status=9
                elif not p['full'] and (p['se']!=seen[0] or p['start']>seen[1]):status=10
                else:
                    # Reachable receiver state must subsume the sender prefix
                    # used as the interval base, independently of integer ACKs.
                    assert below(p['base'],n['state']);causal_checks+=1
                    if join(n['state'],p['body'])!=n['state']:publish(n,p['body'])
                    n['received'][p['sender']]=(p['se'],max(p['end'],seen[1] if seen[0]==p['se'] else 0))
                    ack=(a,p['sender'],n['epoch'],p['se'],p['end']);acks[b]=ack;fields=list(ack)
            elif op=='K':
                b=rng.choice(list(acks));peer,a,pe,own_epoch,end=acks[b];n=nodes[a];epoch=nodes[peer]['epoch'];sent=n['sent'].get(peer,(0,0))
                if own_epoch!=n['epoch'] or pe!=epoch or sent[0]!=epoch:status=9
                elif end>n['sequence'] or end>sent[1]:status=1
                else:
                    prior=n['acked'].get(peer,(0,0));n['acked'][peer]=(epoch,max(end,prior[1] if prior[0]==epoch else 0))
            emit(op,a,b,c,status,fields)
    run=subprocess.run([str(driver),'oracle'],input=''.join(commands),text=True,capture_output=True,timeout=180)
    assert run.returncode==0,run.stderr
    got=run.stdout.splitlines();assert len(got)==len(want),(len(got),len(want))
    for i,(a,b) in enumerate(zip(got,want)):assert a==b,(i,commands[i],a,b)
    return dict(passed=True,histories=histories,transition_rows=len(want),snapshot_comparisons=3*len(want),causal_base_checks=causal_checks,status_counts=statuses,
                output_sha256=hashlib.sha256(run.stdout.encode()).hexdigest(),
                limits=['Independent causal-base checks cover generated histories under identified noncorrupt peers and persistent state premises.',
                        'Native physical power failure, Byzantine peers and unaided learner work are not inferred.'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path,required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
