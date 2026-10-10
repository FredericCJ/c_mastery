"""Independent arithmetic/state oracles for integration fixtures, not solved C."""
from fractions import Fraction
from statistics import median
import zlib
from common import ROOT, read_json

def main():
    folder=ROOT/'fixtures/integration'; seen=set()
    def check(name, result):
        data=read_json(folder/(name+'.json'))
        assert result==data['expected'],(name,result,data['expected'])
        seen.add(name)
    data=read_json(folder/'queue.json')['input']; q=[]; statuses=[]; removed=[]
    for op in data['operations']:
        if op[0]=='put':
            if len(q)==data['capacity']: statuses.append('FULL')
            else: q.append(op[1]); statuses.append('OK')
        else: removed.append(q.pop(0)); statuses.append('OK')
    check('queue',{'statuses':statuses,'removed':removed,'remaining':q})
    data=read_json(folder/'binary.json')['input']; remaining=data['write_request_bytes']; trace=[]
    for outcome in data['write_outcomes']:
        trace.append(remaining)
        if isinstance(outcome,int): remaining-=outcome
    check('binary',{'forbidden':sorted(set(data['referenced'])-set(data['allowed'])),'remaining_before_call':trace,'transferred':data['write_request_bytes']-remaining})
    data=read_json(folder/'wire.json')['input']; payload=bytes.fromhex(data['payload_hex'])
    body=bytes([data['version']])+data['sequence'].to_bytes(2,'little')+len(payload).to_bytes(2,'little')+payload
    frame=b'\xa5'+body+zlib.crc32(body).to_bytes(4,'little')
    assert zlib.crc32(b'123456789')==0xcbf43926
    check('wire',{'frame_hex':frame.hex(),'frame_bytes':len(frame),'maximum_frame_bytes':10+data['max_payload'],'empty_frame_bytes':10})
    data=read_json(folder/'irq.json')['input']; elapsed=(data['end']-data['start'])%(1<<data['counter_bits'])
    assert elapsed < (1<<(data['counter_bits']-1))
    check('irq',{'elapsed_ticks':elapsed,'accepted':data['samples'][:data['capacity']],'dropped':max(0,len(data['samples'])-data['capacity'])})
    data=read_json(folder/'pressure.json')['input']; q=[]; accepted=[]; rejected=[]; consumed=[]; closed=False
    for op in data['events']:
        if op[0]=='close': closed=True
        elif op[0]=='take':
            if q: consumed.append(q.pop(0))
        elif closed or len(q)==data['capacity']: rejected.append(op[1])
        else: q.append(op[1]); accepted.append(op[1])
    check('pressure',{'accepted':accepted,'rejected':rejected,'consumed':consumed,'remaining':q,'closed':closed})
    data=read_json(folder/'fir.json')['input']; outputs=[]
    for n in range(len(data['samples'])):
        outputs.append(str(sum((Fraction(c*data['samples'][n-j],data['denominator']) for j,c in enumerate(data['coefficient_numerators']) if n-j>=0),Fraction(0))))
    check('fir',{'output':outputs,'median_ns':median(data['runs_ns']),'min_ns':min(data['runs_ns']),'max_ns':max(data['runs_ns'])})
    data=read_json(folder/'recovery.json')['input']; valid=[s for s in data['slots'] if s['valid']]; best=max(valid,key=lambda s:s['generation'])
    check('recovery',{'selected':best['name'],'generation':best['generation'],'invalid_ignored':[s['name'] for s in data['slots'] if not s['valid']]})
    assert seen=={p.stem for p in folder.glob('*.json')}
    print(f'Integration: {len(seen)} fixtures independently recomputed, zero failures.')

if __name__=='__main__': main()
