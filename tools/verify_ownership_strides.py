"""Independent integer-set and staged slot-capacity oracle, without C overlap logic."""
import argparse
import hashlib
import itertools
import json
import subprocess
MAX=(1<<32)-1
def elements(extent,s):
    off,rows,cols,rs,cs=s
    if off>extent or rows>64 or cols>64 or rows*cols>64:return None
    values=[off+r*rs+c*cs for r in range(rows) for c in range(cols)]
    if any(v>=extent for v in values) or len(set(values))!=len(values):return None
    return values
def run(driver):
    inputs=[];answers=[];counts={}
    def add(row,answer):inputs.append(' '.join(map(str,row)));answers.append(' '.join(map(str,answer)))
    for extent in range(7):
        for off,n,off2,n2,w,w2 in itertools.product(range(extent+2),range(extent+2),range(extent+2),range(extent+2),range(2),range(2)):
            ok=off<=extent and off+n<=extent;ok2=off2<=extent and off2+n2<=extent
            first=set(range(off,off+n)) if ok else set();second=set(range(off2,off2+n2)) if ok2 else set()
            add([0,extent,off,n,w,off2,n2,w2],[0 if ok else 1,1 if not ok2 else 4 if first&second and (w or w2) else 0])
    counts['interval_cases']=len(inputs)
    for extent,off,rows,cols,rs,cs in itertools.product(range(13),range(6),range(4),range(4),range(5),range(5)):
        shape=(off,rows,cols,rs,cs);v=elements(extent,shape);add([1,extent,*shape],[1] if v is None else [0,len(v),*v])
    counts['shape_cases']=len(inputs)-sum(counts.values())
    representatives={}
    for s in itertools.product(range(6),range(3),range(3),range(5),range(5)):
        v=elements(8,s)
        if v is not None:representatives.setdefault(tuple(sorted(v)),s)
    shapes=list(representatives.values())+[(0,2,2,1,1),(8,1,1,0,0)]
    for a,b,w,w2 in itertools.product(shapes,shapes,range(2),range(2)):
        va,vb=elements(8,a),elements(8,b);conflict=va is not None and vb is not None and set(va)&set(vb) and (w or w2)
        add([2,8,*a,w,*b,w2],[1 if va is None else 0,1 if vb is None else 4 if conflict else 0])
    counts['intersection_cases']=len(inputs)-sum(counts.values())
    for shape in [(0,3,4,6,1),(1,2,3,7,2),(18,0,0,0,0),(2,1,4,0,3)]:
        for axis,cut,occupied,seed in itertools.product(range(2),range(6),range(4),[0,MAX-1,MAX]):
            extent=18;vals=elements(extent,shape);assert vals is not None
            row=[3,extent,*shape,axis,cut,occupied,seed]
            if seed==MAX:add(row,[2]);continue
            dimension=shape[1+axis]
            if cut>dimension:add(row,[0,1,1]);continue
            free=4-occupied-int(seed==MAX-1)
            if free<2:add(row,[0,2,1]);continue
            left=list(shape);right=list(shape);left[1+axis]=cut;right[1+axis]-=cut
            if right[1] and right[2]:right[0]+=cut*shape[3+axis]
            x,y=elements(extent,left),elements(extent,right);assert x is not None and y is not None and not set(x)&set(y) and set(x)|set(y)==set(vals)
            add(row,[0,0,1,len(x),*x,len(y),*y])
    counts['split_cases']=len(inputs)-sum(counts.values())
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers),(len(actual),len(answers))
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,**counts,unique_sets=len(representatives),total_cases=len(inputs),trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),limits='Finite integer sets do not prove arbitrary pointer lifetimes, concurrency or unbounded shape cardinality.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
