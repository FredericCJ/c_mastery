"""Actual C compared with independent timestamp arithmetic and receipt dictionaries."""
import argparse,ctypes as C,json,random
from pathlib import Path
U=C.c_uint32;B=C.c_uint8;H=C.c_uint16;P=C.POINTER
class Grant(C.Structure):_fields_=[(x,U) for x in ['last','turn_due','earliest','start','end','generation']]+[('phase',B)]
class Fragment(C.Structure):_fields_=[('id',H)]+[(x,B) for x in ['source','destination','index','count','credit','type']]+[('data',B*24),('length',B)]
class Sender(C.Structure):_fields_=[('bytes',B*120)]+[(x,B) for x in ['length','index','count','source','destination','active','waiting','retries','credit','blocked','offered']]+[('id',H)]+[(x,U) for x in ['last','due','blocked_since','offer_token']]+[('offer',Fragment)]
class Receipt(C.Structure):_fields_=[('checkpoint',H),('request',B*80),('response',B*80),('request_length',B),('response_length',B)]
class Session(C.Structure):_fields_=[('session',H),('epoch',H)]+[(x,B) for x in ['open','holder','token_pending','pending_direction','pending_length']]+[('pending',B*80),('pending_checkpoint',H),('generation',U),('pending_token',U),('receipt',Receipt*2)]
def arr(x):return (B*len(x))(*x)
def main():
 p=argparse.ArgumentParser();p.add_argument('--library',type=Path,required=True);args=p.parse_args();lib=C.CDLL(str(args.library.resolve()));rng=random.Random(400040);counts=dict(grant=0,offer=0,session=0)
 sig={'e7_grant_begin':[P(Grant),U,P(U)],'e7_grant_first':[P(Grant),U,U],'e7_grant_finish':[P(Grant),U,U],'e7_grant_tick':[P(Grant),U],'e7_send_begin':[P(Sender),H,C.c_uint,C.c_uint,P(B),C.c_size_t,U],'e7_send_offer':[P(Sender),U,P(Fragment),P(U)],'e7_send_accept':[P(Sender),U,C.c_uint,U],'e7_session_open':[P(Session),H,H,C.c_uint,C.c_uint],'e7_session_prepare':[P(Session),C.c_uint,H,H,H,P(B),C.c_size_t,P(U),P(B),C.c_size_t,P(C.c_size_t)],'e7_session_commit':[P(Session),U,C.c_uint,P(B),C.c_size_t],'e7_token_begin':[P(Session),C.c_uint,C.c_uint],'e7_token_complete':[P(Session),C.c_uint,C.c_uint]}
 for name,types in sig.items():getattr(lib,name).argtypes=types;getattr(lib,name).restype=C.c_int
 for start in [0,1,10000,2**32-25001]:
  for delay in [199,200,1500,2200,2201]:
   s=Grant();token=U();assert lib.e7_grant_begin(C.byref(s),start,C.byref(token))==0;rc=lib.e7_grant_first(C.byref(s),token,start+delay);expected=2 if delay<200 else 10 if delay>2200 else 0;assert rc==expected
   if not rc:
    for duration in [0,100,9999,10000,10001]:
     clone=Grant.from_buffer_copy(s);assert lib.e7_grant_finish(C.byref(clone),token,start+delay+duration)==(11 if duration>10000 else 0);counts['grant']+=1
   counts['grant']+=1
 for _ in range(128):
  n=rng.randrange(1,121);data=rng.randbytes(n);s=Sender();assert lib.e7_send_begin(C.byref(s),1,7,8,arr(data),n,0)==0;now=0;f=Fragment();token=U()
  for j in range(rng.randrange(1,8)):
   now+=rng.randrange(0,100);assert lib.e7_send_offer(C.byref(s),now,C.byref(f),C.byref(token))==0;assert f.length==min(n,24) and bytes(f.data)[:f.length]==data[:24] and f.count==(n+23)//24;assert lib.e7_send_accept(C.byref(s),token,0,now)==2;assert not s.waiting and not s.retries and s.due==0;counts['offer']+=1
  first=now+5;assert lib.e7_send_offer(C.byref(s),first,C.byref(f),C.byref(token))==0 and lib.e7_send_accept(C.byref(s),token,1,first)==0;assert s.due==first+100
  for retry in range(1,4):
   now=first+100*retry;assert lib.e7_send_offer(C.byref(s),now-1,C.byref(f),C.byref(token))==8;assert lib.e7_send_offer(C.byref(s),now,C.byref(f),C.byref(token))==0 and lib.e7_send_accept(C.byref(s),token,1,now)==0;assert s.retries==retry and bytes(f.data)[:f.length]==data[:24];counts['offer']+=1
  assert lib.e7_send_offer(C.byref(s),first+400,C.byref(f),C.byref(token))==5 and not s.active;counts['offer']+=1
 for _ in range(40):
  s=Session();assert lib.e7_session_open(C.byref(s),1,1,1,1)==0;receipts={17:(0,b'',b''),51:(0,b'',b'')};holder=17
  for step in range(12):
   cp=receipts[holder][0]+1;data=rng.randbytes(rng.randrange(1,81));reply=rng.randbytes(rng.randrange(81));token=U(77);cache=(B*80)(*([165]*80));length=C.c_size_t(77);assert lib.e7_session_prepare(C.byref(s),holder,1,1,cp,arr(data),len(data),C.byref(token),cache,80,C.byref(length))==0;assert s.receipt[0].checkpoint==receipts[17][0] and s.receipt[1].checkpoint==receipts[51][0]
   if step%4==0:
    assert lib.e7_session_commit(C.byref(s),token,0,None,0)==0;assert lib.e7_session_prepare(C.byref(s),holder,1,1,cp,arr(data),len(data),C.byref(token),cache,80,C.byref(length))==0
   assert lib.e7_session_commit(C.byref(s),token,1,arr(reply),len(reply))==0;receipts[holder]=(cp,data,reply);assert lib.e7_session_prepare(C.byref(s),holder,1,1,cp,arr(data),len(data),C.byref(token),cache,80,C.byref(length))==7 and token.value==0 and bytes(cache)[:length.value]==reply
   other=51 if holder==17 else 17;assert lib.e7_token_begin(C.byref(s),holder,other)==0;assert lib.e7_token_complete(C.byref(s),other,0)==2 and s.holder==holder;assert lib.e7_token_complete(C.byref(s),other,1)==0;holder=other;counts['session']+=1
 print(json.dumps(dict(passed=True,cases=counts,total=sum(counts.values()),independence='Unbounded timestamp/fragment arithmetic and per-direction Python dictionary expectations; same-author oracle, not independent human review.')))
if __name__=='__main__':main()
