"""Independent admitted-set/error-order model, plus retained baseline oracle."""
import argparse
import itertools
import json
from pathlib import Path
import subprocess
import sys
from collections import deque

ROOT = Path(__file__).resolve().parents[1]


def model(outer, inner, codes):
    groups = range(2 if not outer else outer-1)
    admitted = {2*g+j for g in groups for j in range(2 if not inner[g] else inner[g]-1)}
    errors = [code if i in admitted else 0 for i, code in enumerate(codes)]
    failed = bool(outer or any(inner[g] for g in groups))
    return [sum(1 << i for i in admitted), int(failed), next((x for x in errors if x), 0), *errors]


def owned_model(word):
    queues=[deque(),deque()]; limits=[2,1]; closed=[False,False]
    stop=False; serial=drops=preferred=0; rows=[]
    for op in word:
        selected=77; observed=0
        if op in 'ab':
            channel=ord(op)-ord('a'); serial+=1
            status=4 if stop else 3 if closed[channel] else 2 if len(queues[channel])==limits[channel] else 0
            if status: drops+=1
            else: queues[channel].append(serial)
        elif op=='r':
            ready={i for i in range(2) if queues[i]}
            if stop: status=4
            elif ready:
                status=0;selected=preferred if preferred in ready else next(iter(ready))
                observed=queues[selected].popleft();preferred=1-selected;drops+=1
            else: status=3 if all(closed) else 1
        elif op in 'cd': closed[ord(op)-ord('c')]=True;status=0
        else: assert op=='x';stop=True;status=0
        rows.append(f'{status}:{selected}:{observed}:{drops},')
    return ''.join(rows)+f'D{drops+sum(map(len,queues))}'


def supervision_model(strategy, child, mask):
    live=set(range(16));events=[];attempt=0;parent=rollback=closed=False
    affected={child} if strategy==0 else set(range(4)) if strategy==1 else set(range(child,4))
    def stop(nodes):
        for node in sorted(nodes,reverse=True):
            if node in live: events.append(-node-1);live.remove(node)
    def construct(node):
        nonlocal attempt
        failed=bool(mask&(1<<attempt)) or (node==1 and 0 not in live);attempt+=1
        events.append(node+1+(100 if failed else 0))
        if not failed:live.add(node)
        return not failed
    stop(affected)
    for node in sorted(affected):
        if not construct(node):
            rollback=parent=True;stop(set(range(4)));break
    status=0
    if parent:
        for node in range(4):
            if not construct(node):
                rollback=closed=True;stop(set(range(16)));status=10;break
    return ' '.join(map(str,[status,int(closed),int(parent),int(rollback),attempt,len(live),int(parent),0 if parent and not closed else 1]))+' |'+''.join(str(x)+',' for x in events)


def lifetime_model(word):
    queue=deque();capacity=2;serial=drops=0;rows=[]
    for op in word:
        observed=0
        if op in 'af':
            serial+=1
            if len(queue)==8:status=2;drops+=1
            elif op=='f':status=10;drops+=1
            else:
                status=0
                if len(queue)==capacity:capacity*=2
                queue.append(serial)
        else:
            if not queue:status=1
            else:status=0;observed=queue.pop() if op=='p' else queue.popleft();drops+=1
        rows.append(f'{status}:{observed}:{capacity}:{drops},')
    return ''.join(rows)+f'D{drops+len(queue)}'


def version_model(word):
    clock=current=serial=0;pins=[];rows=[]
    for op in word:
        n=99
        if op in 'pf':
            serial+=1
            if op=='f':status=10
            else:status=0;clock+=1;current=serial
        elif op=='s':
            if len(pins)==4:status=2
            else:status=0;pins.append(current)
        elif op=='r':
            status=0 if pins else 7
            if pins:n=pins[0]
        else:
            status=0 if pins else 7
            if pins:pins.pop(0)
        rows.append(f'{status}:{n}:{clock},')
    return ''.join(rows)


def predicate_model(x,y,k0,k1,v0,v1,strict,order):
    values=[x,y];changed=set();statuses=[None,None];keys=[k0,k1];proposed=[v0,v1]
    for i in [order,1-order]:
        observed={0,1} if strict else ({keys[i]} if keys[i]<2 else set())
        if observed&changed:statuses[i]=9
        else:
            statuses[i]=0
            if keys[i]<2:values[keys[i]]=proposed[i];changed.add(keys[i])
    return ' '.join(map(str,statuses+values))


