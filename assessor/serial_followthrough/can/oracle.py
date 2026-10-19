"""Independent published-packet arithmetic and bounded state expectations."""
import json,pathlib,sys
packets={
 'arbitration':[2047,0x123,11,29],
 'codec':[9*4,9*4*2,0,8],
 'stream':[124,(124+1)*(124+2)//2,1187,1],
 'fault':[120+1+8,16*8,32*8,128*11],
 'timing':[42000000//(6*(1+11+2)),5|(10<<16)|(1<<20),31,6],
 'fifo':[2*3,30,40,1],
 'mailbox':[1,2,2,2**32-1],
 'recovery':[3,1,128,11],
 'port':[0x40006400,32,1,5|(10<<16)|(1<<20)]}
# Derive stream length and first-boundary count independently from the CRC
# recurrence and mandatory stuffing of the all-zero8-byte standard frame.
raw=[0]*19+[0]*64;raw[15:19]=[1,0,0,0];crc=0
for bit in raw:
 feedback=((crc>>14)&1)^bit;crc=(crc<<1)&0x7fff
 if feedback:crc^=0x4599
raw += [(crc>>i)&1 for i in range(14,-1,-1)]
wire=[];last=None;run=0;boundaries=[]
for bit in raw:
 wire.append(bit);run=run+1 if bit==last else 1;last=bit
 if run==5:boundaries.append(len(wire));last=1-bit;wire.append(last);run=1
n=len(wire)+10
packets['stream']=[n,(n+1)*(n+2)//2,sum(n-i+1 for i in boundaries),1]
# Independent exhaustive exact-rate search counts every legal partition.
count=0
for pre in range(1,1025):
 for bs1 in range(1,17):
  for bs2 in range(1,9):count+=48000000==500000*pre*(1+bs1+bs2)
packets['timing'][2]=count
lines=pathlib.Path(sys.argv[1]).read_text().splitlines();seen={}
for line in lines:
 if line.startswith('OBS can_'):
  parts=line.split();key=parts[1][4:];assert key not in seen;seen[key]=list(map(int,parts[2:]));assert seen[key]==packets[key],(key,seen[key],packets[key])
expected=set(packets) if sys.argv[2]=='all' else {sys.argv[2]};assert set(seen)==expected
print(json.dumps(dict(status='pass',packets=len(seen),keys=sorted(seen),limits='Independent bounded arithmetic/trace expectations; no physical or exhaustive implementation proof.')))
