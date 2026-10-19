"""Independent GF(2) polynomial long division, not the C byte recurrence."""
import sys
from pathlib import Path
rows=Path(sys.argv[1]).read_text().splitlines()
assert len(rows)==65536
seen=set()
for row in rows:
 fields=row.split();assert fields[0]=='F';a,b,*wire=map(int,fields[1:]);assert (a,b) not in seen;seen.add((a,b))
 dividend=(a<<16)|(b<<8)
 for power in range(23,7,-1):
  if dividend&(1<<power):dividend^=0x107<<(power-8)
 expected=[0xf0]
 for x in (a,b,dividend):expected += [0x10|(x>>4),0x10|(x&15)]
 expected += [0xf1]
 assert wire==expected,(a,b,wire,expected)
print('PL_ORACLE_PASS 65536 polynomial remainders and canonical encodings')
