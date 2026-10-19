"""Independent exact offset, slice-base and growth-allocation models."""
import argparse,hashlib,itertools,json,subprocess

def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 def run(case,data):
  r=subprocess.run([a.driver,case],input=data,text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;return r.stdout.splitlines()
 maximum,alignment=map(int,run('limits','')[0].split());assert maximum in [2**32-1,2**64-1] and alignment>=8
 ordinary=[(c,n,align) for c in range(33) for n in range(1,36) for align in [1,2,4,8]]
 extras=[(c,n,align) for c in range(33) for n in [0,maximum] for align in [0,1,2,3,4,8,16]]
 cases=ordinary+extras;rows=run('extents',''.join(f'{c} {n} {align}\n' for c,n,align in cases));assert len(rows)==len(cases)
 digest=hashlib.sha256()
 for (c,n,align),row in zip(cases,rows):
  if not n or align not in [1,2,4,8]:expected=[1,77,c,c,c]
  else:
   offset=((c+align-1)//align)*align
   expected=[0,offset,offset+n,c+n,offset+n] if offset+n<=32 else [2,77,c,c,c]
  assert list(map(int,row.split()))==expected,(c,n,align,row,expected)
  digest.update((' '.join(map(str,expected))+'\n').encode())
 slices=[(base,c,n,align) for base in range(8) for c in range(65-base) for n in [0,1,7,32,64,maximum] for align in [0,1,2,3,4,8,16]]
 rows=run('slices',''.join(f'{b} {c} {n} {align}\n' for b,c,n,align in slices));assert len(rows)==len(slices)
 for (b,c,n,align),row in zip(slices,rows):
  if not n or align not in [1,2,4,8]:expected=[1,77,c,c,c]
  else:
   offset=((b+c+align-1)//align)*align
   expected=[0,offset,offset-b+n,c+n,offset-b+n] if offset+n<=64 else [2,77,c,c,c]
  assert list(map(int,row.split()))==expected,(b,c,n,align,row,expected)
  digest.update((' '.join(map(str,expected))+'\n').encode())
 cases=[(ops,fail) for n in range(6) for ops in itertools.product(range(6),repeat=n) for fail in range(4)]
 rows=run('oracle',''.join(f'{len(ops)} {fail} '+(''.join(map(str,ops)) or '-')+'\n' for ops,fail in cases));assert len(rows)==len(cases);transitions=0
 for (ops,fail),row in zip(cases,rows):
  chunks=[0];payload=occupied=attempts=0;expected=[len(ops)]
  for op in ops:
   transitions+=1;n=[1,20,32][op//2];align=8 if op%2 else 1;offset=((chunks[-1]+align-1)//align)*align;rc=0;output_chunk=output_offset=77
   if offset+n>32:
    if len(chunks)==4:rc=2
    else:
     attempts+=1
     if attempts==fail:rc=2
     else:chunks.append(0);offset=0
   if not rc:
    occupied+=offset+n-chunks[-1];chunks[-1]=offset+n;payload+=n;output_chunk=len(chunks)-1;output_offset=offset
   expected.extend([rc,output_chunk,output_offset,len(chunks),payload,occupied,attempts])
  assert list(map(int,row.split()))==expected,(ops,fail,row,expected)
  digest.update((' '.join(map(str,expected))+'\n').encode())
 print(json.dumps(dict(passed=True,ordinary_admissions=len(ordinary),extra_admissions=len(extras),slice_cases=len(slices),growth_histories=len(cases),growth_transitions=transitions,size_max=maximum,max_align=alignment,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
