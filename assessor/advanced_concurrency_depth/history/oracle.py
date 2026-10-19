"""Independent permutations/sets oracle; batch compares actual compiled checker."""
import itertools,json,subprocess,sys
PENDING=2**32-1
def oracle(ops,cap,spec,policy):
 pending=[i for i,p in enumerate(ops) if p[1]==PENDING]
 if pending and policy==0:return 2
 mandatory=[i for i,p in enumerate(ops) if p[1]!=PENDING]
 for bits in range(1<<len(pending)):
  chosen=mandatory+[x for j,x in enumerate(pending) if bits&(1<<j)]
  for order in itertools.permutations(chosen):
   rank={v:k for k,v in enumerate(order)}
   if any(ops[a][1]!=PENDING and ops[a][1]<ops[b][0] and rank[a]>rank[b] for a in chosen for b in chosen):continue
   q=[];good=True
   for i in order:
    inv,res,tid,kind,value,expected=ops[i]
    if not kind:
     actual=3 if spec and value in q else 1 if len(q)==cap else 0
     if actual==0:q.append(value)
     observed=value
    elif spec:
     actual=0 if value in q else 4;observed=value
     if actual==0:q.remove(value)
    else:
     actual=0 if q else 2;observed=q.pop(0) if q else value
    if res!=PENDING and (actual!=expected or (not spec and kind and actual==0 and observed!=value)):good=False;break
   if good:return 0
 return 1
def witness(row,answer):
 n,cap,spec,policy=row[:4];ops=[row[4+6*i:10+6*i] for i in range(n)]
 length,mask=answer[1:3];assert len(answer)==3+3*length
 triples=[answer[3+3*i:6+3*i] for i in range(length)];order=[x[0] for x in triples]
 pending={i for i,p in enumerate(ops) if p[1]==PENDING};complete=set(range(n))-pending
 assert len(set(order))==length and complete<=set(order)<=set(range(n))
 assert mask==sum(1<<i for i in set(order)&pending)
 rank={v:i for i,v in enumerate(order)}
 assert all(not(ops[a][1]!=PENDING and ops[a][1]<ops[b][0]) or rank[a]<rank[b] for a in order for b in order)
 q=[]
 for i,result,value in triples:
  inv,res,tid,kind,v,expected=ops[i]
  if not kind:
   r=3 if spec and v in q else 1 if len(q)==cap else 0;observed=v
   if r==0:q.append(v)
  elif spec:
   r=0 if v in q else 4;observed=v
   if r==0:q.remove(v)
  else:r=0 if q else 2;observed=q.pop(0) if q else v
  assert (result,value)==(r,observed)
  assert res==PENDING or (r==expected and (spec or not kind or r!=0 or observed==v))
def main():
 rows=[];expected=[];counts={};valid={}
 endpoints=[p for p in itertools.permutations(range(6)) if all(p[2*i]<p[2*i+1] for i in range(3))]
 for spec,actions in [(0,[(0,1,0),(0,2,0),(0,1,1),(1,1,0),(1,2,0),(1,0,2)]),(1,[(0,1,0),(0,2,0),(0,1,3),(0,2,1),(1,1,0),(1,2,0),(1,1,4),(1,2,4)])]:
  key='fifo' if spec==0 else 'set';counts[key]=0;valid[key]=0
  for times in endpoints:
   for actions3 in itertools.product(actions,repeat=3):
    ops=[(times[2*i],times[2*i+1],i,*a) for i,a in enumerate(actions3)];result=oracle(ops,2,spec,0);rows.append([3,2,spec,0,*[x for p in ops for x in p]]);expected.append(result);counts[key]+=1;valid[key]+=result==0
 for kind in range(2):
  for value in [1,2]:
   for result in [0,1,2,3,4]:
    for spec in [0,1]:
     for policy in [0,1]:
      ops=[(0,PENDING,0,0,1,0),(1,2,1,kind,value,result)];rows.append([2,2,spec,policy,*[x for p in ops for x in p]]);expected.append(oracle(ops,2,spec,policy))
 data=''.join(' '.join(map(str,r))+'\n' for r in rows)
 completed=subprocess.run([sys.argv[1],'driver'],input=data,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=120)
 assert completed.returncode==0,completed.stderr
 answers=[list(map(int,line.split())) for line in completed.stdout.splitlines()];actual=[line[0] for line in answers];assert actual==expected,next(((i,a,b,rows[i]) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('length',len(actual),len(expected)))
 witnessed=0
 for row,answer in zip(rows,answers):
  if answer[0]==0:witness(row,answer);witnessed+=1
  else:assert len(answer)==1
 print(json.dumps(dict(status='pass',counts=counts,linearizable=valid,pending_cases=80,total=len(rows),endpoint_orders=len(endpoints),independently_checked_witnesses=witnessed,limit='All chosen three-operation actions/event orders and explicit one-pending fragment; not all histories or C executions.'),indent=2))
if __name__=='__main__':main()
