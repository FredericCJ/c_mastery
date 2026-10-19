"""Independent integer and stable-order oracle for the migrated callback API."""
import argparse,hashlib,itertools,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();digest=hashlib.sha256();values=[-2147483648,-1,0,1,2147483647]
 cases=[(reverse,keys) for reverse in [0,1] for n in range(7) for keys in itertools.product(values,repeat=n)]
 r=subprocess.run([a.driver,'oracle'],input=''.join(f'{rev} {len(keys)} '+' '.join(map(str,keys))+'\n' for rev,keys in cases),text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;rows=r.stdout.splitlines();assert len(rows)==len(cases)
 for (reverse,keys),row in zip(cases,rows):
  expected=sorted(enumerate(keys),key=lambda pair:-pair[1] if reverse else pair[1]);want=''.join(f'{key}:{i} ' for i,key in expected);assert row==want,(keys,row,want);digest.update((row+'\n').encode())
 pairs=list(itertools.product(values+[-2147483647,2147483646,-10000,10000],repeat=2));pairs += [(i,-i) for i in range(-32768,32769)]
 r=subprocess.run([a.driver,'sum_oracle'],input=''.join(f'{x} {y}\n' for x,y in pairs),text=True,capture_output=True,timeout=90);assert r.returncode==0,r.stderr;rows=r.stdout.splitlines();assert len(rows)==len(pairs)
 for (x,y),row in zip(pairs,rows):
  total=x+y;want=f'0 {total}' if -2147483648<=total<=2147483647 else '2 77';assert row==want,(x,y,row,want);digest.update((row+'\n').encode())
 print(json.dumps(dict(passed=True,sort_cases=len(cases),sum_cases=len(pairs),max_length=6,keys=5,orders=2,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
