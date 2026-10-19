"""Recompute the checkpoint timing model and falsify deliberately wrong budget rules."""
import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

def response(case, *, floor=False, omit_blocking=False):
    c=case['execution_us'];b=0 if omit_blocking else case['blocking_us']
    irq=case['interrupt_execution_us'];period=case['interrupt_period_us'];jitter=case['interrupt_jitter_us']
    if min(c,b,irq,jitter)<0 or period<=0 or case['deadline_us']<=0:raise ValueError('invalid model')
    r=c+b;iterations=[r]
    for _ in range(10000):
        releases=(r+jitter)//period if floor else (r+jitter+period-1)//period
        following=c+b+releases*irq;iterations.append(following)
        if following>case['deadline_us']:return iterations,None,False
        if following==r:return iterations,r,True
        if following<r:raise ValueError('nonmonotonic response iteration')
        r=following
    raise ValueError('iteration bound exceeded')

def uart(case):
    service=Fraction(case['bits_per_byte']*case['frame_bytes']*1000000,case['baud'])
    arrivals=range(0,case['burst_duration_us']+1,case['arrival_period_us'])
    departures=[];available=Fraction(0)
    for arrival in arrivals:
        available=max(available,arrival)+service;departures.append(available)
    # A departure exactly at an arrival is processed first, releasing capacity.
    peak=max(i+1-sum(d<=a for d in departures) for i,a in enumerate(arrivals))
    completed=sum(d<=case['burst_duration_us'] for d in departures)
    rate=Fraction(case['bits_per_byte']*case['frame_bytes']*1000000,case['arrival_period_us'])
    return {'service_us':str(service),'required_bits_per_second':int(rate),'completed':completed,
            'peak_outstanding':peak,'stable':rate<=case['baud']}

def run():
    path=ROOT/'fixtures/legacy_checkpoints/transfer/P4_timing.json';value=json.loads(path.read_text(encoding='utf-8'))
    observations=[]
    for case in value['response_cases']:
        trace,bound,meets=response(case)
        assert trace==case['expected_iterations'] and bound==case['expected_response_us'] and meets==case['expected_meets_deadline'],(case['name'],trace,bound,meets)
        observations.append({'case':case['name'],'iterations':trace,'response_us':bound,'meets_deadline':meets})
    arrivals=value['arrival_window'];count=arrivals['blackout_us']//arrivals['minimum_separation_us']+1
    assert count==arrivals['expected_arrivals'] and arrivals['physical_ring_slots']-1==arrivals['expected_usable_slots']
    observed=uart(value['uart'])
    assert all(observed[k]==value['uart']['expected_'+k] for k in observed),observed
    negatives=[response(value['response_cases'][0],floor=True)!=response(value['response_cases'][0]),
               response(value['response_cases'][0],omit_blocking=True)!=response(value['response_cases'][0]),
               arrivals['physical_ring_slots']!=arrivals['expected_usable_slots'],
               value['uart']['frame_bytes']*1000000//value['uart']['arrival_period_us']<value['uart']['baud'] and not observed['stable']]
    assert all(negatives)
    return {'passed':True,'response_cases':observations,'arrival_window_count':count,'uart':observed,
            'negative_controls':{'floor_instead_of_ceiling':negatives[0],'omit_blocking':negatives[1],'count_reserved_ring_slot':negatives[2],'compare_bytes_to_bits':negatives[3]},
            'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in (path,Path(__file__).resolve())},
            'scope':'Exact rational/integer recomputation of declared synthetic model only; no target timing, WCET measurement, independent learner assessment or hardware observation.'}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report');args=parser.parse_args();result=run()
    if args.report:
        destination=(ROOT/args.report).resolve()
        if not destination.is_relative_to((ROOT/'reviews/v5/expansion/legacy_completion').resolve()):raise ValueError('completion evidence only')
        destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps(result,indent=2))
