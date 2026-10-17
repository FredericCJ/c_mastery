"""Independent mathematical serial oracles; host ctypes is not a target dependency."""
import argparse, ctypes as C, itertools, json, random
from pathlib import Path
class Uart(C.Structure):_fields_=[('bits',C.c_uint8*12),('length',C.c_uint8),('brr',C.c_uint16),('divisor',C.c_uint32)]
class Sync(C.Structure):_fields_=[('level',C.c_uint8*16),('sample',C.c_uint8*16),('data',C.c_uint8*8),('edges',C.c_uint8)]
class I2c(C.Structure):_fields_=[('phase',C.c_uint8),('active',C.c_uint8),('value',C.c_uint8),('token',C.c_uint32)]
class Spi(C.Structure):_fields_=[('state',C.c_uint8),('count',C.c_uint8),('position',C.c_uint8),('cpol',C.c_uint8),('cpha',C.c_uint8),('rx',C.c_uint8*16),('token',C.c_uint32)]
class Rs(C.Structure):_fields_=[('token',C.c_uint32),('due',C.c_uint32),('last',C.c_uint32)]+[(x,C.c_uint8) for x in ['phase','count','position','rxcount','event_count','pending_event']]+[('tx',C.c_uint8*16),('rx',C.c_uint8*16),('events',C.c_uint8*32)]
class Can(C.Structure):_fields_=[('identifier',C.c_uint32),('extended',C.c_uint8),('remote',C.c_uint8),('length',C.c_uint8),('data',C.c_uint8*8)]
class Timing(C.Structure):_fields_=[('prescaler',C.c_uint16),('bs1',C.c_uint8),('bs2',C.c_uint8),('sjw',C.c_uint8),('btr',C.c_uint32),('rate',C.c_uint32)]
class Fifo(C.Structure):_fields_=[('items',C.c_uint32*3),('count',C.c_uint8),('locked',C.c_uint8),('overrun',C.c_uint8)]
class Fd(C.Structure):_fields_=[('length',C.c_uint8),('crc_width',C.c_uint8),('brs',C.c_uint8),('esi',C.c_uint8),('spi_bytes',C.c_uint16)]
def bits(v,n):return list(map(int,format(v,'0'+str(n)+'b')))
def arbitration(f):return bits(f.identifier>>18 if f.extended else f.identifier,11)+([1,1]+bits(f.identifier&0x3ffff,18)+[f.remote] if f.extended else [f.remote,0])
def can_frame(f):
 raw=[0]+arbitration(f)+([0,0] if f.extended else [0])+bits(f.length,4)
 if not f.remote:
  for x in f.data[:f.length]:raw+=bits(x,8)
 # Polynomial long division on an arbitrary-precision integer, independent of C LFSR.
 polynomial=int(''.join(map(str,raw)),2)<<15
 while polynomial.bit_length()>15:polynomial^=0xc599<<(polynomial.bit_length()-16)
 raw+=bits(polynomial,15)
 result=[]
 for bit in raw:
  result.append(bit)
  if len(result)>=5 and len(set(result[-5:]))==1:result.append(1-bit)
 return result+[1]*10
