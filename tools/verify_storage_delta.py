"""Independent SDW1 bytes, interruption outcome and equal-contract cost oracle."""
import argparse,hashlib,json,struct,subprocess,zlib

def run(driver):
 def invoke(arg):
  p=subprocess.run([driver,arg],text=True,capture_output=True,timeout=400);assert p.returncode==0,p.stdout+p.stderr;return p.stdout
 expected=[]
 for full in range(2):
  events=37 if full else 8
  for mode in range(7):
   for cut in range(1,events+1):
    publication=cut>=events-1
    expected.append([full,mode,cut,5 if publication else 4,int(publication and mode in [1,4])])
 observed=[[int(v) for v in line.split(',')] for line in invoke('oracle').splitlines()]
 assert observed==expected
 pages=[]
 for full in range(2):
  mask=0xffffffff if full else 0x80000003;p=bytearray(4096)
  struct.pack_into('<IIQII',p,0,0x53445731,2,2,mask,mask.bit_count())
  for index in range(32):
   page=bytearray(4096);page[0]={0:11,1:0,31:77}.get(index,0)
   struct.pack_into('<I',p,32+4*index,zlib.crc32(page))
  struct.pack_into('<I',p,4092,zlib.crc32(p[:4092]));pages.append(p.hex())
 assert invoke('pages').splitlines()==pages
 costs=[dict(full=0,commit=[0,5,3],checkpoint=[36,5,3],useful_bytes=3),dict(full=1,commit=[0,34,3],checkpoint=[65,34,3],useful_bytes=3)]
 assert json.loads(invoke('metrics'))==costs
 trace=hashlib.sha256(json.dumps(dict(cuts=expected,pages=pages,costs=costs),sort_keys=True,separators=(',',':')).encode()).hexdigest()
 return dict(passed=True,commit_cut_rows=len(expected),marker_page_rows=len(pages),costs=costs,trace_sha256=trace,limits=['CRC32 detects selected corruption; it does not authenticate or exclude collisions.','Equal raw-page transaction contracts, not format compatibility with CST2 or a B+tree cost claim.'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
