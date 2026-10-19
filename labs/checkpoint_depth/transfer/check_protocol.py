"""Whole-buffer zlib oracle versus original streaming C parser and refusal schedules."""
import json,random,subprocess,sys,zlib

def frame(version,sequence,payload,stamp=None):
 header=bytes((version,))+sequence.to_bytes(2,'little')+len(payload).to_bytes(2,'little')
 if version==2:header+=bytes((stamp is not None,))+(stamp.to_bytes(4,'little') if stamp is not None else b'')
 body=header+payload
 return b'\xa5'+body+zlib.crc32(body).to_bytes(4,'little')

def oracle(data,limit):
 result=[];i=0
 while i<len(data):
  if data[i]!=0xa5:i+=1;continue
  if len(data)-i<2:break
  version=data[i+1]
  if version not in (1,2):i+=1;continue
  if len(data)-i<6:break
  n=int.from_bytes(data[i+4:i+6],'little')
  if n>limit:i+=1;continue
  header=6;stamp=0
  if version==2:
   if len(data)-i<7:break
   flags=data[i+6]
   if flags not in (0,1):i+=1;continue
   header=7+4*flags
   if len(data)-i<header:break
   if flags:stamp=int.from_bytes(data[i+7:i+11],'little')
  total=header+n+4
  if len(data)-i<total:break
  if zlib.crc32(data[i+1:i+total-4])!=int.from_bytes(data[i+total-4:i+total],'little'):i+=1;continue
  sequence=int.from_bytes(data[i+2:i+4],'little');payload=data[i+header:i+header+n]
  result.append(f'{version}:{sequence}:{stamp}:{payload.hex()}');i+=total
 return '\n'.join(result)+('\n' if result else '')

def run(executable,limit):
 rng=random.Random(0x435058);known=frame(1,258,b'\0A\xff');v2=frame(2,259,b'abc',0x11223344)
 streams=[b'',known,v2,known+v2,frame(2,0,b''),frame(2,65535,bytes(range(limit)),7),b'\xa5'*256]
 for cut in range(len(v2)):streams.append(v2[:cut])
 for _ in range(24):
  payload=rng.randbytes(rng.randrange(limit+1));wire=bytearray(frame(2,rng.randrange(65536),payload,rng.randrange(2**32)))
  if rng.randrange(2):wire[rng.randrange(len(wire))]^=1
  stream=rng.randbytes(rng.randrange(8))+bytes(wire)+known+v2
  streams.append(stream[:256])
 # A complete invalid candidate with a valid nested v1 frame; no whole-candidate skip.
 nested=bytearray(frame(1,8,bytes(limit)));at=6
 if limit>=len(known):nested[at:at+len(known)]=known
 nested[-1]^=1;streams.append(bytes(nested)+v2)
 comparisons=0
 for data in streams:
  expected=oracle(data,limit)
  decisions=''.join(str((byte>>i)&1) for byte in data[:16] for i in range(8))
  for chunk in (1,7,256):
   for capacity in (1,3):
    for schedule in ('0'*80,decisions):
     completed=subprocess.run([executable,'oracle',data.hex(),str(chunk),str(limit),str(capacity),schedule],capture_output=True,text=True,timeout=5)
     if completed.returncode or completed.stdout!=expected:raise ValueError(json.dumps({'stream':data.hex(),'chunk':chunk,'queue_capacity':capacity,'refusal_schedule':schedule,'expected':expected,'observed':completed.stdout,'stderr':completed.stderr}))
     comparisons+=1
 return {'passed':True,'streams':len(streams),'comparisons':comparisons,'payload_limit':limit,'source_of_expected':'independent whole-buffer Python/zlib oracle','scope':'Finite deterministic corpus, explicit consumer-refusal input, two queue capacities; no exhaustive arbitrary-byte proof.'}

if __name__=='__main__':print(json.dumps(run(sys.argv[1],int(sys.argv[2]))))
