"""Independent logical lists plus separate sharing/accounting for legal COW traces."""
import argparse,hashlib,json,subprocess
def histories():
    result=[]
    def visit(word,values,groups,next_group,attempts,trace):
        result.append((word or '-',trace))
        if len(word)==5:return
        for op in range(12):
            v=[None if x is None else list(x) for x in values];g=list(groups);fresh=next_group;count=attempts;status=0
            if op<3:
                a,b=op,(op+1)%3
                if v[a] is None or v[b] is not None:continue
                v[b]=list(v[a]);g[b]=g[a]
            elif op<6:
                a=op-3
                if v[a] is None:continue
                v[a]=None;g[a]=None
            else:
                a=(op-6)%3
                if v[a] is None:continue
                if g.count(g[a])>1:
                    count+=1
                    if op>=9:status=5
                    else:g[a]=fresh;fresh+=1
                if status==0:v[a][0]=7
            live=len({x for x in g if x is not None})
            state=str(status)+':'+''.join('-;' if x is None else ','.join(map(str,x))+';' for x in v)+f'{live},{count}|'
            visit(word+chr(ord('a')+op),v,g,fresh,count,trace+state)
    visit('',[[1,2],None,None],[0,None,None],1,1,'');return result
def run(driver):
    rows=histories();inputs=[r[0] for r in rows];answers=[r[1] for r in rows];p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,histories=len(rows),max_operations=5,owners=3,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Scalar logical value and independent sharing/accounting model; resource-clone failure and concurrent snapshots separately exercised.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
