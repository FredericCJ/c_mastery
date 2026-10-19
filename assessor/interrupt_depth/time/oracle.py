"""Independent integer/interval reasoning and full small counter histories."""
from pathlib import Path
from collections import Counter
import sys
counts=Counter();MAX=(1<<64)-1
for line in Path(sys.argv[1]).read_text().splitlines():
    fields=line.split();kind=fields[0];v=list(map(int,fields[1:]));counts[kind]+=1
    if kind=='S':
        lb,hb,start,step,status,value,reads=v;period=1<<(lb+hb)
        if 2*step>=(1<<lb):want=(4,999,0)
        else:
            points=[(start+i*step)%period for i in range(3)]
            # Accepted result must be the actual middle instant, not recomputed
            # by concatenating the implementation's chosen high/low fields.
            want=(0,points[1],3) if points[0]//(1<<lb)==points[2]//(1<<lb) else (3,999,3)
        assert (status,value,reads)==want,(line,want)
    elif kind=='C':
        anchor,begin,end,quantum,ppm,status,lower,upper,epoch=v
        low=begin*quantum;high=end*quantum
        expected_low=anchor+low-(low*ppm+999999)//1000000
        expected_high=anchor+high+(high*ppm+999999)//1000000
        want=(2,999,1000,9) if max(low,high,expected_low,expected_high)>MAX else (0,expected_low,expected_high,7)
        assert (status,lower,upper,epoch)==want,(line,want)
    elif kind=='G':
        before,after,bound,epoch,full,status,last,missing=v;old=MAX if full else 10;distance=(after-before)%(1<<32)
        if epoch!=1:want=(8,before,old)
        elif not distance:want=(5,before,old)
        elif distance>bound:want=(6,before,old)
        elif old+distance-1>MAX:want=(2,before,old)
        else:want=(7 if distance>1 else 0,after,old+distance-1)
        assert (status,last,missing)==want,(line,want)
    elif kind=='O':
        al,au,bl,bu,status,order=v
        # Set relation of all candidate physical instants, including endpoints.
        aa=set(range(al,au+1));bb=set(range(bl,bu+1))
        want=0 if all(a<b for a in aa for b in bb) else 1 if all(b<a for a in aa for b in bb) else 2
        assert (status,order)==(0,want),(line,want)
    elif kind=='E':
        lo,hi,mn,mx,status,lower,upper,epoch=v
        possibilities={time-delay for time in range(lo,hi+1) for delay in range(mn,mx+1)}
        want=(2,999,1000,9) if min(possibilities)<0 else (0,min(possibilities),max(possibilities),1)
        assert (status,lower,upper,epoch)==want,(line,want)
    else:raise AssertionError(kind)
assert counts==dict(S=2100,C=4096,G=400,O=1296,E=360),counts
print('XK_ORACLE_PASS snapshot=2100 calibration=4096 sequence=400 order=1296 event=360')
