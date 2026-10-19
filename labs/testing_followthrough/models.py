"""Independent finite relations: no native instrumentation or physical inference."""
import hashlib,itertools,json
from fractions import Fraction
from pathlib import Path
def decisions(changed=False):
 rows=[]
 for bits in itertools.product((False,True),repeat=3 if changed else 2):
  a,b=bits[:2];trace=['A'];ab=False
  if a:trace.append('B');ab=b
  if changed:
   if not ab:trace.append('C')
   value=ab or bits[2]
  else:value=ab
  rows.append({'inputs':list(bits),'evaluated':trace,'decision':value})
 pairs={}
 for k,key in enumerate('ABC'[:3 if changed else 2]):
  pairs[key]=[[i,j] for i,x in enumerate(rows) for j,y in enumerate(rows) if i<j and x['decision']!=y['decision'] and x['inputs'][k]!=y['inputs'][k] and all(x['inputs'][h]==y['inputs'][h] for h in range(len(bits)) if h!=k)]
 return rows,pairs
def main():
 rows,pairs=decisions();assert pairs=={'A':[[1,3]],'B':[[2,3]]};assert rows[1]['evaluated']==['A']
 changed,cp=decisions(True);assert all(cp[k] for k in 'ABC')
 # Domain expansion does NOT break these two integer predicates: both exclude5.
 equivalence={str(top):all((n<5)==(n<=4) for n in range(top+1)) for top in (4,5)}
 assert all(equivalence.values()) and (4<4)!=(4<=4)
 memory=[];live=False;initialized=set();extent=0
 for event in [('allocate',4),('branch_read',1),('write',0),('write',4),('free',),('read',0)]:
  op=event[0]
  if op=='allocate':extent=event[1];live=True
  elif op=='free':live=False
  elif not live:memory.append('use-after-free')
  elif event[1]>=extent:memory.append('out-of-bounds')
  elif op=='write':initialized.add(event[1])
  elif event[1] not in initialized:memory.append('uninitialized-use')
 assert memory==['uninitialized-use','out-of-bounds','use-after-free']
 digest=hashlib.sha256(b'abc').hexdigest();assert digest=='ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad' and digest!='0'*64
 payload=bytes.fromhex('01021234abcd');correct=sum(int.from_bytes(payload[i:i+2],'big') for i in (2,4));native_le=sum(int.from_bytes(payload[i:i+2],'little') for i in (2,4));assert correct==48641 and native_le!=correct
 saved=bytes([0,1]);gen1=lambda seed:bytes([seed&255,1]);gen2=lambda seed:bytes([seed&255,2]);assert gen1(0)!=gen2(0) and saved==gen1(0)
 # Example uncertainty calculation, explicitly not a measured physical capture.
 period=Fraction(104,1000000);q=Fraction(1,2000000);eps=Fraction(1,10000)
 baud_interval=[1/((period+q)*(1+eps)),1/((period-q)*(1-eps))]
 print(json.dumps({'A_and_B':rows,'pairs':pairs,'changed':changed,'changed_pairs':cp,'equivalence':equivalence,'memory_labels':memory,'abc_sha256':digest,'wire_sum':correct,'wrong_native_little_sum':native_le,'illustrative_baud_interval':[float(x) for x in baud_interval],'unmeasured_timebase':'Unknown calibration means no defensible finite accuracy bound; example assumes100ppm plus0.5us quantization.','authority':{'host':['grammar','FIFO','ownership'],'emulator':['identified modeled core exception behavior'],'physical_required':['routing','baud','ADC accuracy']}}))
 print('MODELS_PASS:mcdc,short-circuit,equivalence,history,hash,endian,generator,uncertainty')
if __name__=='__main__':main()
