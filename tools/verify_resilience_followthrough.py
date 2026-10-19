"""Arbitrary-precision rational/ordered-owner models independent of C representation."""
import argparse
from fractions import Fraction
import itertools
import json
from pathlib import Path
import random
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
MAX = (1 << 64) - 1

def run(executable, mode, cases, expected):
    result = subprocess.run([str(executable), mode], input='\n'.join(cases)+'\n',
                            text=True, capture_output=True, timeout=90)
    assert result.returncode == 0, result.stdout + result.stderr
    actual = result.stdout.splitlines()
    assert len(actual) == len(expected), (mode, len(actual), len(expected))
    for index, (got, want) in enumerate(zip(actual, expected)):
        assert got == want, (mode, index, cases[index], got, want)
    return len(cases)

def budget_cases():
    cases=[];expected=[]
    for remaining, transit, processing, origin, parent in itertools.product(
            range(17), range(9), range(9), [0, 1000, MAX-8, MAX], [0, 3, MAX]):
        cases.append(f'{remaining} {transit} {processing} {origin} {parent}')
        deadline=0
        if parent != MAX and origin + parent > MAX: status=7
        elif not remaining or transit >= remaining: status=1
        elif processing >= remaining-transit: status=5
        else:
            budget=remaining-transit-processing
            if parent != MAX: budget=min(budget,parent)
            if origin + budget > MAX: status=7
            else: status=0;deadline=origin+budget
        expected.append(f'{status} {deadline}')
    return cases,expected

def fractional_cases():
    cases=[];expected=[]
    def add(capacity,p,q,last,operations):
        cases.append(' '.join(map(str,[capacity,p,q,last,len(operations),
                                       *itertools.chain.from_iterable(operations)])))
        valid=0<capacity<=(1<<32)-1 and 0<p<=65535 and 0<q<=65535
        output=['0' if valid else '1'];credit=Fraction(capacity if valid else 0)
        if not valid:last=0
        for now,cost in operations:
            if not valid:status=1
            elif now<last or cost>capacity:status=7
            else:
                credit=min(Fraction(capacity),credit+(now-last)*Fraction(p,q));last=now
                status=0 if credit>=cost else 15
                if status==0:credit-=cost
            tokens=int(credit);remainder=int((credit-tokens)*q) if valid else 0
            output.append(f'{status},{tokens},{remainder},{last}')
        expected.append(' '.join(output))
    for capacity,p,q in itertools.product(range(5),range(5),range(1,6)):
        for elapsed,cost in itertools.product(range(9),range(capacity+2)):
            add(capacity,p,q,0,[(0,capacity),(elapsed,cost),(elapsed+1,1),(elapsed,0)])
    randomizer=random.Random(75004)
    for _ in range(3000):
        capacity=randomizer.choice([1,2,17,(1<<32)-1]);p=randomizer.choice([1,2,65535]);q=randomizer.choice([1,3,65535])
        times=sorted([randomizer.randrange(40) for _ in range(6)])+[MAX]
        add(capacity,p,q,0,[(0,capacity)]+[(now,randomizer.choice([0,1,capacity,capacity+1])) for now in times])
    for capacity,p,q in itertools.product([1,4],[1,3],[1,3]):
        for length in range(7):
            for word in itertools.product('stbi',repeat=length):
                now=0;operations=[]
                for char in word:
                    if char=='t':now+=1
                    operations.append((max(0,now-1) if char=='b' else now,
                                       capacity+1 if char=='i' else 0 if char=='b' else 1))
                add(capacity,p,q,0,operations)
    return cases,expected

def deadline_cases():
    cases=[];expected=[]
    for parent_now,parent_budget in itertools.product(range(17),repeat=2):
        for child_now,child_budget in itertools.product(range(parent_now,17),range(17)):
            cases.append(f'{parent_now} {parent_budget} {child_now} {child_budget}')
            expected.append(f'0 {min(parent_now+parent_budget,child_now+child_budget)}')
    for args in [(MAX,0,MAX,MAX),(MAX,1,MAX,0),(MAX-1,1,MAX-1,MAX)]:
        p,b,n,c=args;cases.append(' '.join(map(str,args)))
        expected.append('7 0' if p+b>MAX else f'0 {min(p+b,n+c)}')
    return cases,expected

def migration_cases():
    cases=[];expected=[]
    for capacity,p,q,elapsed,downtime,origin,policy in itertools.product(
            [1,4],[1,3],[2,3],[0,1,5,MAX],[0,2,MAX],[0,1000,MAX],[0,1,2]):
        spent=capacity
        cases.append(' '.join(map(str,[capacity,p,q,spent,elapsed,downtime,origin,policy])))
        credit=min(Fraction(capacity),Fraction(elapsed*p,q))
        if policy==2:expected.append('1 0 0 0');continue
        if policy==1:credit=min(Fraction(capacity),credit+Fraction(downtime*p,q))
        tokens=int(credit);remainder=int((credit-tokens)*q)
        expected.append(f'0 {tokens} {remainder} {origin}')
    return cases,expected

