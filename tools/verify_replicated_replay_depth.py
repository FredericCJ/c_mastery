"""Independent integer/byte fold: never imports the C reducer or its fixtures."""
import argparse
import hashlib
import json
import random
import struct
import subprocess
from pathlib import Path

INITIAL=((0,0,0,1),(0,7,0,9))
def step(s,m):
    tick,pos,score,seed=s
    seed=(1664525*seed+1013904223)%(1<<32)
    return tick+1,(pos+m)%16,score+(seed&3),seed

def snapshot(local,actual,revision,next_effect=0,pending=False):
    head=len(local);known=[v is not None for v in actual];confirmed=0
    while confirmed<head and known[confirmed]:confirmed+=1
    predicted=0;remote=[];states=[INITIAL]
    for i,m in enumerate(local):
        if known[i]:predicted=actual[i]
        remote.append(predicted)
        states.append((step(states[-1][0],m),step(states[-1][1],predicted)))
    b=bytearray(480);b[:4]=b'RPR1';b[4:8]=bytes((1,8,head,confirmed))
    struct.pack_into('>QI',b,8,9,revision);b[20:22]=bytes((0,7))
    struct.pack_into('>II',b,28,1,9)
    for i in range(head):b[36+i]=local[i]+1;b[44+i]=remote[i]+1;b[52+i]=known[i]
    struct.pack_into('>I',b,60,next_effect);b[64]=pending
    if pending:
        a,c=states[next_effect];value=2*(a[1]+16*c[1]+256*(a[2]+c[2]))
        struct.pack_into('>II',b,68,2*value,next_effect)
    for i,pair in enumerate(states):
        for j,s in enumerate(pair):struct.pack_into('>IIII',b,80+32*i+16*j,*s)
    return bytes(b)+hashlib.sha256(b).digest()

def digits(code,n):
    result=[]
    for _ in range(n):result.append(code%3-1);code//=3
    return result
def order_code(order):return sum(v*9**i for i,v in enumerate(order))
def trace(n,lc,rc,order):
    local=digits(lc,n);remote=digits(rc,n);history=[];known=[];revision=1
    h=hashlib.sha256(snapshot(history,known,revision));rows=1
    for v in local:
        history.append(v);known.append(None);revision+=1
        h.update(snapshot(history,known,revision));rows+=1
    for tick in order:
        known[tick-1]=remote[tick-1];revision+=1
        h.update(snapshot(history,known,revision));rows+=1
    for tick in range(1,n+1):
        h.update(snapshot(history,known,revision,tick,True));h.update(snapshot(history,known,revision,tick,False));rows+=2
    return h.hexdigest(),rows
def cases():
    # All 0..8 one-player ternary histories; the other player's inputs are the
    # reversed sign. Late corrections arrive in reverse tick order.
    for n in range(9):
        for code in range(3**n):yield n,code,3**n-1-code,list(range(n,0,-1))
    rng=random.Random(7609)
    for i in range(768):
        n=1+i%8;order=list(range(1,n+1));rng.shuffle(order)
        yield n,rng.randrange(3**n),rng.randrange(3**n),order
def verify(driver):
    records=list(cases());commands=[];expected=[];snapshots=0
    for n,lc,rc,order in records:
        commands.append(f'{n} {lc} {rc} {order_code(order)}\n')
        result,count=trace(n,lc,rc,order);expected.append(result);snapshots+=count
    p=subprocess.run([str(driver),'oracle'],input=''.join(commands),capture_output=True,text=True,timeout=240)
    assert p.returncode==0,(p.returncode,p.stderr)
    actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(a,e) in enumerate(zip(actual,expected)):assert a==e,('trace',i,records[i],a,e)
    return {'passed':True,'ordinary_histories':9841,'changed_order_histories':768,'histories':len(records),'snapshots':snapshots,'trace_sha256':hashlib.sha256(p.stdout.encode()).hexdigest()}
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path,required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
