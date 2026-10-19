"""Independent rational timing and bit-at-a-time CRC fixture expectations."""
import json,pathlib,sys
body=bytes([1,0,1,2,4,1,0x7e,0x7d,2]);crc=0xffff
for b in body:
 crc^=b<<8
 for _ in range(8):crc=((crc<<1)^0x1021 if crc&0x8000 else crc<<1)&0xffff
wire=bytearray([0x7e])
for b in body+crc.to_bytes(2,'big'):
 wire.extend([0x7d,b^0x20] if b in [0x7e,0x7d] else [b])
wire.append(0x7e)
assert len(wire)==15
expected={'config':[16+16,11000,650,32//8],
 'timing':[600+2*10*10_000,600+2*10*10_000+200+2000,2,2**31],
 'events':[2,3,1,8],'cancellation':[1,16,2,5000+200+2000],
 'link':[len(wire),len(wire)+1,1,0],'port':[1<<26,16_000_000//100_000,201+1+2,6]}
lines=pathlib.Path(sys.argv[1]).read_text().splitlines();rows={}
for line in lines:
 if line.startswith('OBS rs485_'):
  words=line.split();key=words[1].removeprefix('rs485_');assert key not in rows;rows[key]=list(map(int,words[2:]))
want=expected if sys.argv[2]=='all' else {sys.argv[2]:expected[sys.argv[2]]}
assert rows==want,(rows,want)
print(json.dumps(dict(status='pass',packets=len(rows),keys=list(rows),crc=crc,wire=list(wire))))
