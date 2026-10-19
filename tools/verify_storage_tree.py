"""Independent finite map histories, canonical page bytes and repair accounting."""
import argparse,hashlib,itertools,json,struct,subprocess,zlib

def run(driver,case,data=None):
 p=subprocess.run([driver,case],input=data,text=True,capture_output=True)
 assert p.returncode==0,(driver,case,p.stdout,p.stderr)
 return p.stdout

def check(drivers):
 assert len(drivers)==4
 inputs=[];expected=[]
 # Fifteen distinct operations: put each key with either value, or delete.
 operations=[(op,k,value) for k in range(5) for op,value in [(1,17),(1,93),(0,0)]]
 for history in itertools.product(operations,repeat=3):
  model={}
  for i,(op,key,value) in enumerate(history):
   inputs.append(f'{int(i==0)} {op} {key} {value}')
   status=0 if op or key in model else 1
   if op:model[key]=value
   else:model.pop(key,None)
   expected.append(' '.join(map(str,[status]+[model.get(k,-1) for k in range(5)])))
 # Four distinct insertions force capacity3 splitting, unlike depth3 alone.
 for order in itertools.permutations(range(5)):
  model={}
  for i,key in enumerate(order):
   inputs.append(f'{int(i==0)} 1 {key} {key+33}');model[key]=key+33
   expected.append(' '.join(map(str,[0]+[model.get(k,-1) for k in range(5)])))
 traces=[];metrics=[];pages=[]
 for index,driver in enumerate(drivers):
  out=run(driver,'oracle','\n'.join(inputs)+'\n');rows=out.splitlines();assert rows==expected,next(((i,a,b) for i,(a,b) in enumerate(zip(rows,expected)) if a!=b),('length',len(rows),len(expected)))
  traces.append(hashlib.sha256(out.encode()).hexdigest());metrics.append(json.loads(run(driver,'metrics')))
  capacity=7 if index<2 else 3
  p=bytearray(4096);struct.pack_into('<IIQI',p,0,0x43535432 if capacity==7 else 0x43535434,11,7,1);p[20]=1;p[21]=1;p[32]=1;p[40]=1;p[64]=0x80;p[192]=0xa5;struct.pack_into('<I',p,4092,zlib.crc32(p[:4092]))
  actual=run(driver,'page');assert bytes.fromhex(actual)==bytes(p);pages.append(actual)
 for i,driver in enumerate(drivers):assert 'format-rejection' in run(driver,'foreign',pages[2 if i<2 else 0])
 assert len(set(traces))==1
 for local,full in [(metrics[0],metrics[1]),(metrics[2],metrics[3])]:
  for k in ['capacity','leaf_splits','internal_splits','left','right','merge_left','merge_right','shrink','store_bytes']:assert local[k]==full[k]
  assert local['minimum_visits']>0 and local['repair_nodes']>0 and local['full_visits']==0
  assert full['full_visits']>0 and full['minimum_visits']==0 and full['repair_nodes']==0
 return dict(passed=True,histories=15**3+120,transitions=len(inputs),trace_sha256=traces[0],profiles=metrics,canonical_pages=4,cross_format_rejections=4,limits='Finite independent map states and twelve larger mutation schedules per capacity/policy; visit counts are logical work, not timing or asymptotic proof.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--drivers',nargs=4,required=True);a=p.parse_args();print(json.dumps(check(a.drivers)))