def transaction_model(initial,r0,r1,w0,w1,word):
    values=[initial,initial];stamps=[0,0];clock=0;steps=[0,0];tx=[None,None];rows=[]
    for op in word:
        i=ord(op)-ord('a');step=steps[i];steps[i]+=1;status=0;observed=99
        if step==0:tx[i]={'active':True,'snapshot':clock,'read':None,'value':0,'write':None}
        else:
            t=tx[i]
            if not t['active']:status=5
            elif step==1:
                key=[r0,r1][i]
                if stamps[key]>t['snapshot']:status=9;t['active']=False
                else:observed=values[key];t['value']=observed;t['read']=key
            elif step==2:t['write']=[w0,w1][i]
            else:
                clock+=1
                if t['read'] is not None and stamps[t['read']]>t['snapshot']:status=9
                else:values[t['write']]=t['value']+1;stamps[t['write']]=clock
                t['active']=False
        rows.append(f'{status}:{observed},')
    return ''.join(rows)+':'.join(map(str,values+[clock]))


def stream_model(word):
    demand=count=ends=terminal_status=0;cancelled=terminal=False;rows=[];maximum=(1<<64)-1
    for op in word:
        if op=='c':cancelled=True;status=0
        elif cancelled or terminal:status=3
        elif op in 'ru':status=0;demand=min(maximum,demand+(1 if op=='r' else maximum))
        elif op=='e':
            if not demand:status=1
            else:status=0;count+=1;demand-=int(demand!=maximum)
        else:
            status=0;terminal=True;ends+=1;terminal_status={'z':5,'t':0,'f':10}[op]
        rows.append(f'{status}:{demand}:{count}:{ends}:{terminal_status},')
    return ''.join(rows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--driver', type=Path)
    args = parser.parse_args()
    if args.driver:
        subprocess.run([sys.executable,str(ROOT/'tools/verify_messaging_registration.py'),'--driver',
                        str(args.driver.parent/'followthrough/messaging-registration-oracle')],check=True)
        subprocess.run([sys.executable,str(ROOT/'tools/verify_messaging_actor_executor.py'),'--driver',
                        str(args.driver.parent/'followthrough/messaging-actor-executor')],check=True)
        subprocess.run([sys.executable,str(ROOT/'tools/verify_messaging_completion_winner.py'),'--driver',
                        str(args.driver.parent/'followthrough/messaging-completion-winner')],check=True)
    subprocess.run([sys.executable, str(ROOT/'tools/verify_messaging_fixtures.py')] +
                   (['--driver', str(args.driver)] if args.driver else []), check=True)
    fixture = json.loads((ROOT/'fixtures/messaging/nursery_follow.json').read_text(encoding='utf8'))
    for row in fixture['cases']:
        assert model(row['outer'], row['inner'], row['codes']) == row['expected'], row['id']
    inputs = []; expected = []
    for outer, a, b in itertools.product(range(3), repeat=3):
        for codes in itertools.product([-7, 0, 5], repeat=4):
            inputs.append(' '.join(map(str, [outer, a, b, *codes])))
            expected.append(model(outer, [a, b], codes))
    if args.driver:
        driver = args.driver.parent/'followthrough/messaging-nursery-follow'
        run = subprocess.run([str(driver), 'oracle'], input='\n'.join(inputs)+'\n',
                             text=True, capture_output=True, timeout=180, check=True)
        actual = [list(map(int, line.split())) for line in run.stdout.splitlines()]
        if actual != expected:
            for i, (x, y) in enumerate(zip(actual, expected)):
                if x != y: raise AssertionError((inputs[i], x, y))
            raise AssertionError(('result count', len(actual), len(expected)))
    words=[''.join(w) for n in range(6) for w in itertools.product('abrcdx',repeat=n)]
    words+=['aaabrcrcdr','aaabbxrr','ababrrrrcdrr']
    owned_expected=[owned_model(word) for word in words]
    fixtures=json.loads((ROOT/'fixtures/messaging/owned_follow.json').read_text(encoding='utf8'))
    for row in fixtures['channel_cases']:
        parts=owned_model(row['word']).split(',')
        assert [int(p.split(':')[0]) for p in parts[:-1]]==row['statuses']
        assert parts[-1]=='D'+str(row['drops_after_destroy'])
    if args.driver:
        run=subprocess.run([str(args.driver.parent/'followthrough/messaging-owned-follow'),'oracle'],
                           input='\n'.join(words)+'\n',text=True,capture_output=True,timeout=180,check=True)
        observed=run.stdout.splitlines()
        if observed!=owned_expected:
            for word,a,b in zip(words,observed,owned_expected):
                if a!=b: raise AssertionError((word,a,b))
            raise AssertionError(('owned result count',len(observed),len(owned_expected)))
    supervision=list(itertools.product(range(3),range(4),range(256)))
    if args.driver:
        run=subprocess.run([str(args.driver.parent/'followthrough/messaging-supervisor-follow'),'oracle'],
                           input='\n'.join(' '.join(map(str,row)) for row in supervision)+'\n',
                           text=True,capture_output=True,timeout=180,check=True)
        expected_supervision=[supervision_model(*row) for row in supervision]
        if run.stdout.splitlines()!=expected_supervision:
            for row,a,b in zip(supervision,run.stdout.splitlines(),expected_supervision):
                if a!=b:raise AssertionError((row,a,b))
            raise AssertionError('supervision result count')
    lifetime_words=[''.join(w) for n in range(7) for w in itertools.product('afps',repeat=n)]
    lifetime_words+=['aaaaaaaaappppppppp','aafappsaafsps','aaaaaaaafssppsspp']
    if args.driver:
        run=subprocess.run([str(args.driver.parent/'followthrough/messaging-lifetime-follow'),'oracle'],
                           input='\n'.join(lifetime_words)+'\n',text=True,capture_output=True,timeout=180,check=True)
        actual=run.stdout.splitlines();wanted=[lifetime_model(w) for w in lifetime_words]
        if actual!=wanted:
            for word,a,b in zip(lifetime_words,actual,wanted):
                if a!=b:raise AssertionError((word,a,b))
            raise AssertionError(('lifetime result count',len(actual),len(wanted)))
    version_words=[''.join(w) for n in range(6) for w in itertools.product('pfsrd',repeat=n)]
    version_words+=['spspspspsrrrrdddd','pppppppppppppppp','ssrsdrsddr','psfprdr']
    predicate_rows=list(itertools.product(range(3),range(3),range(3),range(3),range(2),range(2),range(2),range(2)))
    if args.driver:
        driver=args.driver.parent/'followthrough/messaging-versions-follow'
        for case,inputs,wanted in [
            ('owned-oracle',version_words,[version_model(w) for w in version_words]),
            ('predicate-oracle',[' '.join(map(str,row)) for row in predicate_rows],[predicate_model(*row) for row in predicate_rows])]:
            run=subprocess.run([str(driver),case],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=180,check=True)
            actual=run.stdout.splitlines()
            if actual!=wanted:
                for row,a,b in zip(inputs,actual,wanted):
                    if a!=b:raise AssertionError((case,row,a,b))
                raise AssertionError((case,'result count',len(actual),len(wanted)))
    schedules=[]
    for positions in itertools.combinations(range(8),4):
        schedules.append(''.join('a' if i in positions else 'b' for i in range(8)))
    transaction_rows=[(*numbers,word) for numbers in itertools.product(range(3),range(2),range(2),range(2),range(2)) for word in schedules]
    if args.driver:
        inputs=[' '.join(map(str,row)) for row in transaction_rows]
        run=subprocess.run([str(args.driver.parent/'followthrough/messaging-transactions-follow'),'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=180,check=True)
        actual=run.stdout.splitlines();wanted=[transaction_model(*row) for row in transaction_rows]
        if actual!=wanted:
            for row,a,b in zip(transaction_rows,actual,wanted):
                if a!=b:raise AssertionError((row,a,b))
            raise AssertionError(('transaction result count',len(actual),len(wanted)))
    stream_words=[''.join(w) for n in range(6) for w in itertools.product('ruzec tf'.replace(' ',''),repeat=n)]
    if args.driver:
        run=subprocess.run([str(args.driver.parent/'followthrough/messaging-stream-follow'),'oracle'],input='\n'.join(stream_words)+'\n',text=True,capture_output=True,timeout=180,check=True)
        actual=run.stdout.splitlines();wanted=[stream_model(w) for w in stream_words]
        if actual!=wanted:
            for word,a,b in zip(stream_words,actual,wanted):
                if a!=b:raise AssertionError((word,a,b))
            raise AssertionError(('stream result count',len(actual),len(wanted)))
    print(json.dumps({'nursery_model_cases':len(expected), 'compiled_cases':len(expected) if args.driver else 0,
                      'owned_model_cases':len(words),'owned_compiled_cases':len(words) if args.driver else 0,
                      'supervision_model_cases':len(supervision),'supervision_compiled_cases':len(supervision) if args.driver else 0,
                      'lifetime_model_cases':len(lifetime_words),'lifetime_compiled_cases':len(lifetime_words) if args.driver else 0,
                      'version_model_cases':len(version_words),'version_compiled_cases':len(version_words) if args.driver else 0,
                      'predicate_model_cases':len(predicate_rows),'predicate_compiled_cases':len(predicate_rows) if args.driver else 0,
                      'transaction_model_cases':len(transaction_rows),'transaction_compiled_cases':len(transaction_rows) if args.driver else 0,
                      'stream_model_cases':len(stream_words),'stream_compiled_cases':len(stream_words) if args.driver else 0,
                      'scope':'Actual native admissions, deterministic aggregation, finite owned-channel/deque histories and fallible supervision traces; not all schedules or fairness.'}))


if __name__ == '__main__':
    main()
