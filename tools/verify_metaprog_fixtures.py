"""Independent finite/set/integer oracles for original metaprogramming targets."""
import argparse
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]

def hash_bytes(raw):
    value=14695981039346656037
    for byte in raw:value=((value^byte)*1099511628211)%(1<<64)
    return value

def word(value):return value.to_bytes(8,'little')

def verify_fixtures():
    cases=json.loads((ROOT/'fixtures/metaprog/cases.json').read_text(encoding='utf-8'))['cases']
    for row in cases:
        k=row['kind'];v=row['input'];e=row['expected']
        if k=='mono':a=v['push']+[v['pop_then_push']]
        elif k=='constraints':a=[v['count']*v['width'] if v['count']*v['width']<=limit else None for limit in v['limits']]
        elif k=='policies':a={'reject':sum(v['values']) if sum(v['values'])<=v['limit'] else None,'saturate':min(sum(v['values']),v['limit']),'empty':0}
        elif k=='variadic':a=[sum(range(v['seed'],v['seed']+arity)) for arity in v['arities']]
        elif k=='generic':a={'sum':v['mutable']+100+v['const']+200,'evaluations':2}
        elif k=='reflection':a=(v['code'].to_bytes(2,'big')+v['value'].to_bytes(4,'big')).hex()
        elif k=='laws':a={'equal':v['a'][:2]==v['b'][:2],'first_sum_failure':next([x,y] for x in range(v['bound']+1) for y in range(v['bound']+1) if x+y>=5)}
        elif k=='formatting':a=[str(n).encode().hex() for n in v['values']]
        elif k=='hygiene':a={'contexts':[v['fresh'],v['argument_context'],v['fresh']],'next':v['fresh']+1}
        elif k=='buildgraph':
            def order(deps):
                done=set();out=[]
                while len(out)<len(deps):
                    ready=[i for i,d in enumerate(deps) if i not in done and {j for j in range(len(deps)) if d&(1<<j)}<=done]
                    if not ready:return None
                    out.append(min(ready));done.add(min(ready))
                return out
            a={'order':order(v['dependencies']),'cycle_accepted':order(v['cycle']) is not None}
        else:raise ValueError(k)
        assert a==e,(k,a,e)
    return len(cases)

def differential(driver):
    requests=[];expected=[]
    for x in range(16):
        for y in range(16):
            for cache in [0,9,4294967295]:
                requests.append(f'H {x} {y} {cache}');expected.append(str(hash_bytes(word(x)+word(y))))
    for n in list(range(100))+[255,256,999,1000,4294967295]:
        raw=str(n).encode()
        for cap in range(12):
            requests.append(f'F {n} {cap}');expected.append('1 '+str(len(raw))+' '+' '.join(f'{b:02x}' for b in raw) if len(raw)<=cap else '0 0')
    for configuration in range(64):
        k0=hash_bytes(word(2)+b'\x01\x02'+word(10)+word(configuration))
        k1=hash_bytes(word(1)+b'\x03'+word(10)+word(20)+word(0)+word(k0))
        requests.append(f'B {configuration}');expected.append(f'0 1 {k0} {k1}')
    run=subprocess.run([str(driver)],input='\n'.join(requests)+'\n',text=True,capture_output=True,check=True)
    observed=run.stdout.splitlines();assert len(observed)==len(expected),(len(observed),len(expected))
    for i,(actual,want) in enumerate(zip(observed,expected)):assert actual==want,(requests[i],actual,want)
    return len(requests)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',type=Path);args=parser.parse_args()
    count=verify_fixtures();runs=differential(args.driver) if args.driver else 0
    print(json.dumps({'passed':True,'fixture_cases':count,'independent_driver_cases':runs,'scope':'Python mathematical reduction, explicit byte encodings, set scheduling and specified hash recurrence; no crypto or universal correctness claim.'}))

if __name__=='__main__':main()
