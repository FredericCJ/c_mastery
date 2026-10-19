"""Independent finite oracles; these do not certify implementation or hardware."""
from pathlib import Path
import json, math
from collections import deque
ROOT=Path(__file__).resolve().parents[2]

def compute(k,x):
    if k=='syscall': return {'result':-1 if x['raw_result']<0 else x['raw_result'],'errno':-x['raw_result'] if x['raw_result']<0 else 0}
    if k=='references':
        v=x['initial']; a=[]
        for d in x['deltas']: v+=d; assert v>=0; a.append(v)
        return {'counts':a,'first_zero_step':a.index(0)+1 if 0 in a else None}
    if k in ('srtf','edf'):
        jobs=x['jobs']; rem={j[0]:j[2] for j in jobs}; starts={};finish={};t=0
        while any(rem.values()):
            ready=[j for j in jobs if j[1]<=t and rem[j[0]]]
            if ready:
                j=min(ready,key=lambda j:((rem[j[0]] if k=='srtf' else j[3]),j[1],j[0]));name=j[0]
                starts.setdefault(name,t); rem[name]-=1
                if not rem[name]:finish[name]=t+1
            t+=1;assert t<10000
        return {'finish':finish,'first_start':starts} if k=='srtf' else {'finish':finish,'misses':[j[0] for j in jobs if finish[j[0]]>j[3]]}
    if k=='stride':
        p=[0]*len(x['weights']);s=p.copy()
        for _ in range(x['dispatches']):i=min(range(len(p)),key=lambda i:(p[i],i));p[i]+=x['scale']//x['weights'][i];s[i]+=1
        return {'service':s}
    if k=='pages':
        a=sorted(set(v//x['page_size'] for v in x['addresses']));return {'unique_pages':a,'first_touch_faults':len(a)}
    if k=='fifo_faults':
        out=[]
        for c in x['capacities']:
            q=deque();n=0
            for v in x['refs']:
                if v not in q:
                    n+=1
                    if len(q)==c:q.popleft()
                    q.append(v)
            out.append(n)
        return {'faults':out}
    if k=='offsets':
        handles={};offsets={}
        for a,b,c in x['actions']:
            if a=='open':handles[b]=c;offsets[c]=0
            elif a=='dup':handles[c]=handles[b]
            else:offsets[handles[b]]+=c
        return {'offsets':{h:offsets[v] for h,v in handles.items()}}
    if k=='completion':
        a=[];r=0
        for g,v in x['events']:
            if g==x['active_generation'] and not a:a.append(v)
            else:r+=1
        return {'accepted':a,'rejected':r}
    if k=='quota':return {'full_periods':x['elapsed']//x['period'],'service_ceiling':x['elapsed']//x['period']*x['quota']}
    if k=='bytes':return {'application_written':sum(x['writes']),'application_read':sum(x['reads']),'acknowledged_durable_prefix':sum(x['writes'][:x['durable_prefix_writes']])}
    if k=='image':return {'unresolved':sorted(set(x['symbols'])-set(x['provided'])),'forbidden_libraries':[v for v in x['libraries'] if v in ['libc','libgcc','compiler-rt','libatomic']]}
    if k=='pool':
        used=0;a=[];r=[]
        for i,d in enumerate(x['requests']):
            if 0<=used+d<=x['capacity']:used+=d
            else:r.append(i)
            a.append(used)
        return {'used':a,'rejected':r}
    if k=='frame':
        n=(x['software_words']+x['hardware_words'])*x['word_bytes'];a=x['alignment'];return {'payload_bytes':n,'aligned_bytes':(n+a-1)//a*a}
    if k=='states':
        transitions={('ready','dispatch'):'running',('running','block'):'blocked',('blocked','wake'):'ready',('running','exit'):'exited',('exited','reap'):'free'};s=x['initial'];o=[]
        for a in x['actions']:s=transitions[s,a];o.append(s)
        return {'states':o}
    if k=='periodic':
        t=x['phase'];r=[]
        for w in x['work']:t+=x['period'];r.append(t);t+=w
        return {'absolute_releases':[x['phase']+x['period']*(i+1) for i in range(len(r))],'relative_releases':r}
    if k=='masks':
        s=[x['initial']];during=[]
        for v in x['enter_masks']:
            nonzero=[m for m in (s[-1],v) if m]
            s.append(min(nonzero) if nonzero else 0);during.append(s[-1])
        after=[]
        while len(s)>1:s.pop();after.append(s[-1])
        return {'during':during,'after_exit':after}
    if k=='winner':return {'terminal':x['events'][0],'ignored':x['events'][1:]}
    if k=='timer_counts':
        n=(x['microseconds']*x['frequency']+999999)//1000000;return {'counts':n,'actual_us_numerator':n*1000000,'actual_us_denominator':x['frequency']}
    if k=='budget':
        n=sum(x['ram_components']);return {'ram_used':n,'ram_free':x['ram_limit']-n,'cycles_total':sum(x['cycles'])}
    if k=='wait_protocol':
        published=False
        for e in x['events']:
            if e=='publish':published=True
            if e=='recheck':return {'sleep':not published,'reason':'predicate_true_at_recheck' if published else 'enrolled_before_sleep'}
    if k=='semaphore':
        c=x['initial'];b=over=success=0
        for a in x['actions']:
            if a=='wait':
                if c:c-=1;success+=1
                else:b+=1
            elif b:b-=1;success+=1
            elif c<x['maximum']:c+=1
            else:over+=1
        return {'count':c,'blocked':b,'overflow':over,'successful_waits':success}
    if k=='mutex':
        owner=None;errors=[]
        for a,t in x['actions']:
            if a=='lock':
                if owner is None:owner=t
                else:errors.append('busy')
            elif owner!=t:errors.append('nonowner_unlock')
            else:owner=None
        return {'owner':owner,'errors':errors}
    if k=='inheritance':
        p=x['base'].copy()
        for _ in p:
            for waiter,owner in x['waits']:p[owner]=max(p[owner],p[waiter])
        return {'effective':p}
    if k=='predicate':
        n=x['initial_items']
        for a in x['actions']:
            if a=='other_consume':n-=1
        return {'items':n,'waiter_blocks':n==0}
    if k=='queue':
        q=deque();out=[];r=0
        for a in x['actions']:
            if a[0]=='put':
                if len(q)<x['capacity']:q.append(a[1])
                else:r+=1
            elif q:out.append(q.popleft())
            else:r+=1
        return {'received':out,'rejected':r,'remaining':list(q)}
    if k=='rendezvous':
        pending=False;done=False;canceled=False
        for a in x['events']:
            if a=='send':pending=True
            if a=='cancel_sender' and pending:pending=False;canceled=True
            if a=='receive' and pending:done=True;pending=False
        return {'transferred':done,'sender_result':'canceled' if canceled else 'complete' if done else 'pending'}
    if k=='events':
        seen=x['bits']&x['mask'];m=bool(seen) if x['mode']=='any' else seen==x['mask'];return {'matched':m,'observed':seen if m else 0,'remaining':x['bits']&~seen if m and x['clear'] else x['bits']}
    if k=='notification':
        accepted=min(sum(x['posts']),x['maximum']-x['initial']);n=x['initial']+accepted;c=min(n,x['consumes']);return {'accepted':accepted,'consumed':c,'remaining':n-c}
    if k=='barrier':
        pending=set();g=0;released=[]
        for t in x['arrivals']:
            assert t not in pending;pending.add(t)
            if len(pending)==x['participants']:released.append(g);g+=1;pending.clear()
        return {'released_generations':released,'generation':g,'pending':sorted(pending)}
    if k=='once':
        state='uninitialized';owner=None;n=0
        for a in x['attempts']:
            if a.startswith('begin_') and state=='uninitialized':state='initializing';owner=a[-1];n+=1
            if a=='fail_'+str(owner):state='failed'
        return {'state':state,'initializer':owner,'attempt_count':n}
    if k=='rw_admit':
        readers=set();writer=None;wait=[];admitted=[]
        for e in x['events']:
            a,t=e.split('_')
            if a=='read' and writer is None and not wait:readers.add(t);admitted.append(t)
            elif a in ('read','write'):wait.append((a,t))
            elif a=='release':
                readers.discard(t)
                if not readers and writer is None and wait:_,writer=wait.pop(0);admitted.append(writer)
        return {'admitted':admitted,'waiting':[t for _,t in wait]}
    if k=='lifetime':
        r=x['refs'];s='open'
        for e in x['events']:
            if e=='close':s='closed'
            elif e=='drop':r-=1
            elif e=='destroy':assert r==0 and s=='closed';s='destroyed'
        return {'state':s,'refs':r}
    if k=='priority_order':return {'rm':[j[0] for j in sorted(x['tasks'],key=lambda j:(j[1],j[0]))],'dm':[j[0] for j in sorted(x['tasks'],key=lambda j:(j[2],j[0]))]}
    if k=='response':
        r=x['cost']+x['blocking'];seq=[r]
        for _ in range(100):
            new=x['cost']+x['blocking']+sum((r+j+t-1)//t*c for c,t,j in x['higher']);seq.append(new)
            if new==r or new>x['deadline']:break
            r=new
        return {'iterations':seq,'response':seq[-1],'schedulable':seq[-1]<=x['deadline']}
    if k=='ceiling':
        a=sorted(x['held_ceilings']);return {'effective':max([x['base']]+a),'after_release_highest':max([x['base']]+a[:-1])}
    if k=='replenishment':return {'replenishments':[[t+x['period'],v] for t,v in x['consumption']],'total':sum(v for _,v in x['consumption'])}
    if k=='reservation':
        q=x['runtime'];d=x['demand'];a=[]
        while d:a.append(min(q,d));d-=a[-1]
        return {'windows':a,'finish_if_start_zero':(len(a)-1)*x['period']+a[-1]}
    if k=='modular':
        d=(x['deadline']-x['now'])%(1<<x['bits']);return {'distance':d,'future_within_half_range':0<d<(1<<(x['bits']-1))}
    if k=='packing':
        bins=[0]*x['cores'];bad=[]
        for v in sorted(x['loads_tenths'],reverse=True):
            i=next((i for i,b in enumerate(bins) if b+v<=10),None)
            if i is None:bad.append(v)
            else:bins[i]+=v
        return {'bins_tenths':bins,'unplaced':bad,'total_tenths':sum(x['loads_tenths'])}
    if k=='dhall':return {'long_job_start':x['epsilon'],'long_job_finish':x['period']+x['epsilon'],'deadline':x['period'],'miss':x['epsilon']>0}
    if k=='tick_round':
        n=(x['delay_ms']+x['tick_ms']-1)//x['tick_ms'];return {'ticks':n,'realized_ms':n*x['tick_ms']}
    if k=='reservation_valid':return {'valid':0<x['runtime']<=x['deadline']<=x['period'],'utilization_numerator':x['runtime'],'utilization_denominator':x['period']}
    if k=='cpu_inventory':return {'enabled_apic_ids':[a for a,e in x['entries'] if e]}
    if k=='ack':
        a=sorted({c for c,e in x['acks'] if e==x['expected_epoch'] and c in x['expected_cpus']});return {'acknowledged':a,'missing':sorted(set(x['expected_cpus'])-set(a))}
    if k=='ranges':
        a=sorted(x['ranges']);return {'overlap':any(a[i][1]>a[i+1][0] for i in range(len(a)-1)),'total_bytes':sum(b-a for a,b in a)}
    if k=='tickets':
        m=1<<x['bits'];a=[(x['next']+i)%m for i in range(x['acquires'])];return {'tickets':a,'next':(x['next']+x['acquires'])%m,'order':sorted(a,key=lambda t:(t-x['serving'])%m)}
    if k=='owners':
        loads={}
        for _,c in x['placements']:loads[str(c)]=loads.get(str(c),0)+1
        return {'unique':len(set(t for t,_ in x['placements']))==len(x['placements']),'loads':loads}
    if k=='doorbell':return {'drained':x['published'],'required_messages':len(x['published']),'notification_count':x['interrupts']}
    if k=='migration':
        ok=x['owner']==x['source'] and x['destination'] in x['affinity'];return {'allowed':ok,'owner_after':x['destination'] if ok else x['owner']}
    if k=='partition':
        n=len(x['values']);w=x['workers'];ranges=[];sums=[];a=0
        for i in range(w):b=a+n//w+(i<n%w);ranges.append([a,b]);sums.append(sum(x['values'][a:b]));a=b
        return {'ranges':ranges,'partial_sums':sums,'sum':sum(sums)}
    if k=='dag':
        order=[];end={};nodes=set(x['costs'])
        while nodes:
            ready=sorted(n for n in nodes if all(a in end for a,b in x['edges'] if b==n));assert ready
            n=ready[0];end[n]=x['costs'][n]+max([end[a] for a,b in x['edges'] if b==n] or [0]);order.append(n);nodes.remove(n)
        return {'work':sum(x['costs'].values()),'span':max(end.values()),'topological':order}
    if k=='ring':
        n=(x['head']-x['tail'])%(1<<x['bits']);return {'used':n,'free':x['capacity']-n,'valid':n<=x['capacity']}
    if k=='cache_span':
        a=x['line_bytes'];lo=x['address']//a*a;hi=(x['address']+x['length']+a-1)//a*a;return {'start':lo,'end':hi,'bytes':hi-lo}
    if k=='buffer_states':
        trans={('free','fill'):'cpu_filling',('cpu_filling','submit'):'device_owned',('device_owned','complete'):'completed',('completed','consume'):'cpu_consuming',('cpu_consuming','release'):'free'};s='free';out=[]
        for a in x['actions']:s=trans[s,a];out.append(s)
        return {'states':out}
    if k=='cycles':return {'microseconds':x['cycles']*1000000/x['frequency']}
    if k=='epochs':return {'accepted_indices':[i for i,e in enumerate(x['message_epochs']) if e==x['current']],'rejected_indices':[i for i,e in enumerate(x['message_epochs']) if e!=x['current']]}
    if k=='pipeline':return {'uncontended_latency':sum(x['latencies']),'total_queue_slots':sum(x['queues'])}
    if k=='cas':
        v=x['initial'];r=[]
        for expected,new in x['attempts']:
            ok=v==expected;r.append(ok)
            if ok:v=new
        return {'results':r,'final':v}
    if k=='tag':
        v=(x['initial']+x['reuses'])%(1<<x['bits']);return {'final':v,'wrapped':x['initial']+x['reuses']>=1<<x['bits'],'equal_to_initial':v==x['initial']}
    if k=='hazards':return {'reclaimable':[v for v in x['retired'] if v not in x['protected']],'retained':[v for v in x['retired'] if v in x['protected']]}
    if k=='epoch_reclaim':
        limit=min(x['active_epochs']);return {'reclaimable':[v for v,e in x['retired'] if e<limit],'retained':[v for v,e in x['retired'] if e>=limit]}
    if k=='grace':
        a=[i for i,(en,ex) in enumerate(zip(x['reader_entries'],x['reader_exits'])) if en<x['publication_time']<=ex];return {'grace_end':max([x['reader_exits'][i] for i in a] or [x['publication_time']]),'preexisting_readers':a}
    if k=='history':
        ops=x['operations']
        def search(pending,q,done):
            if not pending:return True
            for i in pending:
                op,val,begin,end=ops[i]
                if any(ops[j][3]<begin for j in pending if j!=i):continue
                nq=q.copy()
                if op=='enq':nq.append(val)
                elif not nq or nq.pop(0)!=val:continue
                if search(pending-{i},nq,done|{i}):return True
            return False
        return {'linearizable':search(set(range(len(ops))),[],set())}
    if k=='speedup':
        s=x['serial']/x['parallel'];return {'speedup':s,'efficiency':s/x['workers']}
    if k=='reap':
        seen=set();out=[]
        for c in x['waits']:
            if c not in x['children']:out.append('not_child')
            elif c in seen:out.append('already_reaped')
            else:out.append(x['children'][c]);seen.add(c)
        return {'results':out,'remaining':sorted(set(x['children'])-seen)}
    if k=='cow':return {'original_refs':x['initial_refs']-1,'child_private':True,'free_frames':x['free_frames']-1}
    if k=='span':
        end=x['address']+x['length'];return {'valid':x['allowed_start']<=x['address']<=end<=x['allowed_end'],'end':end}
    if k=='read':
        n=min(x['requested'],max(0,x['file_size']-x['offset']));return {'returned':n,'new_offset':x['offset']+n}
    if k=='fuel':
        f=x['fuel'];n=0
        for c in x['instruction_costs']:
            if c>f:break
            f-=c;n+=1
        return {'completed':n,'remaining':f,'terminal':'complete' if n==len(x['instruction_costs']) else 'fuel_exhausted'}
    if k=='durable':return {'reply_is_durable':'flush_complete' in x['events'] and x['events'].index('flush_complete')<x['events'].index('reply')}
    if k=='recovery':
        committed={r[1] for r in x['records'] if r[0]=='commit'};value=None;discard=[]
        for r in x['records']:
            if r[0]=='put':
                if r[1] in committed:value=r[2]
                else:discard.append(r[1])
        return {'committed_value':value,'discarded_transactions':discard}
    raise ValueError(k)

def verify():
    count=0;errors=[]
    for p in sorted((ROOT/'fixtures/os_v3').glob('*.json')):
        d=json.loads(p.read_text(encoding='utf8'));count+=1
        try:
            got=compute(d['oracle'],d['input'])
            if got!=d['expected']:errors.append({'id':d['id'],'expected':d['expected'],'actual':got})
        except Exception as e:errors.append({'id':d['id'],'error':repr(e)})
    return {'passed':not errors,'fixtures':count,'errors':errors,'scope':'Finite declared examples only; no implementation, emulator or physical certification'}

if __name__=='__main__':
    report=verify();print(json.dumps(report,indent=2));raise SystemExit(0 if report['passed'] else 1)
