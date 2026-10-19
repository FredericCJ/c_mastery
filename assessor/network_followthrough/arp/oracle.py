"""Independent interval/set interpretation of the local millisecond policy."""
from pathlib import Path
import sys
groups={};count=0
for line in Path(sys.argv[1]).read_text().splitlines():
    tag,b,step,mask,index,time,rc,accepted,requests,pending=line.split();assert tag=='TIME'
    b,step,mask,index,time,rc,accepted,requests,pending=map(int,[b,step,mask,index,time,rc,accepted,requests,pending])
    events=groups.setdefault((b,step,mask),[]);assert index==len(events) and time==index*step
    # Occupied intervals come from previous accepted requests, independent of
    # the C state bitset/cursor. Expiry is an absolute interval endpoint.
    served={next(i for i in range(3) if 1000*i<=t<1000*(i+1)) for t,r,a in events if r==0 and a}
    prior_expiry=any(r==5 for _,r,_ in events)
    if prior_expiry:expected=4
    elif time>=3000:expected=5
    elif any(lo<=time<hi for lo,hi in [(1000*i,1000*(i+1)) for i in served]):expected=4
    else:expected=0
    assert rc==expected and accepted==((index+mask)%4!=0)
    events.append((time,rc,accepted));assert requests==sum(r==0 and a for _,r,a in events) and pending==(time<3000)
    count+=1
assert set(groups)=={(b,s,m) for b in range(3) for s in [1,17,999,1000,2000,4000] for m in range(4)}
for (b,step,mask),rows in groups.items():assert len(rows)==4000//step+1
print(f'NA_ORACLE_PASS groups={len(groups)} events={count} independent_intervals refusal_absolute_expiry')
