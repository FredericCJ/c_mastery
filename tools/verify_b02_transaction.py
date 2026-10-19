"""Independent acquisition-stack and fallible compensation oracle."""
import argparse,hashlib,itertools,json,subprocess
def expected(failure,commit_failure,*failures):
 owned=[];events=[];primary=cleanup_error=failed_mask=0;stage=4;state=0;published=99;remaining=list(failures);rows=[]
 def cleanup():
  nonlocal state,cleanup_error,failed_mask
  state=2
  while owned:
   slot=owned[-1];events.append(10+slot)
   if remaining[slot]:
    remaining[slot]-=1;cleanup_error=20+slot;failed_mask|=1<<slot;return 7
   owned.pop()
  state=3;return 6 if primary else 0
 def snapshot(status):
  mask=sum(1<<i for i in owned);rows.append(f'{status},{published},{state},{mask},{stage},{primary},{cleanup_error},{failed_mask},{mask}|')
 for slot in range(3):
  events.append(slot)
  if slot==failure:primary=10+slot;stage=slot;break
  owned.append(slot)
 else:
  events.append(3)
  if commit_failure:primary=13;stage=3
 if primary:status=cleanup()
 else:state=1;published=3;status=0
 snapshot(status)
 for _ in range(8):
  status=(6 if primary else 0) if state==3 else cleanup();snapshot(status)
 return ''.join(rows)+''.join(f'{i},' for i in events)
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();cases=list(itertools.product(range(4),range(2),range(3),range(3),range(3)))
 run=subprocess.run([a.driver,'oracle'],input=''.join(' '.join(map(str,c))+'\n' for c in cases),text=True,capture_output=True,timeout=60);assert run.returncode==0,run.stderr;rows=run.stdout.splitlines();assert len(rows)==len(cases);digest=hashlib.sha256()
 for case,row in zip(cases,rows):
  want=expected(*case);assert row==want,(case,row,want);digest.update((row+'\n').encode())
 print(json.dumps(dict(passed=True,histories=len(cases),transitions=len(cases)*9,acquisition_outcomes=4,commit_outcomes=2,cleanup_failures_per_slot=3,recovery_calls=8,trace_sha256=digest.hexdigest())))
if __name__=='__main__':main()
