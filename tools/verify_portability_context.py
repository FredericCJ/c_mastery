"""Independent Python codec/original-boundary oracle; no C decoder reuse."""
import argparse,codecs,hashlib,itertools,json,subprocess,unicodedata

def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();cases=[]
 for n in range(3):
  for values in itertools.product(range(256),repeat=n):
   data=bytes(values)
   for split in range(n+1):cases.append((data,split))
 boundaries=[0x80,0x7ff,0x800,0xd7ff,0xe000,0xffff,0x10000,0x10ffff]+list(range(0,0x110000,997))
 for value in boundaries:
  if 0xd800<=value<=0xdfff:continue
  data=chr(value).encode('utf8')
  for n in range(len(data)+1):
   for split in range(n+1):cases.append((data[:n],split))
 for data in [b'\xed\xa0\x80',b'\xf4\x90\x80\x80',b'\xf0\x80\x80\x80',b'\xe0\x80\x80',b'e\xcc\x81',b'\xc3\xa9',b'A\x00B']:
  for split in range(len(data)+1):cases.append((data,split))
 request=''.join(f'{len(data)} {split} '+' '.join(f'{v:02x}' for v in data)+'\n' for data,split in cases)
 r=subprocess.run([a.driver,'oracle'],input=request,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;rows=r.stdout.splitlines();assert len(rows)==len(cases);digest=hashlib.sha256();counts={0:0,2:0,3:0}
 for (data,split),row in zip(cases,rows):
  status=0;out=77
  try:
   out=len(data.decode('utf8'))
  except UnicodeDecodeError as error:status=3 if error.reason=='unexpected end of data' else 2
  expected=f'{status} {out}';assert row==expected,(data,split,row,expected);counts[status]+=1;digest.update((row+'\n').encode())
 assert unicodedata.normalize('NFC','e\u0301')=='\u00e9' and len('e\u0301')==2 and len('\u00e9')==1
 print(json.dumps(dict(passed=True,cases=len(cases),complete=counts[0],invalid=counts[2],incomplete=counts[3],unicode_version=unicodedata.unidata_version,canonical_equivalence=True,trace_sha256=digest.hexdigest())))

if __name__=='__main__':main()
