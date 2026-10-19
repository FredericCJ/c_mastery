"""Independent finite calendar enumeration; no division/multiplication count formula."""
import re
def check(text):
    rows=text.splitlines(); expected=16*9*49*2
    assert len(rows)==expected
    seen=set()
    for line in rows:
        d,p,n,skip,result,count,effects,nxt=map(int,line.split())
        key=(d,p,n,skip);assert key not in seen;seen.add(key)
        assert d in range(1,17) and p in range(9) and n in range(49) and skip in (0,1)
        if n<d:want=(0,0,0,0)
        elif p==0:want=(1,1,1,0)
        else:
            calendar=[];instant=d
            while instant<=n:calendar.append(instant);instant+=p
            want=(1,len(calendar),1 if skip else len(calendar),instant)
        assert (result,count,effects,nxt)==want,(key,(result,count,effects,nxt),want)
    return dict(cases=expected,independent_representation='enumerated absolute calendar instants')

def check_native(text):
    found=re.search(r'TIMER-PERIODIC:first=(\d+);period=(\d+);read_before=(\d+);dispatch=(\d+);count=(\d+);catchup=(\d+);skip=1;next=(\d+);',text)
    assert found,'missing complete native periodic record'
    first,p,before,after,count,effects,nxt=map(int,found.groups())
    assert p==2000000 and first<before<=after and count>=6 and effects==count
    at=first;low=high=0
    while at<=after:
        high+=1
        if at<=before:low+=1
        at+=p
    assert low<=count<=high
    cursor=first
    for _ in range(count):cursor+=p
    assert nxt==cursor
    return dict(counter=count,minimum=low,maximum=high,dispatch_minus_first=after-first)
