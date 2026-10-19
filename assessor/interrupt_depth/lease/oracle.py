"""Symbolic ownership/event oracle, independent of the C predicate ordering."""
import sys
from pathlib import Path
lines=Path(sys.argv[1]).read_text().splitlines();assert len(lines)==26244
for word in range(6561):
    # CPU, DEVICE, STOPPING, SERVICE, QUARANTINE are semantic labels here.
    state='device';epoch=1;generation=1;terminal=complete=possible=quiet=0;count=0
    for step in range(4):
        event=(word//(9**step))%9;status=0
        if event in (7,8):
            if state!='quarantine':status=2
            elif event==7:status=2
            else:epoch+=1;generation=0;state='cpu';terminal=complete=possible=count=0;quiet=1
        elif event==6:state='quarantine';quiet=0
        elif event==5 or epoch!=1 or generation!=1 or state in ('cpu','quarantine'):status=3
        elif event==0:
            if state in ('stopping','service'):status=4
            else:state='stopping'
        elif event==4:
            if state!='service':status=2
            else:state='cpu'
        elif state=='service':
            wanted=2 if event==2 else 1
            status=4 if terminal==wanted or (terminal==1 and wanted==2) else 3
        elif event==2 and state!='stopping':status=1
        elif event==3:status=2
        else:
            state='service';terminal=2 if event==2 else 1;count=1
            complete=possible=8 if event==2 else 32;quiet=1
        phase={'quarantine':0,'cpu':1,'device':2,'stopping':3,'service':4}[state]
        expected=[word,step,event,status,epoch,generation,phase,terminal,count,complete,possible,quiet]
        actual=list(map(int,lines[word*4+step].split()))
        assert actual==expected,(actual,expected)
print('XD_ORACLE_PASS words=6561 transitions=26244')
