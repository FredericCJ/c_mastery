"""Bind architectural observations to the selected QEMU trace, not board cycles."""
from pathlib import Path
import sys
text=Path(sys.argv[1]).read_text();interrupts=Path(sys.argv[2]).read_text();callee=int(sys.argv[3]);rows=[list(map(lambda x:int(x,16),s.split()[1:])) for s in text.splitlines() if s.startswith('XF_TRACE ')]
assert len(rows)==16 and 'XF_TARGET_PASS' in text
seen=set()
for group,fp,lazy,pad,a,b,c,asp,bsp,csp,before,after,fpcar in rows:
 assert (group,fp,lazy,pad) not in seen;seen.add((group,fp,lazy,pad))
 assert a==c==(0xffffffed if fp else 0xfffffffd) and asp==csp and asp%8==0
 assert before==fp*lazy and after==(0 if callee else before)
 if before:assert fpcar==asp+32
 if group==3:assert b==(0xffffffe1 if callee else 0xfffffff1) and bsp!=asp
 else:assert b==a and bsp==asp
assert seen=={(g,f,l,p) for g in [3,6] for f in [0,1] for l in [0,1] for p in [0,1]}
assert interrupts.count('...tailchaining to pending exception')==24
for n in [16,17,18]:assert interrupts.count('...taking pending nonsecure exception '+str(n)+'\n')==16
assert 'XF_UNEXPECTED_FAULT' not in text
print('XF_TARGET_ORACLE_PASS cases=16 tailchains=24')
