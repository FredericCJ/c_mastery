"""Independent owner-size/accounting model for wrapper and upstream failure indices."""
import argparse,hashlib,itertools,json,subprocess
def expected(word):
    sizes=[0,0];attempts=successes=resizes=releases=invalid=live=cells=peak=calls=up_attempts=up_releases=up_live=up_peak=0;fail=up_fail=0;trace=''
    for op in word:
        status=0
        if op in 'ab':
            index='ab'.index(op)
            if sizes[index]:status=4
            else:
                attempts+=1
                if attempts==fail:status=5
                else:
                    calls+=1;up_attempts+=1
                    if up_attempts==up_fail:status=5
                    else:sizes[index]=index+1;live+=1;cells+=sizes[index];peak=max(peak,cells);successes+=1;up_live+=1;up_peak=max(up_peak,up_live)
        elif op=='r':
            if not sizes[0]:status=1
            else:
                attempts+=1
                if attempts==fail:status=5
                elif sizes[0]!=3:
                    calls+=1;up_attempts+=1
                    if up_attempts==up_fail:status=5
                    else:up_peak=max(up_peak,up_live+1);up_releases+=1;cells+=3-sizes[0];sizes[0]=3;peak=max(peak,cells);successes+=1;resizes+=1
        elif op in 'xy':
            index='xy'.index(op)
            if not sizes[index]:invalid+=1;status=1
            else:cells-=sizes[index];sizes[index]=0;live-=1;releases+=1;up_live-=1;up_releases+=1
        elif op=='f':fail=attempts+1
        elif op=='u':up_fail=up_attempts+1
        trace+=','.join(map(str,[status,*sizes,attempts,successes,resizes,releases,invalid,live,cells,peak,calls,up_attempts,up_releases,up_live,up_peak,11 if sizes[0] else 0]))+';'
    return trace
def run(driver):
    words=[''.join(x) for n in range(6) for x in itertools.product('abrxyfu',repeat=n)];answers=[expected(w) for w in words];inputs=[w or '-' for w in words];p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,histories=len(words),max_operations=5,alphabet=7,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Two logical owners, typed upstream copy-resize, no reporter reentry in this oracle; recursive formatting and counter extrema tested separately.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
