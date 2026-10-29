"""Independent post-decode transition and virtual backend-delay models for T75.07."""
import argparse
import itertools
import json
import random
import subprocess


def histories():
    cases=[];expected=[]
    words=[''.join(word) for length in range(1,5) for word in itertools.product('aAbBx tref'.replace(' ',''),repeat=length)]
    words+=['efBarax','aBtrax','tABrab','xefaBr','efAbrax','eAbfrbx','aBrrxx','efxABrr']
    for word in words:
        cases.append(word);completed=set();winner=-1;cancel=set();abandoned=taken=False;time=2;returned=999;freed=[0,0];total=0;effect=False;output=[]
        for operation in word:
            status=0
            if operation=='t':time=5
            elif operation=='x':
                abandoned=True;cancel|={0,1}-completed
                if winner>=0 and not taken:freed[winner]+=1;taken=True
            elif operation=='r':
                if taken:status=14
                elif winner>=0:taken=True;returned=10+winner;freed[winner]+=1
                elif abandoned:status=4
                else:status=8
            elif operation in 'ef':
                attempt=int(operation=='f')
                if attempt in completed:status=6
                elif abandoned or attempt in cancel or time==5:status=4
                elif not effect:total=1;effect=True
            else:
                attempt=int(operation in 'bB');valid=operation in 'ab'
                if attempt in completed:status=6;freed[attempt]+=1
                else:
                    completed.add(attempt)
                    if time==5:status=5
                    elif valid and not abandoned and winner<0:winner=attempt
                    else:status=14
                    if status!=0:freed[attempt]+=1
                    if winner>=0 or time==5 or abandoned:cancel|={0,1}-completed
            bits=lambda values:sum(1<<value for value in values)
            output.append(f'{status}:{winner}:{bits(completed)}:{2-len(completed)}:{bits(cancel)}:{int(abandoned)}:{int(taken)}:{returned}:{freed[0]}:{freed[1]}:{total};')
        expected.append(''.join(output))
    return cases,expected


def populations():
    randomizer=random.Random(75007);cases=[];expected=[];summaries=[]
    for correlation in ['independent','perfectly-correlated']:
        # Common heavy-tail marginal: 80% duration1..6; 15%7..18; 5%19..64.
        def delay():
            p=randomizer.randrange(100)
            return randomizer.randint(1,6) if p<80 else randomizer.randint(7,18) if p<95 else randomizer.randint(19,64)
        workload=[(delay(),delay()) for _ in range(4096)]
        if correlation=='perfectly-correlated':workload=[(a,a) for a,_ in workload]
        for hedge in [0,1]:
            latencies=[];timeout=backend=work=cleanup=0
            for first,second in workload:
                count=2 if hedge and first>4 else 1
                events=[(first,0)]+([(4+second,1)] if count==2 else [])
                due,winner=min(events);latency=due if due<20 else -1
                if latency<0:winner=-1;timeout+=1
                else:latencies.append(latency)
                amount=first+(second if count==2 else 0);quiescent=max(time for time,_ in events)
                cases.append(f'{first} {second} {hedge}');expected.append(f'{latency} {count} {amount} {quiescent} {winner}')
                backend+=count;work+=amount;cleanup+=quiescent
            values=sorted(latencies)
            quantile=lambda numerator:values[(len(values)*numerator+99)//100-1]
            summaries.append({'delay_model':correlation,'hedged':bool(hedge),'requests':len(workload),
                              'successful':len(values),'timeouts':timeout,'successful_latency_p50':quantile(50),
                              'successful_latency_p95':quantile(95),'successful_latency_p99':quantile(99),
                              'backend_attempts':backend,'backend_duration_sum':work,'quiescence_duration_sum':cleanup,
                              'units':'injected ticks; backend durations are work proxy, not struct lr_cpu or wall-clock measurements',
                              'cancellation_model':'losing work ignores cancellation and completes normally'})
        original,hedged=summaries[-2:]
        if correlation=='perfectly-correlated':
            assert original['successful']==hedged['successful'] and original['successful_latency_p99']==hedged['successful_latency_p99']
            assert hedged['backend_duration_sum']>original['backend_duration_sum']
    return cases,expected,summaries


def compare(driver,mode,cases,expected):
    result=subprocess.run([driver,mode],input='\n'.join(cases)+'\n',text=True,capture_output=True,timeout=90)
    assert result.returncode==0,result.stdout+result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(mode,len(actual),len(expected))
    for index,(a,b) in enumerate(zip(actual,expected)):
        assert a==b,(mode,index,cases[index],a,b)
    return len(cases)


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',required=True);args=parser.parse_args()
    cases,expected=histories();count=compare(args.driver,'oracle',cases,expected)
    cases,expected,summary=populations();population=compare(args.driver,'population',cases,expected)
    print(json.dumps({'histories':count,'population_requests':population,'population_comparison':summary,
                      'limitations':'Serialized post-decode histories; native races are separate tests, no exhaustive thread-schedule or physical timing proof.'}))
    return 0
if __name__=='__main__':raise SystemExit(main())
