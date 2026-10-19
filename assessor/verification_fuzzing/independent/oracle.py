"""Independent finite records oracle; no target control-flow calls used."""
from pathlib import Path
import ctypes as C,itertools,random,struct,sys
class State(C.Structure):_fields_=[('present',C.c_ubyte),('value',C.c_ubyte*4)]
def decode(raw,initial):
 if len(raw)<3 or raw[:2]!=bytes([0xb6,1]) or raw[2]>4:return 0,initial
 records=[];tail=raw[3:]
 for _ in range(raw[2]):
  if len(tail)<2:return 0,initial
  tag,size=tail[:2];payload=tail[2:2+size];tail=tail[2+size:]
  if len(payload)!=size or (tag,size) not in [(1,2),(2,1)] or payload[0]>3:return 0,initial
  records.append((tag,payload))
 if tail or len({payload[0] for tag,payload in records})!=len(records):return 0,initial
 present,values=initial[0],list(initial[1:])
 for tag,payload in records:
  key=payload[0]
  if tag==1:present|=1<<key;values[key]=payload[1]
  else:present&=~(1<<key);values[key]=0
 return 1,bytes([present,*values])
def main():
 lib=C.CDLL(sys.argv[1]);lib.audit_packet.argtypes=[C.c_char_p,C.c_size_t,C.POINTER(State)];cases=0;detected=[]
 records=[bytes([1,2,k,v]) for k in range(4) for v in [0,1,255]]+[bytes([2,1,k]) for k in range(4)]
 packets=[bytes([0xb6,1,0])]
 for count in [1,2]:packets += [bytes([0xb6,1,count])+b''.join(parts) for parts in itertools.product(records,repeat=count)]
 for packet in list(packets):packets += [packet[:i] for i in range(len(packet))]+[packet+b'X']
 rng=random.Random(6108);packets += [rng.randbytes(rng.randrange(24)) for _ in range(10000)]
 packets += [bytes([0xb6,1,2,1,2,0,77,1,2,0,88]),bytes([0xb6,1,2,1,2,0,77,2,1,9])]
 for raw in packets:
  for initial in [bytes([0,0,0,0,0]),bytes([15,10,20,30,40])]:
   want,value=decode(raw,initial);s=State.from_buffer_copy(initial);got=lib.audit_packet(raw,len(raw),C.byref(s));actual=bytes(s);cases+=1
   if got!=want or actual!=value:
    detected.append(dict(input=raw.hex(),initial=initial.hex(),expected=[want,value.hex()],actual=[got,actual.hex()]));
    if len(detected)==3:break
  if len(detected)==3:break
 if detected:print('B06_AUDIT_FAIL',detected);return 1
 print('B06_AUDIT_PASS cases='+str(cases));return 0
if __name__=='__main__':raise SystemExit(main())
