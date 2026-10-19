"""Independent transition specification; finish is an abstract quiescence event."""
import argparse,hashlib,itertools,json,subprocess
def expected(word):
    address=pending=cancel=notified=recovery=generation=releases=state=result=0;ticket=92;value=91;trace=''
    for op in word:
        status=0
        if op=='s':
            if pending:status=4
            else:generation+=1;ticket=generation;pending=1;cancel=notified=recovery=result=0;state=1
        elif op in 'ct':
            if not pending:status=1
            else:cancel=1;state=2;recovery=int(op=='t') or recovery
        elif op in 'nwx':
            if op=='w':status=1
            elif op=='x':status=3
            elif not pending or notified:status=1
            else:notified=result=1
        elif op=='f':
            if not pending:status=1
            elif not notified:status=4
            else:pending=cancel=recovery=0;releases+=1;state=3;value=42
        elif op=='r':
            if pending:status=4
            else:address=1-address
        trace+=','.join(map(str,[status,address,pending,cancel,notified,recovery,generation,releases,state,ticket,value,result]))+';'
    return trace
def run(driver):
    words=[''.join(x) for n in range(6) for x in itertools.product('sctnfrwx',repeat=n)];answers=[expected(w) for w in words];inputs=[w or '-' for w in words]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,histories=len(words),max_operations=5,alphabet=8,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Serialized abstract quiescence event; actual callback thread and join authority are tested separately, not a DMA hardware proof.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
