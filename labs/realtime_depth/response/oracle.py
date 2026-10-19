"""Independent Python-integer expectations; no port of the C restoring divider."""
import argparse
import itertools
from pathlib import Path
import random
MAX=(1<<64)-1
def expected_response(tasks,target,limit):
    invalid=(not tasks or len(tasks)>16 or target>=len(tasks) or not 1<=limit<=128 or any(not t[1] or not t[2] for t in tasks))
    if invalid:return 'R 3 777 0 0'
    own=tasks[target];base=own[0]+own[4]+own[5]
    if base>MAX:return 'R 4 777 0 0'
    values=[base];status=0 if not base else 1 if base>own[2] else 2
    if base and base<=own[2]:
        for _ in range(limit):
            total=base
            for cost,period,deadline,jitter,blocking,overhead in tasks[:target]:
                count=-(-(values[-1]+jitter)//period)
                if count>MAX or cost+overhead>MAX or count*(cost+overhead)>MAX:return 'R 4 777 0 0'
                total+=count*(cost+overhead)
                if total>MAX:return 'R 4 777 0 0'
            previous=values[-1];values.append(total)
            if total>own[2]:status=1;break
            if total==previous:status=0;break
    return ' '.join(map(str,['R',status,values[-1],len(values)-1,*values]))
def cases():
    for a,b,d in itertools.product(range(16),range(16),range(10)):
        q=-(-(a+b)//d) if d else 777
        yield f'1 {a} {b} {d}',f'A {0 if d else 3} {q}'
    edge=(0,1,2,3,(1<<31)-1,1<<32,(1<<63)-1,1<<63,MAX-1,MAX)
    for a,b,d in itertools.product(edge,repeat=3):
        q=-(-(a+b)//d) if d else 777;status=3 if not d else 4 if q>MAX else 0
        yield f'1 {a} {b} {d}',f'A {status} {q if status==0 else 777}'
    generator=random.Random(5203)
    for _ in range(2400):
        count=generator.randrange(1,5);target=count-1;limit=generator.randrange(1,15)
        tasks=[(generator.randrange(0,6),generator.randrange(1,14),generator.randrange(1,75),generator.randrange(0,5),generator.randrange(0,3),generator.randrange(0,3)) for _ in range(count)]
        yield ' '.join(map(str,[2,count,target,limit,*itertools.chain.from_iterable(tasks)])),expected_response(tasks,target,limit)
    for _ in range(600):
        tasks=[tuple(generator.choice(edge) for _ in range(6)) for _ in range(2)];cap=generator.randrange(1,7)
        yield ' '.join(map(str,[2,2,1,cap,*itertools.chain.from_iterable(tasks)])),expected_response(tasks,1,cap)
def check(output):
    rows=output.splitlines();expected=[answer for _,answer in cases()]
    if len(rows)!=len(expected):raise ValueError('Missing or extra arithmetic/recurrence row')
    for index,(actual,wanted) in enumerate(zip(rows,expected)):
        if actual!=wanted:raise ValueError(f'Row{index}: {actual!r} != {wanted!r}')
    return {'rows':len(rows),'ceil_rows':3560,'response_rows':3000,'scope':'Finite exact Python integers; specified count/arithmetic/status model, no schedulability theorem or target timing proof.'}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--input',type=Path);parser.add_argument('--check',type=Path)
    args=parser.parse_args()
    if args.input:args.input.write_text('\n'.join(line for line,_ in cases())+'\n',encoding='utf-8',newline='\n')
    if args.check:print(check(args.check.read_text(encoding='utf-8')))
