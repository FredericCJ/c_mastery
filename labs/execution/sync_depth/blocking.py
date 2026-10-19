"""Independent finite workload recurrence and tick simulation for the PI transfer."""
import argparse
import json
from pathlib import Path


def response(execution, blocking, higher, mutant=0):
    if execution <= 0 or blocking < 0 or any(c <= 0 or period <= 0 for c, period in higher):
        raise ValueError('Positive execution/period and nonnegative blocking required')
    initial = execution + (0 if mutant == 1 else blocking)
    value = initial
    for _ in range(10000):
        next_value = initial + sum(((value // period) if mutant == 2 else ((value+period-1)//period))*cost for cost,period in higher)
        if next_value == value:
            return value
        if next_value < value or next_value > 10000:
            raise ValueError('Outside finite analysis bound')
        value = next_value
    raise ValueError('No bounded fixed point')


def simulate(execution, blocking, higher):
    """Release higher jobs at multiples of period and execute one tick at a time."""
    ready = [0] * len(higher)
    own = execution + blocking
    for now in range(10000):
        for index, (cost, period) in enumerate(higher):
            if now % period == 0:
                ready[index] += cost
        selected = next((i for i,remaining in enumerate(ready) if remaining), None)
        if selected is not None:
            ready[selected] -= 1
        else:
            own -= 1
            if own == 0:
                return now + 1
    raise ValueError('Simulation bound exhausted')


def run(mutant=0):
    data=json.loads((Path(__file__).with_name('blocking.json')).read_text(encoding='utf-8'))
    observations=[]
    for case in data['cases']:
        actual=response(case['execution'],case['blocking'],case['higher'],mutant)
        trace=simulate(case['execution'],case['blocking'],case['higher'])
        if actual != case['expected_response'] or actual != trace or (actual<=case['deadline']) != case['expected_feasible']:
            print('CDPI_CHECK:response/independent tick trace mismatch: '+case['id'])
            return 1
        observations.append({'id':case['id'],'response':actual,'deadline_met':actual<=case['deadline']})
    try:response(2,5,[(1,0)])
    except ValueError:pass
    else:raise AssertionError('Invalid period accepted')
    print('CDPI_RTA:'+json.dumps(observations))
    return 0


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mutant',type=int,choices=(0,1,2),default=0)
    raise SystemExit(run(parser.parse_args().mutant))
