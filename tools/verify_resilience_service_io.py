"""Independent binary wire grammar oracle with complete-prefix accounting."""
import argparse
import itertools
import json
import random
import struct
import subprocess


def model(raw):
    consumed=min(len(raw),18);status=8;fields=(99,99,99,99);malformed=False
    if len(raw)>=2 and int.from_bytes(raw[:2],'big')!=16:status=18;consumed=2;malformed=True
    elif len(raw)>=18:
        version,client,identity,delta=raw[2:6];epoch=int.from_bytes(raw[6:14],'big')
        valid=version==1 and client<=1 and 1<=identity<=15 and delta<=16 and epoch and raw[14:18]==bytes(4)
        if valid:status=0;fields=(client,identity,epoch,delta-8)
        else:status=18;malformed=True
    ended=18 if malformed or 0<consumed<18 else 16 if not consumed else 14
    malformed=ended==18
    return ','.join(map(str,[status,consumed,ended,*fields,int(malformed)]))


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    cases=[];expected=[]
    def add(raw,chunk):cases.append(f'{chunk} {raw.hex() or "-"}');expected.append(model(raw))
    for client,identity,delta,epoch,reserved in itertools.product([0,1,2],[0,1,15,16],[0,8,16,17],[0,1,2**64-1],[0,1]):
        raw=struct.pack('>HBBBBQ4B',16,1,client,identity,delta,epoch,reserved,0,0,0)
        for chunk in [1,2,3,5,18]:add(raw,chunk)
    good=struct.pack('>HBBBBQ4B',16,1,1,15,0,2**64-1,0,0,0,0)
    for n in range(37):
        for chunk in range(1,19):add((good+good)[:n],chunk)
    for position,value in itertools.product(range(18),range(256)):
        raw=bytearray(good);raw[position]=value;add(raw,position%18+1)
    structured=len(cases);randomizer=random.Random(75910)
    for _ in range(5000):
        raw=bytearray(good+good)
        for _ in range(randomizer.randrange(5)):raw[randomizer.randrange(len(raw))]=randomizer.randrange(256)
        add(raw[:randomizer.randrange(37)],randomizer.randrange(1,19))
    result=subprocess.run([args.driver,'oracle'],input='\n'.join(cases)+'\n',capture_output=True,text=True,timeout=90)
    assert result.returncode==0,result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for index,(got,want) in enumerate(zip(actual,expected)):assert got==want,(index,cases[index],got,want)
    print(json.dumps({'wire_cases':len(cases),'structured_cases':structured,'random_cases':5000,
                      'scope':'Binary grammar and consumed prefixes; native POSIX concurrency/cancellation tests are separate.'}))


if __name__=='__main__':main()