def main():
 p=argparse.ArgumentParser();p.add_argument('--library',type=Path,required=True);a=p.parse_args();lib=C.CDLL(str(a.library.resolve()));counts={x:0 for x in ['uart','sync','i2c','spi','electrical','rs485','can','bxcan','fd']}
 for width,parity,stop,over in itertools.product([7,8],range(3),[1,2],[0,1]):
  for value in range(1<<width):
   out=Uart();assert lib.sd_uart(42000000,115200,over,width,parity,stop,value,C.byref(out))==0
   data=[(value>>i)&1 for i in range(width)];expected=[0]+data+([] if not parity else [sum(data)%2^(parity==2)])+[1]*stop
   assert list(out.bits)[:out.length]==expected and out.divisor==365 and out.brr==(0x2d5 if over else 0x16d);counts['uart']+=1
 for mode,lbcl,value in itertools.product(range(4),range(2),range(256)):
  out=Sync();assert lib.sd_sync(mode>>1,mode&1,lbcl,1,1,0,value,C.byref(out))==0
  assert out.edges==14+2*lbcl and list(out.data)==[(value>>i)&1 for i in range(8)]
  assert list(out.sample)[:out.edges]==[int(i%2==mode%2) for i in range(out.edges)];counts['sync']+=1
 for failure_at,event in itertools.product(range(4),[2,4,5]):
  s=I2c();action=C.c_uint8();value=C.c_uint8();assert lib.sd_i2c_begin(C.byref(s),1,0x50,7,C.byref(action),C.byref(value))==0
  for i in range(failure_at):assert lib.sd_i2c_event(C.byref(s),1,1,0,C.byref(action),C.byref(value))==0
  rc=lib.sd_i2c_event(C.byref(s),1,event,0,C.byref(action),C.byref(value)); expected=12 if event==4 else 13 if event==5 else 1 if failure_at==3 else 11 if failure_at==1 else 10
  assert rc==expected
  if rc==1:assert s.active;continue
  assert not s.active and action.value==(5 if event==4 else 6);counts['i2c']+=1
 for scl,sda,release in itertools.product(range(2),range(2),range(11)):
  n=C.c_uint();rc=lib.sd_i2c_clear(scl,sda,release,C.byref(n));assert (rc,n.value)==((6,0) if not scl else (0,0) if sda else (0,release) if 1<=release<=9 else (6,9));counts['i2c']+=1
 for mode,peer in itertools.product(range(4),range(256)):
  l=(C.c_uint8*16)();pulses=(C.c_uint8*16)();mosi=(C.c_uint8*16)();rx=C.c_uint8();assert lib.sd_spi_edge(mode,0xa6,peer,l,pulses,mosi,C.byref(rx))==0
  assert rx.value==peer and list(l)==[(mode>>1)^((i+1)%2) for i in range(16)] and list(mosi)==[(0xa6>>(7-i//2))&1 for i in range(16)];counts['spi']+=1
 for count in range(1,17):
  s=Spi();assert lib.sd_spi_begin(C.byref(s),1,0,count)==0
  for i in range(count):assert lib.sd_spi_poll(C.byref(s),1,1,1,1,i)==3
  assert lib.sd_spi_poll(C.byref(s),1,1,0,0,0)==8 and list(s.rx)[:count]==list(range(count));counts['spi']+=1
 for kind,mv,swap in itertools.product(range(2),[-6000,-3001,-3000,-2999,-201,-200,-199,0,199,200,201,2999,3000,3001,6000],range(2)):
  d=C.c_int();assert lib.sd_electrical(kind,mv,1,1,swap,C.byref(d))==0
  expected=(1 if mv<=-3000 else 0 if mv>=3000 else -1) if kind==0 else (1 if mv>=200 else 0 if mv<=-200 else -1)
  assert d.value==(expected^swap if expected>=0 else -1);counts['electrical']+=1
 for count in range(1,17):
  s=Rs();data=(C.c_uint8*count)(*range(count));assert lib.sd_485_begin(C.byref(s),1,data,count,0)==0
  for i in range(count):assert lib.sd_485_poll(C.byref(s),1,600+i*100000,1,0)==3
  tc=600+count*100000;assert lib.sd_485_poll(C.byref(s),1,tc,1,1)==3
  assert s.due==tc+2200 and lib.sd_485_poll(C.byref(s),1,tc+2200,0,0)==8 and s.events[0]==4;counts['rs485']+=1
 rng=random.Random(38009)
 for extended,remote,length in itertools.product(range(2),range(2),range(9)):
  for case in range(20):
   f=Can(rng.randrange(1<<(29 if extended else 11)),extended,remote,length,(C.c_uint8*8)(*[rng.randrange(256) for _ in range(8)]));expected=can_frame(f);out=(C.c_uint8*160)();n=C.c_size_t(555)
   assert lib.sd_can_encode(C.byref(f),out,160,C.byref(n))==0 and list(out)[:n.value]==expected
   g=Can();assert lib.sd_can_decode(out,n.value,C.byref(g))==0 and (g.identifier,g.extended,g.remote,g.length)==(f.identifier,extended,remote,length)
   if not remote:assert list(g.data)[:length]==list(f.data)[:length]
   # Every payload/CRC corruption is tested through the public full decoder; ACK slot has deliberately different semantics.
   for pos in [0,n.value-1,n.value-3]:
    out[pos]^=1;before=bytes(g);assert lib.sd_can_decode(out,n.value,C.byref(g))!=0 and bytes(g)==before;out[pos]^=1
   counts['can']+=1
 for i in range(1000):
  a=Can(rng.randrange(2048),0,rng.randrange(2),0);b=Can(rng.randrange(1<<29),1,rng.randrange(2),0);w=C.c_int();assert lib.sd_can_arbitration(C.byref(a),C.byref(b),C.byref(w))==0;aa,bb=arbitration(a),arbitration(b);expected=next((-1 if x<y else 1 for x,y in zip(aa,bb) if x!=y),0);assert w.value==expected;counts['can']+=1
 for pre,bs1,bs2,sjw in itertools.product([1,6,1024],[1,11,16],[1,2,8],range(1,5)):
  t=Timing();rc=lib.sd_can_timing(42000000,pre,bs1,bs2,sjw,C.byref(t));assert rc==(0 if sjw<=bs2 else 1)
  if rc==0:assert t.rate==42000000//(pre*(1+bs1+bs2)) and t.btr==pre-1+(bs1-1)*65536+(bs2-1)*1048576+(sjw-1)*16777216
  counts['bxcan']+=1
 for locked in range(2):
  f=Fifo();f.locked=locked;model=[]
  for value in range(8):
   rc=lib.sd_can_push(C.byref(f),value)
   if len(model)<3:model.append(value);assert rc==0
   elif locked:assert rc==2
   else:model[-1]=value;assert rc==0
   assert list(f.items)[:f.count]==model;counts['bxcan']+=1
 lengths=list(range(9))+[12,16,20,24,32,48,64]
 for dlc,fdf,brs,esi,remote,capable in itertools.product(range(17),range(3),range(3),range(3),range(2),range(2)):
  f=Fd();C.memset(C.byref(f),0xa5,C.sizeof(f));old=bytes(f);rc=lib.sd_fd(dlc,fdf,brs,esi,remote,capable,C.byref(f));valid=dlc<=15 and fdf==1 and brs<=1 and esi<=1 and remote==0
  assert rc==(1 if not valid else 7 if not capable else 0)
  if rc:assert bytes(f)==old
  else:assert (f.length,f.crc_width,f.brs,f.esi,f.spi_bytes)==(lengths[dlc],17 if lengths[dlc]<=16 else 21,brs,esi,4+4*((lengths[dlc]+3)//4))
  counts['fd']+=1
 print(json.dumps(dict(status='passed',counts=counts,total=sum(counts.values()),limits='Original finite host models; no device, electrical or complete protocol-conformance execution.')))
if __name__=='__main__':main()
