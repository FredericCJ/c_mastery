"""Independent arithmetic, CRC-library, relational and state-model oracle against actual C."""
import argparse, binascii, ctypes as C, itertools, json, random
from pathlib import Path

class Interface(C.Structure): _fields_=[('protocol',C.c_uint16),('version',C.c_uint16),('shape',C.c_uint8),('ownership',C.c_uint8)]
class Frame(C.Structure): _fields_=[('da',C.c_uint8*6),('sa',C.c_uint8*6),('field',C.c_uint16),('payload_length',C.c_size_t),('stored',C.c_size_t),('payload',C.POINTER(C.c_uint8))]
class Entry(C.Structure): _fields_=[('mac',C.c_uint8*6),('port',C.c_uint8),('live',C.c_uint8),('seen',C.c_uint32)]
class Bridge(C.Structure): _fields_=[('entries',Entry*4),('now',C.c_uint32)]
class Descriptor(C.Structure): _fields_=[('bytes',C.POINTER(C.c_uint8)),('capacity',C.c_uint16),('status',C.c_uint32)]
class Rx(C.Structure): _fields_=[('generation',C.c_uint32),('held',C.c_uint8),('dropping',C.c_uint8)]
class Borrow(C.Structure): _fields_=[('first',C.POINTER(C.c_uint8)),('second',C.POINTER(C.c_uint8)),('first_length',C.c_uint16),('second_length',C.c_uint16),('token',C.c_uint32)]
class Slot(C.Structure): _fields_=[('bytes',C.c_uint8*1514),('phase',C.c_uint8),('length',C.c_uint16),('token',C.c_uint32),('head',C.c_uint32),('tail',C.c_uint32)]
class Tx(C.Structure): _fields_=[('slots',Slot*2)]
class Budget(C.Structure): _fields_=[('ip',C.c_uint32),('frame',C.c_uint32),('slot',C.c_uint32),('ns',C.c_uint32)]
def arr(data):return (C.c_uint8*len(data))(*data)
def main():
 p=argparse.ArgumentParser();p.add_argument('--library',type=Path,required=True);a=p.parse_args();lib=C.CDLL(str(a.library.resolve()));counts=dict.fromkeys(['layers','frame','bridge','phy','rx','tx','handoff'],0);rng=random.Random(390039)
 U=C.c_uint32;P=C.POINTER;Z=C.c_size_t
 signatures={'ed_wrap':[U,P(U),Z,P(U)],'ed_compatible':[Interface,Interface,P(C.c_uint)],'ed_frame_build':[P(C.c_uint8),P(C.c_uint8),C.c_uint16,P(C.c_uint8),Z,C.c_uint,P(C.c_uint8),Z,P(Z)],'ed_frame_parse':[P(C.c_uint8),Z,C.c_uint,P(Frame)],'ed_crc32':[P(C.c_uint8),Z,P(U)],'ed_bridge':[P(Bridge),P(C.c_uint8),P(C.c_uint8),C.c_uint,U,P(C.c_uint)],'ed_mdio_word':[U,C.c_uint,C.c_uint,C.c_uint,P(U),P(U)],'ed_rx_acquire':[P(Rx),P(Descriptor),Z,P(Borrow)],'ed_rx_release':[P(Rx),U],'ed_tx_prepare':[P(Tx),C.c_uint,U,P(C.c_uint8),Z],'ed_tx_tail':[P(Tx),C.c_uint,U],'ed_tx_publish':[P(Tx),C.c_uint,U,C.c_void_p,C.c_void_p],'ed_tx_abort':[P(Tx),C.c_uint,U,C.c_uint],'ed_budget':[U,C.c_uint,P(Budget)],'ed_capture':[P(U),Z,C.c_int32,C.c_uint,U,P(C.c_uint64),P(C.c_uint64)],'ed_first_failure':[C.c_uint,P(C.c_uint)]}
 for name,args in signatures.items():getattr(lib,name).argtypes=args;getattr(lib,name).restype=C.c_int
 for n in range(5):
  for initial in [0,12,2**32-1,2**32-21]:
   headers=[rng.randrange(40) for _ in range(n)];out=(U*4)(77,77,77,77);sizes=list(itertools.accumulate(headers,initial=initial))[1:];good=all(x<2**32 for x in sizes);assert lib.ed_wrap(initial,(U*n)(*headers),n,out)==(0 if good else 1);assert list(out)[:n]==(sizes if good else [77]*n);counts['layers']+=1
 for proto,version,shape,owner in itertools.product([1,2],[1,2],[0,1],[0,1]):
  yes=C.c_uint(77);assert lib.ed_compatible(Interface(1,1,0,0),Interface(proto,version,shape,owner),C.byref(yes))==0;assert yes.value==int((proto,version,shape,owner)==(1,1,0,0));counts['layers']+=1
 da=bytes.fromhex('020000000001');sa=bytes.fromhex('020000000002')
 for n in [0,1,3,45,46,47,1499,1500]+[rng.randrange(1501) for _ in range(128)]:
  data=rng.randbytes(n)
  for fcs in [0,1]:
   for field in [n,0x0800]:
    wire=da+sa+field.to_bytes(2,'big')+data+bytes(max(0,46-n));wire+=binascii.crc32(wire).to_bytes(4,'little') if fcs else b'';out=(C.c_uint8*1518)(*([165]*1518));written=Z(77);assert lib.ed_frame_build(arr(da),arr(sa),field,arr(data),n,fcs,out,1518,C.byref(written))==0;assert written.value==len(wire) and bytes(out)[:len(wire)]==wire;f=Frame();assert lib.ed_frame_parse(out,len(wire),fcs,C.byref(f))==0 and f.payload_length==(n if field<=1500 else max(n,46));counts['frame']+=1
    if fcs:
     for pos in [0,13,len(wire)-1]:
      bad=bytearray(wire);bad[pos]^=1;before=bytes(f);status=lib.ed_frame_parse(arr(bad),len(bad),1,C.byref(f));assert status!=0 and bytes(f)==before;counts['frame']+=1
 for _ in range(80):
  state=Bridge();model={};now=0
  for step in range(25):
   now+=rng.randrange(5);source=rng.randrange(1,7);destination=rng.randrange(1,8);ingress=rng.randrange(1,4);nextmodel={k:v for k,v in model.items() if now-v[1]<10};full=source not in nextmodel and len(nextmodel)==4;before=bytes(state);out=C.c_uint(77);rc=lib.ed_bridge(C.byref(state),arr([2,source,0,0,0,0]),arr([2,destination,0,0,0,0]),ingress,now,C.byref(out));assert rc==(2 if full else 0)
   if full:assert bytes(state)==before and out.value==77
   else:
    nextmodel[source]=(ingress,now);expected=(7^(1<<(ingress-1))) if destination not in nextmodel else (0 if nextmodel[destination][0]==ingress else 1<<(nextmodel[destination][0]-1));assert out.value==expected;model=nextmodel
   counts['bridge']+=1
 for hz,div,cr in [(20000000,16,2),(34999999,16,2),(35000000,26,3),(59999999,26,3),(60000000,42,0),(99999999,42,0),(100000000,62,1),(149999999,62,1),(150000000,102,4),(180000000,102,4)]:
  for phy,reg in itertools.product([0,1,31],[0,1,31]):
   w=U(77);d=U(88);assert lib.ed_mdio_word(hz,phy,reg,0,C.byref(w),C.byref(d))==0 and w.value==(phy<<11|reg<<6|cr<<2|1) and d.value==div;counts['phy']+=1
 a0=arr(bytes(1536));a1=arr(bytes(1536))
 for length in [64,65,100,1517,1518]:
  for split in [1,2,60,62,63]:
   if split>=length:continue
   s=Rx();d=(Descriptor*2)(Descriptor(a0,split,512),Descriptor(a1,length-split,(length<<16)|256));out=Borrow();assert lib.ed_rx_acquire(C.byref(s),d,2,C.byref(out))==0;assert out.first_length==min(length-4,split) and out.second_length==max(0,length-4-split) and out.token==1;assert lib.ed_rx_release(C.byref(s),1)==0 and lib.ed_rx_release(C.byref(s),1)==4;counts['rx']+=1
 for own,error,first,last in itertools.product([0,1],repeat=4):
  s=Rx();d=(Descriptor*1)(Descriptor(a0,64,(64<<16)|(own<<31)|(error<<15)|(first<<9)|(last<<8)));out=Borrow();rc=lib.ed_rx_acquire(C.byref(s),d,1,C.byref(out));expected=3 if own else 5 if not first else 3 if not last else 5 if error else 0;assert rc==expected;counts['rx']+=1
 callback=C.CFUNCTYPE(None,C.c_void_p)(lambda _:None)
 for stopped,tail in itertools.product([0,1],repeat=2):
  s=Tx();data=arr(bytes(range(60)));assert lib.ed_tx_prepare(C.byref(s),0,1,data,60)==0
  if tail:assert lib.ed_tx_tail(C.byref(s),0,1)==0
  assert lib.ed_tx_publish(C.byref(s),0,1,callback,None)==(0 if tail else 3);assert lib.ed_tx_abort(C.byref(s),0,1,stopped)==(0 if stopped else 3);assert s.slots[0].phase==(0 if stopped else 4);assert lib.ed_tx_prepare(C.byref(s),0,2,data,60)==(0 if stopped else 3);counts['tx']+=1
 for app in range(1461):
  for mb in [10,100]:
   b=Budget();assert lib.ed_budget(app,mb,C.byref(b))==0;ip=app+40;frame=14+max(46,ip)+4;assert (b.ip,b.frame,b.slot,b.ns)==(ip,frame,frame+20,(frame+20)*8*1000//mb);counts['handoff']+=1
 for offset in [-20,0,20]:
  for uncertainty in [0,1,20,101]:
   t=(U*2)(100,120);lo=(C.c_uint64*2)(77,77);hi=(C.c_uint64*2)(88,88);valid=100+offset>=uncertainty;assert lib.ed_capture(t,2,offset,1,uncertainty,lo,hi)==(0 if valid else 1);assert list(lo)==([100+offset-uncertainty,120+offset-uncertainty] if valid else [77,77]);assert list(hi)==([100+offset+uncertainty,120+offset+uncertainty] if valid else [88,88]);counts['handoff']+=1
 for mask in range(64):
  index=C.c_uint(77);assert lib.ed_first_failure(mask,C.byref(index))==0 and index.value==next((i for i in range(6) if not mask&(1<<i)),6);counts['handoff']+=1
 print(json.dumps(dict(passed=True,cases=counts,total=sum(counts.values()),independence='Host binascii CRC, integer arithmetic, dictionary bridge and independent state predicates; author-written, no independent human assessor claim.')))
if __name__=='__main__':main()
