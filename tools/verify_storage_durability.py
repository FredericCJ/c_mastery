"""Independent persistence outcomes, model correspondence and CRC collision."""
import argparse,hashlib,json,struct,subprocess,zlib
from model_storage_protocol import check as model_check
def collision(original):
 changed=bytearray(original);changed[192]^=1
 baseline=zlib.crc32(changed[:4092]);target=baseline^zlib.crc32(original[:4092]);basis={}
 for bit in range(32):
  probe=bytearray(changed);probe[1024+bit//8]^=1<<(bit%8);v=zlib.crc32(probe[:4092])^baseline;mask=1<<bit
  while v:
   pivot=v.bit_length()-1
   if pivot in basis:v^=basis[pivot][0];mask^=basis[pivot][1]
   else:basis[pivot]=(v,mask);break
 assert len(basis)==32
 selected=0
 while target:
  pivot=target.bit_length()-1;v,mask=basis[pivot];target^=v;selected^=mask
 for bit in range(32):
  if selected>>bit&1:changed[1024+bit//8]^=1<<(bit%8)
 assert changed!=original and zlib.crc32(changed[:4092])==zlib.crc32(original[:4092])
 return changed
def check(driver):
 def run(case,data=None):
  p=subprocess.run([driver,case],input=data,text=True,capture_output=True);assert p.returncode==0,(case,p.stdout,p.stderr);return p.stdout
 expected=[f'{mode} {cut} {4 if cut<=35 else 7} {int(cut>=36 and mode in [1,4])}' for mode in range(7) for cut in range(1,38)]
 raw=run('oracle');assert raw.splitlines()==expected
 medium=[]
 for subset in range(4):medium.append(f'S {subset} {17 if subset&1 else 0} {34 if subset&2 else 0}')
 positions=[0,511,512,1023,1024,2047,2048,4091,4095]
 for mode in range(8):
  values=[34 if mode in [1,4,7] or mode==2 and p<2048 or mode==3 and p>=2048 or mode==6 and 512<=p<1024 else 0 for p in positions];medium.append(' '.join(map(str,['T',mode]+values)))
 raw_medium=run('medium');assert raw_medium.splitlines()==medium
 commit=[(1,66),(2,0)]+[(1,p) for p in range(34,66)]+[(2,0),(1,66),(2,0)]
 checkpoint=[(1,p) for p in range(2,34)]+[(2,0),(1,0),(2,0),(1,66),(2,0)]
 raw_trace=run('trace');assert raw_trace.splitlines()==[f'C {k} {p}' for k,p in commit]+[f'R {k} {p}' for k,p in checkpoint]
 page=bytearray(4096);struct.pack_into('<IIQI',page,0,0x43535432,11,7,1);page[20]=page[21]=page[32]=page[40]=1;page[64]=0x80;page[192]=0xa5;struct.pack_into('<I',page,4092,zlib.crc32(page[:4092]));other=collision(page);raw_collision=run('decode',page.hex()+'\n'+other.hex()+'\n');assert raw_collision.splitlines()==['0 165','0 164']
 abstract=model_check();assert abstract['positive']['states']==135 and abstract['positive']['transitions']==332
 return dict(passed=True,commit_schedules=259,medium_cases=12,actual_commit_events=len(commit),actual_checkpoint_events=len(checkpoint),crc_collision=dict(rank=32,first_value=165,second_value=164,checksum=zlib.crc32(page[:4092]),page_sha256=hashlib.sha256(page).hexdigest(),changed_sha256=hashlib.sha256(other).hexdigest()),model_checker=abstract,trace_sha256=hashlib.sha256((raw+raw_medium+raw_trace+raw_collision).encode()).hexdigest(),limits='Actual original C IO order matches the stated two-after-image abstraction. Structural validation is an additive read-only guard; the finite CRC-validity abstraction explicitly excludes the separately demonstrated collision and physical persistence behavior.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
