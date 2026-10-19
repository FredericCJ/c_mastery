"""Independent Python layout/CRC and set-of-octets oracle; no C decoder reuse."""
import argparse,hashlib,itertools,json,struct,subprocess,zlib
GEN=0x0102030405060708
def seal(b):struct.pack_into('<I',b,4092,zlib.crc32(b[:4092]));return b
def page(version,n):
 b=bytearray(4096);struct.pack_into('<IIQI',b,0,0x43535432 if version==2 else 0x43535433,11 if version==2 else 3,GEN,7);b[20]=1;b[21]=n;struct.pack_into('<I',b,24,31 if n else 0)
 if version==3:struct.pack_into('<I',b,28,128)
 at=128
 for i in range(n):
  k=bytes(range(16*i,16*i+16));v=bytes(range(i,i+64))
  if version==2:b[32+i]=16;b[40+i]=64;b[64+16*i:80+16*i]=k;b[192+64*i:256+64*i]=v
  else:struct.pack_into('<HHHH',b,32+8*i,at,16,at+16,64);b[at:at+16]=k;b[at+16:at+80]=v;at+=80
 return bytes(seal(b))
def check(driver):
 traces=[]
 for version,n in itertools.product([2,3],[0,7]):
  got=subprocess.run([driver,'dump'+str(version)+('max' if n else 'empty')],capture_output=True,check=True).stdout
  assert got==page(version,n),(version,n)
  traces.append(got)
 inputs=[];expected=[]
 offsets=[0,127,128,129,4091,4092,4093,65535]
 for ko,kl,vo,vl in itertools.product(offsets,[0,1,16,17],offsets,[0,1,64,65]):
  b=bytearray(page(3,0));b[21]=1;struct.pack_into('<HHHH',b,32,ko,kl,vo,vl)
  for a,z in [(ko,kl),(vo,vl)]:
   for i in range(a,min(a+z,4092)):
    if i>=128:b[i]=65
  valid=1<=kl<=16 and vl<=64 and 128<=ko<=4092 and ko+kl<=4092 and 128<=vo<=4092 and vo+vl<=4092 and not(set(range(ko,ko+kl))&set(range(vo,vo+vl)))
  inputs.append(bytes(seal(b)));expected.append(f'0 1 {kl} {vl} 0' if valid else '5 165 165 165 1')
 p=subprocess.run([driver,'oracle'],input=b''.join(inputs),capture_output=True,check=True)
 rows=p.stdout.decode().splitlines();assert rows==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(rows,expected)) if a!=b),('length',len(rows),len(expected)))
 trace=hashlib.sha256(b''.join(traces)+p.stdout).hexdigest()
 return dict(passed=True,layout_pages=4,extent_cases=len(inputs),trace_sha256=trace,limits='Single-entry exhaustive selected offset/length product; multi-entry order and overlap are separately exercised C anchors.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