def pair_cases():
    cases=[];expected=[]
    for ac,pc,aq,pq in itertools.product([1,3],[1,2],[1,3],[1,4]):
        for length in range(6):
            for word in itertools.product('sti',repeat=length):
                now=last=0;average=Fraction(ac);peak=Fraction(pc);operations=[];output=[]
                for char in word:
                    if char=='t':now+=1
                    cost=max(ac,pc)+1 if char=='i' else 1;operations.append((now,cost))
                    if cost>min(ac,pc):status=7
                    else:
                        average=min(Fraction(ac),average+Fraction(now-last,aq))
                        peak=min(Fraction(pc),peak+Fraction(now-last,pq));last=now
                        status=0 if min(average,peak)>=cost else 15
                        if status==0:average-=cost;peak-=cost
                    at,pt=int(average),int(peak)
                    output.append(f'{status},{at},{int((average-at)*aq)},{pt},{int((peak-pt)*pq)},{last};')
                cases.append(' '.join(map(str,[ac,1,aq,pc,1,pq,len(operations),*itertools.chain.from_iterable(operations)])))
                expected.append(''.join(output))
    return cases,expected

def owner_cases():
    cases=[];expected=[]
    for length in range(1,6):
        for chars in itertools.product('abce',repeat=length):
            cases.append(''.join(chars));live={0,1};cancelled=expired=False;output=[]
            for char in chars:
                status=0
                if char=='c':cancelled=True
                elif char=='e':expired=True
                else:
                    borrower=ord(char)-ord('a')
                    if borrower not in live:status=6
                    else:live.remove(borrower)
                observed=14 if not live else 4 if cancelled else 5 if expired else 0
                output.append(f'{status},{observed},{int(not live)};')
            expected.append(''.join(output)+'D2')
    return cases,expected

def policy_counterexamples():
    # Each policy admits two initially and two per four ticks in steady state.
    # Equal configured numbers do not make their burst envelopes equivalent.
    arrivals=[(3,2),(4,2)]
    fixed={};window=[];credit=Fraction(2);last=0;hist=[[],[],[]]
    for now,cost in arrivals:
        bucket=now//4;fixed_ok=fixed.get(bucket,0)+cost<=2
        if fixed_ok:fixed[bucket]=fixed.get(bucket,0)+cost
        window=[t for t in window if t>now-4];sliding_ok=len(window)+cost<=2
        if sliding_ok:window += [now]*cost
        credit=min(Fraction(2),credit+Fraction(now-last,2));last=now;token_ok=credit>=cost
        if token_ok:credit-=cost
        for history,value in zip(hist,[fixed_ok,sliding_ok,token_ok]):history.append(value)
    assert hist==[[True,True],[True,False],[True,False]],hist
    return {'arrivals':arrivals,'fixed_window':hist[0],'sliding_window':hist[1],'token_bucket':hist[2]}

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--exe','--driver',dest='exe');args=parser.parse_args()
    baseline=subprocess.run([sys.executable,str(ROOT/'labs/resilience/tests/verify_fixtures.py')],
                            text=True,capture_output=True)
    assert baseline.returncode==0,baseline.stdout+baseline.stderr
    summary={'baseline':json.loads(baseline.stdout),'policy_counterexample':policy_counterexamples()}
    if args.exe:
        executable=Path(args.exe)
        for mode,factory in [('budget_oracle',budget_cases),('fractional_oracle',fractional_cases),('scope_oracle',owner_cases),
                             ('deadline_oracle',deadline_cases),('migration_oracle',migration_cases),('pair_oracle',pair_cases)]:
            cases,expected=factory();summary[mode]=run(executable,mode,cases,expected)
        timers=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_timers.py'),
                               '--driver',str(executable.with_name('resilience-timers'))],text=True,capture_output=True)
        assert timers.returncode==0,timers.stdout+timers.stderr
        summary['timers']=json.loads(timers.stdout)
        retry=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_retry.py'),
                              '--driver',str(executable.with_name('resilience-retry'))],text=True,capture_output=True)
        assert retry.returncode==0,retry.stdout+retry.stderr
        summary['retry']=json.loads(retry.stdout)
        pressure=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_pressure.py'),
                                 '--driver',str(executable.with_name('resilience-pressure'))],text=True,capture_output=True)
        assert pressure.returncode==0,pressure.stdout+pressure.stderr
        summary['pressure']=json.loads(pressure.stdout)
        breaker=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_breaker.py'),
                                '--driver',str(executable.with_name('resilience-breaker'))],text=True,capture_output=True)
        assert breaker.returncode==0,breaker.stdout+breaker.stderr
        summary['breaker']=json.loads(breaker.stdout)
        hedge=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_hedge.py'),
                              '--driver',str(executable.with_name('resilience-hedge'))],text=True,capture_output=True)
        assert hedge.returncode==0,hedge.stdout+hedge.stderr
        summary['hedge']=json.loads(hedge.stdout)
        journal=subprocess.run([sys.executable,str(ROOT/'tools/verify_resilience_journal.py'),
                                '--driver',str(executable.with_name('resilience-journal'))],text=True,capture_output=True)
        assert journal.returncode==0,journal.stdout+journal.stderr
        summary['journal']=json.loads(journal.stdout)
        for key,driver in [('outbox','resilience-outbox'),('service_io','resilience-service-io'),('composition','resilience-composition')]:
            result=subprocess.run([sys.executable,str(ROOT/f'tools/verify_resilience_{key}.py'),
                                   '--driver',str(executable.with_name(driver))],text=True,capture_output=True)
            assert result.returncode==0,result.stdout+result.stderr
            summary[key]=json.loads(result.stdout)
    else:
        summary['execution']='Not requested; mathematical fixtures only. Pass --exe to compare actual C.'
    print(json.dumps(summary))
    return 0

if __name__=='__main__':raise SystemExit(main())
