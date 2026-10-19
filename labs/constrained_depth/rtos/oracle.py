"""Independent finite ownership words and closed-form cooperative workload."""
import json,re

def history(word):
    closed=False;states=[0]*3;generations=[0]*3;normal=(0,0);fault=(2,0);results=[];code=word
    for _ in range(4):
        op=code%9;code//=9
        if op in (0,1):
            choices=[2] if op else [0,1]
            available=[i for i in choices if states[i]==0]
            result=4 if closed else 2 if not available else 0
            if not result:
                slot=available[0];states[slot]=1;generations[slot]+=1
                if op:fault=(slot,generations[slot])
                else:normal=(slot,generations[slot])
        elif op==6:closed=True;result=0
        elif op==7:result=0 if closed and not any(states) else 4
        else:
            slot,generation=fault if op==8 else normal
            if generations[slot]!=generation or states[slot]==0:result=3
            else:
                before,after={2:(1,2),3:(2,3),4:(2,1),5:((1,3),0),8:((1,3),0)}[op]
                legal=states[slot] in before if isinstance(before,tuple) else states[slot]==before
                result=0 if legal else 4
                if legal:states[slot]=after
        results.append(result)
    return [word,*results,int(closed),*states,*generations]

def check_pool(text):
    rows=text.splitlines()
    if len(rows)!=6561:raise ValueError('Incomplete ownership history set')
    for word,row in enumerate(rows):
        if row!='P,'+','.join(map(str,history(word))):raise ValueError('Ownership word differs: '+str(word))
    return {'words':6561,'operations_per_word':4,'alphabet':9}

def check_cooperative(text):
    rows=text.splitlines()
    if len(rows)!=103 or rows[-1]!='CR_COOP_PASS:drain-quiescence-and-explicit-unsupported-preemption-ISR':raise ValueError('Incomplete cooperative set')
    for cut,row in enumerate(rows[:-1]):
        n=min(cut,100);total=sum(i*(i%16+1) for i in range(n))
        expected=[cut,n,total,2*n+3,1,2]
        if row!='CR_COOP_ROW:'+','.join(map(str,expected)):raise ValueError('Wrong owned drain trace: '+str(cut))
    return {'stop_cuts':102,'full_count':100,'full_sum':41780,'full_steps':203,'blocked':1,'wakes':2}

def check_guest(text,fp,extra):
    if 'CR_M4_PASS:owned-static-tasks-payloads-and-context' not in text or 'CR_SHUTDOWN:blocked-offer-reclaimed-once quiescent=1' not in text:raise ValueError('Missing target publication/quiescence')
    context=re.search(r'CR_CONTEXT:fp=(\d+) task-checks=(\d+) outer-lr=(\d+) inner-lr=(\d+) nested-MSP-delta=(\d+)',text)
    resources=re.search(r'CR_RESOURCES:application-stacks=(\d+) task-controls=(\d+) queue-storage=(\d+) queue-control=(\d+) payload-pool=(\d+) idle-stack-model=(\d+) watermark0=(\d+)',text)
    if not context or not resources:raise ValueError('Missing actual context/resource measurements')
    f,checks,outer,inner,delta=map(int,context.groups());stacks,controls,queue,control,pool,idle,watermark=map(int,resources.groups())
    if (f,checks,outer,inner)!=(fp,200*fp,0xffffffed if fp else 0xfffffffd,0xffffffe1 if fp else 0xfffffff1) or delta<(104 if fp else 32):raise ValueError('Wrong actual FP/integer exception transition')
    if (stacks,controls,queue,control,pool,idle)!=(4096*(3+extra),84*(3+extra),96,88,136,2048) or not 0<watermark<4096:raise ValueError('Wrong target-specific actual resource ledger')
    if 'CR_WORK:count=100 sum=41780 reserved-fault=1 isr-handoff=1 monitor-checks='+str(7*extra) not in text:raise ValueError('Missing work/extra-task/ISR scope')
    return {'context':list(map(int,context.groups())),'resources':list(map(int,resources.groups())),'hardware_frame_bytes':104 if fp else 32,'port_task_frame_bytes':204 if fp else 68,'possible_alignment_bytes':4,'scope':'Observed ABI/frame/resource values, not physical WCET or a whole-program worst-case stack bound.'}

def analysis():
    return {'historical_fixture':256+256+8*12+64+128,'integer_task_context':8*4+9*4,'FP_task_context':26*4+9*4+16*4,'probe_own_saved_register_bytes':2*4+16*4,'pointer_queue_target_bytes':3*32,'ordinary_payload_capacity':2,'reserved_fault_capacity':1,'full_payload_sum':sum(i*(i%16+1) for i in range(100)),'unknown_indirect_call_stack_bound':'unknown; admission refused, never replaced by zero','limits':'Frame arithmetic excludes application/kernel call chains and possible four-byte alignment. Stack fill observations are neither a proof nor an upper bound.'}
if __name__=='__main__':print(json.dumps(analysis(),sort_keys=True))
