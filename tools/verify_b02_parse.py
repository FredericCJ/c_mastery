"""Independent byte-grammar/radix integer oracle, including every short split."""
import argparse,hashlib,itertools,json,subprocess
def expected(data,base):
 if not data:return 1,77
 sign=data[:1]==b'-';digits=data[sign:]
 if not digits or digits[:1]==b'0' and (len(digits)!=1 or sign):return 1,77
 alphabet=b'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'[:base]
 if any(b not in alphabet for b in digits):return 1,77
 value=int(data,base)
 return (2,77) if not -(2**31)<=value<2**31 else (0,value)
def encode(n,base):
 sign=b'-' if n<0 else b'';n=abs(n);out=b''
 while n:out=bytes([b'0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'[n%base]])+out;n//=base
 return sign+(out or b'0')
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();inputs=[];outputs=[];counts={'short_split_rows':0,'radix_boundary_rows':0,'long_precedence_rows':0}
 def add(data,base,split,kind):
  status,value=expected(data,base);ordinary=expected(data,10) if base==10 else (-1,77)
  inputs.append(f'{base} {split} {data.hex() or "-"}\n');outputs.append(' '.join(map(str,[*ordinary,status,value,status,value,status,value])));counts[kind]+=1
 for base,alphabet in [(2,b'012-+ \x00x'),(10,b'019-+ \x00x'),(16,b'019AFG-\x00'),(36,b'01AZ[-a\x00')]:
  for n in range(5):
   for t in itertools.product(alphabet,repeat=n):
    data=bytes(t)
    for split in range(n+2):add(data,base,split,'short_split_rows')
 for base in range(2,37):
  for value in [-(2**31)-1,-(2**31),-1,0,1,2**31-1,2**31]:
   data=encode(value,base)
   for split in range(len(data)+2):add(data,base,split,'radix_boundary_rows')
 for base in [2,10,16,36]:
  for data in [b'1'*1000,b'1'*1000+b'!',b'-'+b'1'*1000]:
   for split in [0,len(data)-1,len(data),len(data)+1]:add(data,base,split,'long_precedence_rows')
 r=subprocess.run([a.driver,'oracle'],input=''.join(inputs),text=True,capture_output=True,timeout=180);assert r.returncode==0,r.stderr
 actual=r.stdout.splitlines();assert len(actual)==len(outputs),(len(actual),len(outputs))
 for i,(x,y) in enumerate(zip(actual,outputs)):assert x==y,(inputs[i],x,y)
 print(json.dumps(dict(passed=True,rows=len(outputs),**counts,trace_sha256=hashlib.sha256(('\n'.join(outputs)+'\n').encode()).hexdigest())))
if __name__=='__main__':main()
