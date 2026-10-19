"""Independent polynomial long division and selected hardware transaction arithmetic."""
import json,pathlib,sys
LENGTHS=[0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64]
def bits(n,width):return [int(c) for c in f'{n:0{width}b}']
def polynomial_crc(message,width,initial):
 value=int(''.join(map(str,message)),2) if message else 0
 remainder=(value<<width)^(initial<<len(message));polynomial=0x3685b if width==17 else 0x302899
 while remainder.bit_length()>width:remainder^=polynomial<<(remainder.bit_length()-width-1)
 return remainder
def encode(identifier,dlc,data,esi=0,current=True):
 prefix=[0]+bits(identifier,11)+[0,0,1,0,1,esi]+bits(dlc,4)+[bit for value in data for bit in bits(value,8)]
 transmitted=[];inserted=0
 for at,value in enumerate(prefix):
  transmitted.append(value)
  if at+1<len(prefix) and len(transmitted)>=5 and len(set(transmitted[-5:]))==1:transmitted.append(1-value);inserted+=1
 width=17 if len(data)<=16 else 21;codes=[0,3,6,5,12,15,10,9];count=bits(codes[inserted%8],4) if current else []
 crc=polynomial_crc(transmitted+count,width,1<<(width-1) if current else 0)
 field=count+bits(crc,width);wire=transmitted+[1-transmitted[-1]]
 for at in range(0,len(field),4):
  chunk=field[at:at+4];wire+=chunk
  if len(chunk)==4:wire.append(1-chunk[-1])
 return wire+[1]*10,crc
a,ca=encode(0x123,0,[],1,True);b,cb=encode(0x123,0,[],1,False)
fingerprint=2166136261;largest=0;frames=0
for dlc,length in enumerate(LENGTHS):
 for pattern in range(4):
  data=[(0 if pattern==0 else 255 if pattern==1 else (15 if i%2 else 240) if pattern==2 else i*37+3)&255 for i in range(length)]
  wire,_=encode(0,dlc,data);largest=max(largest,len(wire));frames+=1
  for bit in wire:fingerprint=((fingerprint^bit)*16777619)&0xffffffff
packets={'admission':[16*2*2,16,65,64],'wire':[len(a),len(b),ca,cb],'wire_matrix':[frames,largest,fingerprint,709],'controller':[4+64,2+64,4,2],'tdc':[18+4,7,1,25]}
seen={}
for line in pathlib.Path(sys.argv[1]).read_text().splitlines():
 if line.startswith('OBS fd_'):
  p=line.split();key=p[1][3:];assert key not in seen;seen[key]=list(map(int,p[2:]));assert seen[key]==packets[key],(key,seen[key],packets[key])
expected=set(packets) if sys.argv[2]=='all' else {'wire','wire_matrix'} if sys.argv[2]=='wire' else {sys.argv[2]}
assert set(seen)==expected
print(json.dumps(dict(status='pass',packets=len(seen),keys=sorted(seen),limits='Polynomial long division, independent frame assembly and bounded trace arithmetic; no physical/complete-MAC conformance proof.')))
