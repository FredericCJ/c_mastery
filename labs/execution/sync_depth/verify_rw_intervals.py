"""Independent interval-set audit of native reader/writer observations.

This checks emitted lifetimes and work equality, not the admission algorithm,
every memory access, hardware cache traffic, fairness or all C executions.
"""
import argparse
from collections import Counter
import json
from pathlib import Path

def fields(line):
    return dict(piece.split('=',1) for piece in line.split(':',1)[1].split(';'))

def check(text):
    samples={};events={};workers={}
    for line in text.splitlines():
        if not line.startswith(('CDRN_SAMPLE:','CDRN_INTERVAL:','CDRN_WORKER:')):continue
        row=fields(line);key=(row['mode'],int(row['repeat']),int(row['cohort']))
        if line.startswith('CDRN_SAMPLE:'):
            if key in samples:raise ValueError('Repeated sample')
            samples[key]=row
        elif line.startswith('CDRN_INTERVAL:'):events.setdefault(key,[]).append(row)
        else:workers.setdefault(key,[]).append(row)
    expected={(m,i,0) for m in ('writer','phase','mutex') for i in range(5)}|{('writer',0,1),('phase',0,1)}
    if set(samples)!=expected or set(events)!=expected or set(workers)!=expected:raise ValueError('Missing actual sample/trace/worker')
    result=[]
    for key,sample in samples.items():
        rounds=1 if key[2] else 200
        cpus={int(sample['cpu0']),int(sample['cpu1'])}
        if len(cpus)!=2 or min(cpus)<0:raise ValueError('Two allowed CPUs not observed')
        if (int(sample['read_ops'])!=rounds*3 or int(sample['write_ops'])!=rounds or int(sample['payload_words'])!=64
            or int(sample['elapsed_ns'])<=0):raise ValueError('Wrong work/elapsed observation')
        if len(workers[key])!=4 or {int(w['task']) for w in workers[key]}!=set(range(4)):raise ValueError('Missing exact worker')
        for w in workers[key]:
            task=int(w['task']);expected_cpu=int(sample['cpu'+str(task%2)])
            if int(w['cpu'])!=expected_cpu or int(w['wait_ns'])<0:raise ValueError('Wrong effective affinity or wait time')
            if task<3 and not 0<=int(w['checksum'])<=64*rounds*rounds:raise ValueError('Payload checksum outside declared workload')
            if task==3 and int(w['checksum'])!=0:raise ValueError('Unexpected writer checksum')
        active={};completed=Counter();last_time=0;max_readers=0
        if len(events[key])!=rounds*8:raise ValueError('Missing interval endpoint')
        for index,event in enumerate(events[key]):
            task=int(event['task']);turn=int(event['round']);stamp=int(event['time']);kind=event['kind']
            if int(event['index'])!=index or stamp<last_time or task not in range(4) or turn not in range(rounds):raise ValueError('Malformed interval order')
            last_time=stamp
            if kind in ('R','W'):
                if task in active or completed[task]!=turn:raise ValueError('Overlapping or reordered participant lifetime')
                if (kind=='W')!=(task==3):raise ValueError('Wrong participant access kind')
                if kind=='W' and active:raise ValueError('Writer overlaps another owner')
                if kind=='R' and any(k=='W' for _,k in active.values()):raise ValueError('Reader overlaps writer')
                if key[0]=='mutex' and active:raise ValueError('Ordinary mutex permits overlap')
                active[task]=(turn,kind)
                max_readers=max(max_readers,sum(k=='R' for _,k in active.values()))
            elif kind in ('r','w'):
                if active.pop(task,None)!=(turn,kind.upper()):raise ValueError('Unmatched owner release')
                completed[task]+=1
            else:raise ValueError('Unknown interval kind')
        if active or completed!=Counter({i:rounds for i in range(4)}):raise ValueError('Unfinished ownership')
        if max_readers!=int(sample['max_readers']) or (key[2] and max_readers!=3):raise ValueError('Reader overlap count mismatch')
        result.append({'mode':key[0],'repeat':key[1],'cohort':bool(key[2]),'intervals':rounds*4,
            'max_readers':max_readers,'elapsed_ns':int(sample['elapsed_ns']),'effective_cpus':sorted(cpus)})
    if 'CDRN_PASS:two-effective-CPUs;live-reader-cohort3;15-equal-work-samples;actual-CMX-mutex+rw+phase;ordinary-payload-publication' not in text:raise ValueError('Missing complete run marker')
    return {'passed':True,'samples':len(result),'intervals':sum(r['intervals'] for r in result),'observations':result,
        'limits':'Recorded intervals and equal semantic work; no hardware traffic, infinite-stream fairness or weak-memory proof.'}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('log',type=Path)
    args=parser.parse_args();print(json.dumps(check(args.log.read_text(encoding='utf-8')),indent=2))
