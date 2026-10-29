"""Explicit node/reservation/deadline oracle using sets and per-struct lr_cpu event lists."""
from itertools import product
from random import Random
NONE=(1<<32)-1

def job(task,generation,cost,deadline,release=0,affinity=3,pred=None,nominal=None):
    nominal=release if nominal is None else nominal
    return {'task':task,'generation':generation,'cost':cost,'pred':pred or [0]*len(cost),'deadline':deadline,
      'release':release,'nominal':nominal,'jitter':release-nominal,'affinity':affinity}

def cases():
    for scale in (10,20,50,100):
        yield {'cpus':2,'horizon':scale+2,'q':0,'m':0,'partitioned':0,'home':[0]*8},[job(0,1,[scale],scale),job(1,1,[1],scale-1),job(2,1,[1],scale-1)]
    for cost in product((1,2,3),repeat=4):
        for cpus in (1,2):
            yield {'cpus':cpus,'horizon':16,'q':0,'m':0,'partitioned':0,'home':[0]*8},[job(0,1,list(cost),14,affinity=(1<<cpus)-1,pred=[0,1,1,6])]
    rng=Random(5210)
    for index in range(300):
        cpus=2;n=3+index%5;partitioned=index%4==0;homes=[rng.randrange(2) for _ in range(8)]
        cfg={'cpus':cpus,'horizon':32,'q':index%3,'m':(index//3)%3,'partitioned':int(partitioned),'home':homes}
        jobs=[]
        for i in range(n):
            task=i%4;nominal=rng.randrange(12);release=nominal+rng.randrange(4);vertices=1 if index%2 else rng.randint(1,4)
            pred=[rng.randrange(1<<v) for v in range(vertices)];mask=(1<<homes[task]) if partitioned else rng.randint(1,3)
            jobs.append(job(task,1+i//4,[rng.randint(1,5) for _ in range(vertices)],nominal+rng.randint(1,14),release,mask,pred,nominal))
        yield cfg,jobs

def inputs():
    for c,jobs in cases():
        values=[len(jobs),c['cpus'],c['horizon'],c['q'],c['m'],c['partitioned'],*c['home']]
        for j in jobs:
            values.extend([j['task'],j['generation'],j['affinity'],len(j['cost']),j['nominal'],j['release'],j['jitter'],j['deadline']])
            for cost,pred in zip(j['cost'],j['pred']):values.extend([cost,pred])
        yield ' '.join(map(str,values))

def simulate(config,jobs):
    # Job and node states are separate; overhead is a list of already-reserved
    # CPU events. Predecessor readiness is snapshotted before any tick service.
    c=config;cpus=c['cpus'];state=[0]*len(jobs);remaining={};executed={};complete=[set() for _ in jobs]
    started=[NONE]*len(jobs);finished=[NONE]*len(jobs);missed=[0]*len(jobs)
    counts=[0]*9;last_owner={};last_on_cpu=[None]*cpus;events=[[] for _ in range(cpus)];reserved=[None]*cpus
    lock=None;traces=[]
    for tick in range(c['horizon']+1):
        if tick<c['horizon']:
            for i,j in enumerate(jobs):
                if j['release']!=tick:continue
                if any(state[k]==1 and jobs[k]['task']==j['task'] for k in range(len(jobs))):state[i]=3;counts[7]+=1
                else:
                    state[i]=1
                    for v,cost in enumerate(j['cost']):remaining[i,v]=cost
        for i,j in enumerate(jobs):
            if state[i]==1 and not missed[i] and j['deadline']<=tick:missed[i]=1;counts[6]+=1
        if tick==c['horizon']:break
        ready={(i,v) for i,j in enumerate(jobs) if state[i]==1 for v in range(len(j['cost']))
          if remaining[i,v] and all(s in complete[i] for s in range(len(j['cost'])) if j['pred'][v]&(1<<s))}
        chosen=set();release_lock=False
        for cpu in range(cpus):
            if reserved[cpu] is None:
                candidates=[node for node in ready-chosen-set(x for x in reserved if x is not None)
                  if jobs[node[0]]['affinity']&(1<<cpu) and (not c['partitioned'] or c['home'][jobs[node[0]]['task']]==cpu)]
                node=min(candidates,key=lambda x:(jobs[x[0]]['deadline'],x[0],x[1])) if candidates else None
                if node is None:
                    traces.append((tick,cpu,0,NONE,NONE,0,0));counts[1]+=1;last_on_cpu[cpu]=None;continue
                switching=node!=last_on_cpu[cpu] or last_owner.get(node)!=cpu
                if switching and c['q'] and lock is not None:
                    traces.append((tick,cpu,3,NONE,NONE,0,0));counts[3]+=1;last_on_cpu[cpu]=None;continue
                reserved[cpu]=node
                queue=c['q'] if switching else 0
                moving=c['m'] if switching and node in last_owner and last_owner[node]!=cpu else 0
                events[cpu]=[2]*queue+[4]*moving+[1]
                if queue:lock=cpu
            i,v=reserved[cpu];kind=events[cpu].pop(0);after=remaining[i,v]
            if kind==2:
                counts[2]+=1
                if not events[cpu] or events[cpu][0]!=2:release_lock=True
            elif kind==4:counts[4]+=1
            else:
                counts[0]+=1;chosen.add((i,v))
                if (i,v) in last_owner and last_owner[i,v]!=cpu:counts[5]+=1
                last_owner[i,v]=cpu;last_on_cpu[cpu]=(i,v);reserved[cpu]=None
                if started[i]==NONE:started[i]=tick
                remaining[i,v]-=1;after=remaining[i,v];executed[i,v]=executed.get((i,v),0)+1
                if after==0:complete[i].add(v)
                if len(complete[i])==len(jobs[i]['cost']):state[i]=2;finished[i]=tick+1;counts[8]+=1
            traces.append((tick,cpu,kind,i,v,jobs[i]['generation'],after))
        if release_lock:lock=None
    rows=['G '+' '.join(map(str,counts))]
    for i,j in enumerate(jobs):
        values=[i,state[i],started[i],finished[i],sum(1<<v for v in complete[i]),missed[i]]
        for v in range(len(j['cost'])):values.extend([remaining.get((i,v),0),executed.get((i,v),0)])
        rows.append('J '+' '.join(map(str,values)))
    rows.extend('T '+' '.join(map(str,t)) for t in traces)
    return rows,counts

def check(text):
    observed=text.splitlines();expected=[];summary={'cases':0,'rows':0,'work':0,'queue_ticks':0,'migration_ticks':0,'misses':0,'rejected_releases':0}
    for c,jobs in cases():
        rows,counts=simulate(c,jobs);expected.extend(rows);summary['cases']+=1
        for key,index in [('work',0),('queue_ticks',2),('migration_ticks',4),('misses',6),('rejected_releases',7)]:summary[key]+=counts[index]
    if len(observed)!=len(expected):raise ValueError('Missing/extra finite trace rows')
    for i,(actual,wanted) in enumerate(zip(observed,expected)):
        if actual!=wanted:raise ValueError(f'Global EDF/DAG/overhead mismatch row{i}: {actual!r} != {wanted!r}')
    summary['rows']=len(expected);return summary
