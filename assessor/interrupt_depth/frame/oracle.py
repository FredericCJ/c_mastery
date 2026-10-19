"""Independent bit-string partition, not a second copy of C shift arithmetic."""
from pathlib import Path
import sys
all_rows=Path(sys.argv[1]).read_text().splitlines();rows=[s for s in all_rows if not s.startswith('P ')];selection=[s for s in all_rows if s.startswith('P ')];assert len(rows)==12288 and len(selection)==41472
seen=set()
for line in rows:
 b,g,r,e,group,sub,normal,opposite=map(int,line.split());key=b,g,r
 assert key not in seen;seen.add(key)
 binary=f'{r:08b}';implemented=binary[:b];boundary=min(b,7-g)
 expected_group=int(implemented[:boundary] or '0',2)
 expected_sub=int(implemented[boundary:] or '0',2)
 encoded=int(implemented+'0'*(8-b),2)
 flipped=int(''.join('1' if x=='0' else '0' for x in implemented)+'0'*(8-b),2)
 assert (e,group,sub,normal,opposite)==(encoded,expected_group,expected_sub,flipped,encoded),key
assert seen=={(b,g,r) for b in range(3,9) for g in range(8) for r in range(256)}
seen=set()
for line in selection:
 b,g,raw,current,mask,state,rc,chosen=map(int,line.split()[1:]);key=b,g,raw,current,mask,state
 assert key not in seen;seen.add(key)
 def parts(value):
  logical=f'{value:08b}'[:b];boundary=min(b,7-g)
  return int(logical[:boundary] or '0',2),int(logical+'0'*(8-b),2)
 irqs=[(18,raw,bool(state&1),bool(state&2),bool(state&4)),(17,0x60,True,True,False),(16,0x60,True,True,False)]
 active=[parts(current)[0]] if current>=0 else []
 active += [parts(prio)[0] for _,prio,_,_,busy in irqs if busy]
 eligible=[(parts(prio)[1],ident) for ident,prio,enabled,pending,busy in irqs if enabled and pending and not busy and not(state&8) and (not active or parts(prio)[0]<min(active)) and (not parts(mask)[1] or parts(prio)[0]<parts(mask)[0])]
 expected=(0,min(eligible)[1]) if eligible else (6,999)
 assert (rc,chosen)==expected,key
assert seen=={(b,g,r,c,m,s) for b in range(3,9) for g in range(8) for r in [0,31,80,96,144,255] for c in [-1,96,144] for m in [0,64,128] for s in range(16)}
print('XF_ORACLE_PASS priority=12288 selection=41472')
