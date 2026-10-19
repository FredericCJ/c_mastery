"""Independent map/prefix-durability and wire-format oracle for T75.08."""
import argparse
import itertools
import json
import struct
import subprocess


def record(sequence,window,kind,now,fence,client=0,identity=0,delta=0,result=0,duplicate=False):
    body=bytearray(56);body[:4]=b'RZJ1';body[4:8]=bytes([kind,client,identity,delta+8])
    struct.pack_into('>QQQi',body,8,sequence,now,fence,result)
    body[36:38]=bytes([duplicate,window]);checksum=2166136261
    for octet in body[:48]:checksum=((checksum^octet)*16777619)&0xffffffff
    struct.pack_into('>I',body,48,checksum)
    return bytes(body)+b'COM1'+struct.pack('>I',checksum^0xffffffff)


def cases():
    inputs=[];expected=[]
    for window in range(1,5):
        for issued,client,identity,delta,fence,now in itertools.product(range(1,4),range(2),range(1,4),range(-1,2),range(1,4),range(window+2)):
            inputs.append(f'{window} {issued} {client} {identity} {delta} {fence} {now}')
            journal=b''.join(record(i,window,2,0,i) for i in range(1,issued+1))
            journal+=record(issued+1,window,1,0,issued,0,1,1,1)
            total=1;value=99;duplicate=True;count=issued+1
            if fence>issued:status=15
            elif fence<issued:status=6
            elif client==0 and identity==1 and now<window and delta!=1:status=13
            else:
                status=0;duplicate=client==0 and identity==1 and now<window
                if not duplicate:total+=delta
                value=1 if duplicate else total;count+=1
                journal+=record(count,window,1,now,fence,client,identity,delta,value,duplicate)
            expected.append(f'{status} {value} {int(duplicate)} {total} {count} {issued} {journal.hex()}')
    return inputs,expected


def cuts():
    inputs=[];expected=[];prefix=record(1,4,2,0,1)
    for operation,mode,cut in itertools.product(range(2),range(3),range(69)):
        inputs.append(f'{operation} {mode} {cut}')
        pending=record(2,4,2,1,2) if operation else record(2,4,1,1,1,0,1,3,3)
        cache=bytearray(prefix);durable=bytearray(prefix);events=[('write',octet) for octet in pending[:56]]+[('flush',None)]+[('write',octet) for octet in pending[56:]]+[('flush',None)]
        status=0
        for step,(kind,octet) in enumerate(events,1):
            if kind=='write':cache.append(octet)
            if step==cut:
                status=17
                if mode==1:durable=cache.copy()
                elif mode==2 and len(cache)%64==0 and len(cache)>len(durable):
                    durable+=bytes(len(cache)-len(durable));durable[-8:]=cache[-8:]
                break
            if kind=='flush':durable=cache.copy()
        durable=durable[:len(durable)//64*64]
        committed=len(durable)==128;count=2 if committed else 1
        value=3 if not operation and status==0 else 99
        duplicate=0 if not operation and status==0 else 1
        token=2 if operation and status==0 else 99
        total=3 if committed and not operation else 0
        issued=2 if committed and operation else 1
        expected.append(f'{status} {value} {duplicate} {token} {int(status!=0)} {total} {count} {issued} {durable.hex()}')
    return inputs,expected


def run(driver,mode,inputs,expected):
    result=subprocess.run([driver,mode],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(mode,len(actual),len(expected))
    for index,(got,want) in enumerate(zip(actual,expected)):assert got==want,(mode,index,inputs[index],got,want)
    return len(inputs)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    count=run(args.driver,'oracle',*cases());cut_count=run(args.driver,'cuts',*cuts())
    print(json.dumps({'request_and_wire_cases':count,'durability_cuts':cut_count,
                      'fault_model':'byte-prefix partial writes; successful flush durability; no/all-unflushed/commit-region-first persistence at each write/flush cut',
                      'limitations':'Finite original journal and virtual persistence model; actual local POSIX file/process tests are separate, not power-loss validation or consensus.'}))
    return 0
if __name__=='__main__':raise SystemExit(main())
