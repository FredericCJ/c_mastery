"""Independent integer/bit derivations for the selected original observations."""
import itertools,json,pathlib,sys

def expected():
 rows={}
 rows['baud']=[]
 for clock,baud in itertools.product([48000000,80000000,125000000],[9600,115200,1000000]):
  # Rounded rational using doubled numerator, independently of C remainder test.
  q=(8*clock+baud)//(2*baud);rows['baud'].append([clock,baud,q//64,q%64])
 count=checksum=0
 for m,par,stop,p,h,l in itertools.product(range(2),range(3),[1,2],range(2),range(2),range(2)):
  payload=8+m-bool(par)
  for v in range(2**payload):
   bits=[(v//(2**i))%2 for i in range(payload)];word=[0]+bits+([] if not par else [(sum(bits)+(par==2))%2])+[1]*stop
   count+=1;checksum+=sum(word)
 rows['word']=[[count,checksum,11,16]];rows['capture']=[[4*4,160//16,(161+15)//16,18]]
 previous=None;sequence=[]
 for value in [-201,-120,0,-120,-201]:
  if value< -130:previous=0
  elif value> -100:previous=1
  sequence.append(previous)
 rows['receiver']=[[sum(sequence),2,3,1]]
 raw=[0]+[int(x) for x in f'{0x123:011b}']+[0,0,0]+[1,0,0,0]+[0]*64
 crc=0
 for b in raw:
  top=(crc>>14)^b;crc=(crc<<1)&0x7fff
  if top:crc^=0x4599
 raw += [(crc>>i)&1 for i in range(14,-1,-1)]
 stuffed=[];last=None;run=0
 for bit in raw:
  stuffed.append(bit);run=run+1 if bit==last else 1;last=bit
  if run==5:last=1-bit;stuffed.append(last);run=1
 n=len(stuffed)+10;rows['stream']=[[n,n+1,2,0x123]]
 rows['policy']=[[127,128,1,0]];rows['bank']=[[len([x for x in range(2048) if x&0x7f0==0x120]),2,9,1]]
 return rows

def main():
 t=pathlib.Path(sys.argv[1]).read_text();selected=sys.argv[2];wanted=expected();seen={}
 for line in t.splitlines():
  if line.startswith('SV_VALUES '):
   q=line.split();seen.setdefault(q[1],[]).append(list(map(int,q[2:])))
 keys=list(wanted) if selected=='all' else [selected]
 assert set(seen)==set(keys),(seen,keys)
 for key in keys:assert seen[key]==wanted[key],(key,seen[key],wanted[key])
 print(json.dumps(dict(status='pass',keys=keys,packets=sum(len(seen[k]) for k in keys),limits='Independent finite arithmetic/bit expectations; no physical observation or full conformance proof.')))
if __name__=='__main__':main()
