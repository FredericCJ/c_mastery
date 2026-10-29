"""Independent association-list history model for the original seven-slot map."""
import argparse,hashlib,itertools,json,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();histories=[h for n in range(6) for h in itertools.product(range(9),repeat=n)]
 data=''.join(str(len(h))+' '+(''.join(map(str,h)) or '-')+'\n' for h in histories)
 run=subprocess.run([a.driver,'oracle'],input=data,text=True,capture_output=True,timeout=120);assert run.returncode==0,run.stderr
 lines=run.stdout.splitlines();assert len(lines)==len(histories);digest=hashlib.sha256();transitions=0;maxprobes=0
 for h,line in zip(histories,lines):
  values=list(map(int,line.split()));assert values[0]==len(h) and len(values)==1+7*len(h);model=[];expected=[]
  for step,op in enumerate(h):
   key=op%3*7;action=op//3;at=next((i for i,pair in enumerate(model) if pair[0]==key),None);out=7777
   if action==0:
    if at is None:model.append((key,step+1))
    else:model[at]=(key,step+1)
    status=0
   elif action==1:
    status=4 if at is None else 0
    if at is not None:model.pop(at)
   else:
    status=4 if at is None else 0
    if at is not None:out=model[at][1]
   state=[next((v for k,v in model if k==wanted),7777) for wanted in [0,7,14]]
   actual=values[1+7*step:1+7*(step+1)];assert actual[:3]==[status,out,len(model)] and actual[4:]==state and 1<=actual[3]<=7,(h,step,actual,model)
   maxprobes=max(maxprobes,actual[3]);expected.extend([status,out,len(model),*state]);transitions+=1
  digest.update((' '.join(map(str,[len(h),*expected]))+'\n').encode())
 print(json.dumps(dict(passed=True,histories=len(histories),transitions=transitions,max_history_probes=maxprobes,trace_sha256=digest.hexdigest(),model='independent association list; actual B02 plus actual struct cd_hash and struct cd_chain compared at each transition')))
if __name__=='__main__':main()
