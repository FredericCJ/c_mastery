"""Independent declarative event/receipt/set oracle; no C source import."""
import argparse,json
OFFSET=14695981039346656037;PRIME=1099511628211;MASK=(1<<64)-1
def fold(h,fields):
 for x in fields:h=((h^x)*PRIME)&MASK
 return h
def cancellation():
 h=OFFSET;count=0
 # (phase,event) -> (status,new phase,retained ownership,report increment)
 table={(1,0):(2,2,1,0),(2,0):(2,2,1,0),(3,0):(2,3,1,0),
        (2,1):(2,1,1,0),(2,2):(2,3,1,0),(3,4):(0,5,0,1)}
 for phase in (1,2,3):table[phase,3]=(0,4,0,1)
 for length in range(7):
  for word in range(5**length):
   for generation in range(1,4):
    for event_generation in range(5):
     phase=1;owned=1;reports=generation-1;remaining=word
     for _ in range(length):
      event=remaining%5;remaining//=5
      status=5
      if owned and generation==event_generation:
       status,phase,owned,extra=table.get((phase,event),(5,phase,owned,0));reports+=extra
      h=fold(h,(status,generation,phase,owned,reports))
     count+=1
 return ['VAR_CANCEL',count,h]
def message():
 h=OFFSET;count=0
 for length in range(7):
  for word in range(3**length):
   data=[word//(3**i)%3 for i in range(length)]
   for quota in range(7):
    for lost in range(2):
     done=min(quota,length);total=sum(data[:done]);complete=int(done==length);out=total if complete and not lost else 999
     h=fold(h,(0 if complete and not lost else 2,out,done,total,done,complete))
     total=sum(data);h=fold(h,(2,out,length,total,length,1))
     h=fold(h,(0,total,length,total,length,1));count+=1
 return ['VAR_MESSAGE',count,h]
def device():
 h=OFFSET;count=0
 for status in range(16):
  for mask in range(32):
   for before in range(16):
    for after in range(16):
     surviving={i for i in range(4) if ((status|before)&(1<<i)) and (mask>=16 or not(mask&(1<<i)))}
     surviving|={i for i in range(4) if after&(1<<i)}
     value=sum(1<<i for i in surviving);valid=mask<16
     h=fold(h,(0 if valid else 1,value,int(valid),0,255-mask if valid else 0,value,0,1));count+=1
 return ['VAR_DEVICE',count,h]
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('kind',choices=['cancel','message','device']);p.add_argument('actual');a=p.parse_args()
 expected={'cancel':cancellation,'message':message,'device':device}[a.kind]();actual=open(a.actual).read().split();actual=[actual[0],*map(int,actual[1:])]
 if expected!=actual:raise SystemExit(json.dumps({'expected':expected,'actual':actual}))
 print('ORACLE_PASS',a.kind,expected[1],expected[2])
