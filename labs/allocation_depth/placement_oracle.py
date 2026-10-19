"""Host-only independent interval model; never uses C tags or free-index internals."""
import json,random,subprocess,sys
def run(binary):
 total=0
 for policy in (0,1):
  for deferred in (0,1):
   for seed in range(16):
    rng=random.Random(seed);blocks=[(0,256,None,0)];live={};operations=[];expected=[]
    def coalesce():
     result=[]
     for off,size,owner,n in blocks:
      if result and owner is None and result[-1][2] is None:
       old=result.pop();result.append((old[0],old[1]+size,None,0))
      else:result.append((off,size,owner,n))
     return result
    def allocate(index,n):
     need=((n+12+7)//8)*8;options=[(i,b) for i,b in enumerate(blocks) if b[2] is None and b[1]>=need]
     if not options:return 2
     i,b=min(options,key=lambda ib:(ib[1][1],ib[1][0])) if policy else options[0]
     off,size,_,_=b;charge=need if size-need>=16 else size;blocks[i:i+1]=[(off,charge,index,n)]+([(off+charge,size-charge,None,0)] if charge<size else []);return 0
    for step in range(256):
     index=rng.randrange(12);op=rng.randrange(1,4) if index in live else 0;n=rng.randrange(1,50);fail=rng.randrange(2);operations.append((op,index,n,fail));old=list(blocks);status=0
     if op==0:status=allocate(index,n)
     elif op==1:
      i=next(i for i,b in enumerate(blocks) if b[2]==index);off,size,_,_=blocks[i];blocks[i]=(off,size,None,0)
      if not deferred:blocks=coalesce()
     elif op==2:
      i=next(i for i,b in enumerate(blocks) if b[2]==index);off,size,_,oldn=blocks[i];need=((n+19)//8)*8;available=size
      if need>size and i+1<len(blocks) and blocks[i+1][2] is None:available+=blocks[i+1][1]
      if need<=available:
       consumed=2 if available>size else 1;charge=need if available-need>=16 else available;blocks[i:i+consumed]=[(off,charge,index,n)]+([(off+charge,available-charge,None,0)] if charge<available else [])
       if not deferred:blocks=coalesce()
      elif fail:status=2
      else:
       status=allocate(99,n)
       if not status:
        blocks=[(o,s,index if k==99 else None if k==index else k,nn if k!=index else 0) for o,s,k,nn in blocks]
        if not deferred:blocks=coalesce()
     else:blocks=coalesce()
     if status:blocks=old
     live={k:(o+8,n) for o,s,k,n in blocks if k is not None}
     tokens=[str(status)]+[f'{live[i][0]}:{live[i][1]}' if i in live else '-' for i in range(12)]+['|',str(sum(n for o,s,k,n in blocks)),str(sum(s for o,s,k,n in blocks if k is not None)),str(sum(k is None for o,s,k,n in blocks))]+[f'{o}:{s}' for o,s,k,n in blocks if k is None]
     expected.append(' '.join(tokens))
    data=f'256 {policy} {deferred}\n'+''.join(' '.join(map(str,op))+'\n' for op in operations);result=subprocess.run([binary],input=data,text=True,capture_output=True,check=True);actual=result.stdout.splitlines()
    if actual!=expected:
     mismatch=next(i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b);raise AssertionError(dict(policy=policy,deferred=deferred,seed=seed,earliest_prefix=operations[:mismatch+1],actual=actual[mismatch],expected=expected[mismatch]))
    total+=len(expected)
 return {'status':'PASS','independent_interval_steps':total,'policies':['lowest-address first fit','best fit then lowest-address tie'],'coalescence':['immediate','explicit deferred collect']}
if __name__=='__main__':print(json.dumps(run(sys.argv[1])))
