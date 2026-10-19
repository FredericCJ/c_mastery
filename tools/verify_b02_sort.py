"""Independent stable-order, traffic-bound and full-width progression checks."""
import argparse,hashlib,itertools,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args()
 cases=[k for n in range(8) for k in itertools.product([-1,0,1],repeat=n)]
 data=''.join(str(len(k))+' '+(''.join(str(v+1) for v in k) or '-')+'\n' for k in cases)
 run=subprocess.run([a.driver,'oracle'],input=data,text=True,capture_output=True,timeout=90);assert run.returncode==0,run.stderr
 lines=run.stdout.splitlines();assert len(lines)==len(cases);digest=hashlib.sha256();comparisons=0;directwrites=0;indexwrites=0
 for k,line in zip(cases,lines):
  row=list(map(int,line.split()));n=len(k);levels=(n-1).bit_length() if n else 0
  expected=sorted(range(n),key=lambda i:k[i]);assert row[0]==n and row[5:]==expected and sorted(row[5:])==list(range(n))
  assert row[1]==row[3] and 0<=row[1]<=n*levels and row[2]==2*n*levels and row[4]==n+2*n*levels
  comparisons+=row[1];directwrites+=row[2];indexwrites+=row[4];digest.update((' '.join(map(str,[n,*expected]))+'\n').encode())
 limit=subprocess.run([a.driver,'limits'],text=True,capture_output=True,check=True);maximum=int(limit.stdout);assert maximum in [2**32-1,2**64-1]
 widths=[(n,w) for n in range(256) for w in range(256)]+[(maximum,w) for w in [0,1,maximum//2,maximum//2+1,maximum-1,maximum]]
 run=subprocess.run([a.driver,'widths'],input=''.join(f'{n} {w}\n' for n,w in widths),text=True,capture_output=True,timeout=90);assert run.returncode==0,run.stderr
 rows=run.stdout.splitlines();assert len(rows)==len(widths)
 for (n,w),row in zip(widths,rows):assert list(map(int,row.split()))==([0,min(n,2*w)] if n>=2 and 0<w<n else [1,77])
 print(json.dumps(dict(passed=True,sequences=len(cases),width_cases=len(widths),size_max=maximum,trace_sha256=digest.hexdigest(),comparisons=comparisons,direct_record_writes=directwrites,indirect_index_writes=indexwrites,indirect_payload_writes=0)))
if __name__=='__main__':main()
