"""Causal graph reachability is the order oracle, independently of timestamps."""
import argparse
import json
import random
import subprocess
import zlib


def history(seed):
    rng=random.Random(seed);nodes=[dict(l=0,h=(0,0),v=[0,0,0]) for _ in range(3)]
    previous=[None]*3;ancestors=[];events=[];inputs=[];wire=None;counterexamples=0
    for event in range(48):
        node=rng.randrange(3);message=rng.randrange(event) if event and rng.randrange(2) else -1
        real=100+event//3;wall=real+rng.randrange(-2,3);local=nodes[node];remote=events[message] if message>=0 else dict(l=0,h=(0,0),v=[0,0,0])
        parents=[x for x in [previous[node],message if message>=0 else None] if x is not None]
        reachable=set(parents)
        for parent in parents:reachable.update(ancestors[parent])
        ancestors.append(reachable);previous[node]=event
        physical=max(wall,local['h'][0],remote['h'][0]);logical=max([p['h'][1] for p in [local,remote] if p['h'][0]==physical],default=-1)+1
        vector=[max(x,y) for x,y in zip(local['v'],remote['v'])];vector[node]+=1
        current=dict(l=1+max(local['l'],remote['l']),h=(physical,logical),v=vector,node=node,real=real,wall=wall)
        assert 0<=physical-wall<=4 and abs(physical-real)<=2
        nodes[node]=current;events.append(current);inputs.append(f'{node} {message} {wall}')
        orders=[-1 if i in reachable else 2 for i in range(event)]
        for i,prior in enumerate(events[:-1]):
            if i in reachable:assert prior['l']<current['l'] and prior['h']<current['h']
            elif prior['l']<current['l']:counterexamples+=1
        current['expected']=[current['l'],*current['h'],*vector];current['orders']=orders
    c=events[-1];entries=[(10+i*10,v) for i,v in enumerate(c['v']) if v]
    raw=bytearray(160);raw[:6]=b'RPC1'+bytes([1,len(entries)])
    for at,value,width in [(8,9,8),(16,10+c['node']*10,8),(24,c['l'],8),(32,c['h'][0],8),(40,c['h'][1],4)]:raw[at:at+width]=value.to_bytes(width,'big')
    for i,(identity,value) in enumerate(entries):raw[48+16*i:64+16*i]=identity.to_bytes(8,'big')+value.to_bytes(8,'big')
    raw[156:]=zlib.crc32(raw[:156]).to_bytes(4,'big');wire=raw.hex()
    return inputs,events,wire,counterexamples


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);a=ap.parse_args();pairs=counterexamples=0
    for seed in range(128):
        inputs,expected,wire,examples=history(760300+seed)
        p=subprocess.run([a.driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=20)
        if p.returncode:raise SystemExit(p.stderr or p.stdout)
        lines=p.stdout.splitlines();assert len(lines)==49
        for i,(line,event) in enumerate(zip(lines,expected)):
            clocks,orders=line.split('|');actual=list(map(int,clocks.split()));relations=list(map(int,orders.split()))
            assert actual==event['expected'],(seed,i,actual,event['expected'])
            assert relations==event['orders'],(seed,i,relations,event['orders']);pairs+=len(relations)
        assert lines[-1]=='WIRE '+wire,(seed,'canonical bytes');counterexamples+=examples
    assert counterexamples>0
    print(json.dumps({'passed':True,'histories':128,'events':6144,'reachability_pairs':pairs,'timestamp_nonconverse_counterexamples':counterexamples,'wire_records':128,'wall_model':'true time nondecreasing, wall offset -2..2, accepted-message causal past; physical-ahead bound4 and absolute error2 only under that premise'}))
if __name__=='__main__':main()
