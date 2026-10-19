"""Independent integer histories and exact logical-command bytes/outcomes."""
import argparse,hashlib,json,struct,subprocess,zlib
def run(driver):
 def invoke(arg):
  p=subprocess.run([driver,arg],text=True,capture_output=True,timeout=400);assert p.returncode==0,p.stdout+p.stderr;return p.stdout
 rows=[]
 for n in range(7):
  for code in range(3**n):
   rest=code;total=10
   for _ in range(n):total+=rest%3-1;rest//=3
   rows.append([n,code,total])
 assert [[int(v) for v in x.split(',')] for x in invoke('oracle').splitlines()]==rows
 cuts=[[mode,cut,9 if cut>=3 else 8,int(cut>=3 and mode in [1,4])] for mode in range(7) for cut in range(1,5)]
 assert [[int(v) for v in x.split(',')] for x in invoke('cuts').splitlines()]==cuts
 page=bytearray(4096);struct.pack_into('<IIQQq',page,0,0x534c5231,2,2,1,-7);struct.pack_into('<I',page,4092,zlib.crc32(page[:4092]));assert invoke('page').strip()==page.hex()
 trace=hashlib.sha256(json.dumps(dict(histories=rows,cuts=cuts,page=page.hex()),sort_keys=True,separators=(',',':')).encode()).hexdigest()
 return dict(passed=True,histories=len(rows),commit_cut_rows=len(cuts),command_page_rows=1,trace_sha256=trace,limits=['The WAL contains ADD and predecessor identity, no resulting counter after-image.','Exact-once durable counter transition is scoped to one exclusive image lifetime and latest sequential operation ID; not arbitrary external effects.'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
