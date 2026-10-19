"""Independent dictionary/prefix and byte-format oracle for the actual adapters.

Python dictionaries and zlib are host verification tools. Neither is linked into
the learner/assessor C implementation or used to implement its studied mechanism.
"""
import argparse
import json
import random
import subprocess
import zlib


def record(uuid,schema,position,kind,key,value):
    wire=bytearray(64)
    wire[:4]=b'RPL1';wire[4]=1;wire[5]=1
    wire[8:16]=uuid.to_bytes(8,'big')
    wire[16:20]=schema.to_bytes(4,'big')
    wire[20:24]=position.to_bytes(4,'big')
    wire[24]=key;wire[25]=kind;wire[26]=2
    signed=value.to_bytes(4,'big',signed=True)
    wire[27:31]=signed;wire[31]=signed[0]^signed[1]^signed[2]^signed[3]
    wire[60:64]=zlib.crc32(wire[:60]).to_bytes(4,'big')
    return list(wire)


def cases():
    commands=[];expected=[];counts={'wire':0,'prefix':0,'state':0,'restart':0,'append':0,'snapshot':0}
    def add(command,result,kind=None):
        commands.append(command);expected.append(list(result))
        if kind:counts[kind]+=1
    values=[-(1<<31),-(1<<31)+1,-65536,-32769,-32768,-1,0,1,32767,32768,65535,(1<<31)-2,(1<<31)-1]
    for uuid in [1,0x1020304050607080,(1<<64)-1]:
        for schema in [1,17,(1<<32)-1]:
            for key in [0,3,31]:
                for value in values:
                    for kind in [1,2]:
                        pos=1+((key+kind)%16)
                        add(f'E {uuid} {schema} {pos} {kind} {key} {value}',[0,*record(uuid,schema,pos,kind,key,value)],'wire')
    for uuid,schema,pos,kind,key in [(0,1,1,1,0),(1,0,1,1,0),(1,1,0,1,0),(1,1,17,1,0),(1,1,1,0,0),(1,1,1,3,0),(1,1,1,1,32)]:
        add(f'E {uuid} {schema} {pos} {kind} {key} 0',[1],'wire')
    rng=random.Random(7601)
    for history in range(64):
        add('N',[0]);roots=[{}];length=history%17
        for index in range(length):
            key=rng.randrange(32);value=rng.choice(values);kind=rng.randrange(1,3)
            state=dict(roots[-1]);state[key]=value;roots.append(state)
            add(f'A {kind} {key} {value}',[0],'append')
            for prefix in sorted({0,index//2,index+1}):
                for query in sorted({0,key,31}):
                    found=query in roots[prefix]
                    add(f'G {prefix} {query}',[0 if found else 6,roots[prefix].get(query,1234567)],'prefix')
        prefix=rng.randrange(length+1)
        add(f'S {prefix}',[0],'snapshot')
        current=roots[-1];mask=sum(1<<key for key in current)
        result=[0,length,mask,*[current.get(key,0) for key in range(32)]]
        add('Q 0',result,'state');add('Q 1',result,'state')
        add('R',[length],'restart');add('Q 1',result,'state')
        if length==16:add('A 1 0 0',[2],'append')
        add(f'G {length+1} 0',[1,1234567],'prefix')
        add('A 3 0 0',[1],'append')
        add('A 1 32 0',[1],'append')
    return commands,expected,counts


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    commands,expected,counts=cases()
    result=subprocess.run([args.driver,'oracle'],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120)
    if result.returncode:raise AssertionError((result.returncode,result.stdout[-2000:],result.stderr))
    lines=result.stdout.splitlines()
    if len(lines)!=len(expected):raise AssertionError(('row count',len(lines),len(expected)))
    for number,(command,want,line) in enumerate(zip(commands,expected,lines)):
        got=[int(x) for x in line.split()]
        if got!=want:raise AssertionError((number,command,want,got))
    print(json.dumps({'passed':True,'cases':len(commands),'counts':counts,
                      'scope':'Actual C canonical event bytes, all queried retained prefixes, durable positioned snapshots, suffix replay and reopened page-store state; no physical durability claim.'}))


if __name__=='__main__':main()
