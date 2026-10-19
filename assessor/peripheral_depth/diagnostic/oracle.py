"""Independent arbitrary-integer/struct wire oracle for all16-bit sequences."""
from pathlib import Path
import struct,sys
rows=Path(sys.argv[1]).read_text().splitlines()
assert len(rows)==65536
for seq,line in enumerate(rows):
 p=line.split();length=seq%33
 if not seq:assert p==['0','0','1','-'];continue
 expected=struct.pack('!HH',seq,length)+bytes((seq+i*17)%256 for i in range(length))
 assert p==[str(seq),str(length),'0',expected.hex(),'0',str(seq),str(length)],(seq,p)
print('DG_ORACLE_PASS sequences=65536 lengths=33')
