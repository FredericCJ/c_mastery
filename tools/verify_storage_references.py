"""Independent map and complete CST5 leaf/metadata byte expectations."""
import argparse,hashlib,itertools,json,struct,subprocess,zlib
def check(driver):
 inputs=[];expected=[]
 ops=[(op,k,v) for k in range(5) for op,v in [(1,17),(1,93),(0,0)]]
 histories=list(itertools.product(ops,repeat=3))+[tuple((1,k,k+33) for k in order) for order in itertools.permutations(range(5))]
 for history in histories:
  model={}
  for i,(op,key,value) in enumerate(history):
   inputs.append(f'{int(i==0)} {op} {key} {value}');status=0 if op or key in model else 1
   if op:model[key]=value
   else:model.pop(key,None)
   expected.append(' '.join(map(str,[status]+[model.get(k,-1) for k in range(5)])))
 def run(case,data=None):
  p=subprocess.run([driver,case],input=data,text=True,capture_output=True);assert p.returncode==0,p.stderr;return p.stdout
 out=run('oracle','\n'.join(inputs)+'\n');assert out.splitlines()==expected
 leaf=bytearray(4096);struct.pack_into('<IIQI',leaf,0,0x43535435,11,7,1);leaf[20]=leaf[21]=leaf[32]=leaf[40]=1;leaf[64]=0x80;leaf[192]=0xa5;struct.pack_into('<Q',leaf,768,19);struct.pack_into('<I',leaf,4092,zlib.crc32(leaf[:4092]));assert bytes.fromhex(run('page'))==leaf
 meta=bytearray(4096);struct.pack_into('<IIQI',meta,0,0x43535435,10,0,0);struct.pack_into('<II',meta,20,1,3);struct.pack_into('<Q',meta,32,1);struct.pack_into('<Q',meta,72,1);struct.pack_into('<I',meta,4092,zlib.crc32(meta[:4092]));assert bytes.fromhex(run('meta'))==meta
 # A valid parent-profile node has no allocation incarnation extension.
 old=bytearray(leaf);struct.pack_into('<I',old,0,0x43535432);old[768:856]=bytes(88);struct.pack_into('<I',old,4092,zlib.crc32(old[:4092]));assert 'format-rejection' in run('foreign',old.hex())
 return dict(passed=True,histories=len(histories),transitions=len(inputs),trace_sha256=hashlib.sha256(out.encode()).hexdigest(),canonical_pages=2,foreign_rejection=1,limits='Finite map histories and exact CST5 byte profile; allocation identity/retirement/persistence cases are separate C anchors, not a physical persistence proof.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
