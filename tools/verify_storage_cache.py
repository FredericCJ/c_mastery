"""Independent cache replacement and authority trace model."""
import argparse,hashlib,itertools,json,subprocess
def check(driver):
 actions=[(1,0,1,0,0),(5,0,1,0,0),(3,0,1,0,0),(1,1,1,0,0),(1,0,2,0,0),(1,0,1,1,0),(9,0,1,0,1)]
 commands=[];expected=[];histories=0
 for policy in range(3):
  for trace in itertools.product(actions,repeat=4):
   histories+=1;slots=[None]*4;recent=[0]*2;reads=hits=0
   for step,(logical,gen,owner,wal,fail) in enumerate(trace):
    commands.append(f'{int(step==0)} {policy} {logical} {gen} {owner} {wal} {fail}')
    physical=(34 if wal else 2)+logical;tag=(owner,gen,physical);value=(17*owner+gen+3*physical)%251
    candidates=[logical%4] if policy==0 else [2*(logical%2),2*(logical%2)+1] if policy==1 else [0 if logical==1 else 1+logical%3]
    hit=next((i for i in candidates if slots[i]==tag),None)
    if hit is not None:
     hits+=1;status=0
     if policy==1:recent[logical%2]=hit-candidates[0]
    else:
     reads+=1;status=4 if fail else 0
     if not fail:
      choice=next((i for i in candidates if slots[i] is None),None)
      if choice is None:choice=candidates[1-recent[logical%2]] if policy==1 else candidates[0]
      slots[choice]=tag
      if policy==1:recent[logical%2]=choice-candidates[0]
    if status:value=165
    expected.append(f'{status} {reads} {hits} {value} {value}')
 p=subprocess.run([driver,'oracle'],input='\n'.join(commands)+'\n',text=True,capture_output=True,check=True);actual=p.stdout.splitlines()
 assert actual==expected,next(((i,a,b,commands[i]) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('length',len(actual),len(expected)))
 return dict(passed=True,histories=histories,transitions=len(commands),trace_sha256=hashlib.sha256(p.stdout.encode()).hexdigest(),model='Three deterministic four-entry policies; exact owner/generation/physical tag; failures retain entries; finite four-operation histories.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(check(a.driver)))
