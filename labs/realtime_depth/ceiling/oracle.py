"""Independent resource-ownership/donation oracle and explicit proof-premise counterexamples."""
import random
class World:
    def __init__(self,mode,strict):
        self.mode=mode;self.strict=strict;self.base=[2,5,8,1];self.priority=list(self.base)
        self.users=[{0,2},{1,2}];self.ceiling=[2,5];self.held=[set() for _ in range(4)]
        self.owner=[-1,-1];self.pending=[None]*4;self.waiting=[False]*4;self.result=[0]*4;self.queue=[[],[]]
    def recompute(self):
        p=list(self.base)
        if self.mode:
            for r,t in enumerate(self.owner):
                if t>=0:p[t]=min(p[t],self.ceiling[r])
        for _ in range(4):
            for r,queue in enumerate(self.queue):
                if self.owner[r]>=0:
                    for t in queue:p[self.owner[r]]=min(p[self.owner[r]],p[t])
        self.priority=p
    def step(self,op,t,r):
        if t>=4:return 4
        if op==4:return 0 if r==self.base[t] else 4
        if op in (0,1):
            if r>=2:return 4
            if self.pending[t] is not None:return 1
        if op==0:
            if t not in self.users[r]:return 4
            if any(held>=r for held in self.held[t]):return 6
            if self.owner[r]<0:self.owner[r]=t;self.held[t].add(r);answer=0
            else:self.queue[r].append(t);self.waiting[t]=True;self.pending[t]=r;self.result[t]=1;answer=1
        elif op==1:
            if r not in self.held[t]:return 7
            if self.strict and any(held>r for held in self.held[t]):return 4
            self.held[t].remove(r)
            if self.queue[r]:
                user=self.queue[r].pop(0);self.owner[r]=user;self.waiting[user]=False;self.result[user]=0
            else:self.owner[r]=-1
            answer=0
        elif op==2:
            if self.pending[t] is None or not self.waiting[t]:return 4
            self.queue[self.pending[t]].remove(t);self.waiting[t]=False;self.result[t]=3;answer=0
        elif op==3:
            if self.pending[t] is None:return 4
            if self.waiting[t]:return 1
            answer=self.result[t]
            if answer==0:self.held[t].add(self.pending[t])
            self.pending[t]=None
        else:raise ValueError(op)
        self.recompute();return answer
    def row(self,result):
        fields=[result,1]
        for i in range(4):fields += [self.priority[i],sum(1<<r for r in self.held[i]),0 if self.pending[i] is None else self.pending[i]+1,int(self.waiting[i])]
        return fields+self.owner
def cases():
    randomizer=random.Random(5204)
    for mode in (0,1):
        for strict in (0,1):
            for _ in range(128):
                operations=[]
                for _ in range(12):
                    op=randomizer.randrange(5);t=randomizer.randrange(5);arg=randomizer.choice([1,2,5,8]) if op==4 else randomizer.randrange(3)
                    operations.append((op,t,arg))
                world=World(mode,strict);expected=[world.row(world.step(*op)) for op in operations]
                yield ' '.join(map(str,[mode,strict,len(operations),*(v for op in operations for v in op)])),expected
def check(text):
    expected=[row for _,rows in cases() for row in rows];actual=[list(map(int,line.split())) for line in text.splitlines()]
    if actual!=expected:
        at=next((i for i,(a,e) in enumerate(zip(actual,expected)) if a!=e),min(len(actual),len(expected)))
        raise ValueError(f'Resource oracle differs at row{at}; got {actual[at:at+1]}, expected {expected[at:at+1]}')
    return {'passed':True,'traces':512,'transitions':len(expected),'seed':5204,'scope':'Finite two-resource ownership/donation/refusal states; no arbitrary execution proof.'}
def proofs():
    # The predicate is original-PCP admission only, not a complete PCP mutex.
    nominal=2;other_locked_ceilings=[2]
    pcp_admits=all(nominal<c for c in other_locked_ceilings)
    immediate_free_resource_admits=True
    assert not pcp_admits and immediate_free_resource_admits
    # Two dedicated processors: low runs its own sequential sections; high
    # on the other processor cannot preempt low between its unlock and next
    # lock at t=3. This legal ordering creates two direct blocking intervals.
    owners={'A':'L','B':None};wait=None;intervals=[];opened=None;events=[]
    for t in range(7):
        if t==1:wait='A';opened=t;events.append([t,'H waits A'])
        if t==3:
            owners['A']=None;owners['B']='L';events.append([t,'L unlocks A then locks B on CPU0'])
            assert wait=='A';intervals.append((opened,t));owners['A']='H';owners['A']=None
            wait='B';opened=t;events.append([t,'H finishes zero-cost A use then waits B on CPU1'])
        if t==6:
            owners['B']=None;assert wait=='B';intervals.append((opened,t));wait=None;events.append([t,'H obtains B'])
    blocking=sum(end-start for start,end in intervals)
    assert blocking==5 and len(intervals)==2 and blocking>3-1
    return {'passed':True,'pcp_free_resource_refusal':True,'smp_blocking_intervals':intervals,'smp_blocking':blocking,
      'single_cpu_discrete_bound':2,'events':events,
      'limits':'Original finite counterexample model only; not an SMP lock implementation, physical measurement or complete PCP proof.'}
if __name__=='__main__':
    import sys
    if len(sys.argv)>1 and sys.argv[1]=='input':print('\n'.join(line for line,_ in cases()))
    else:print(check(sys.stdin.read()))
