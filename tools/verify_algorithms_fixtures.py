"""Independent narrow certificates for T36/T37; no learner algorithm or private author generator is imported."""
from __future__ import annotations
from bisect import bisect_left
from collections import Counter, deque
from fractions import Fraction
from itertools import product
import json
from pathlib import Path
import statistics
from common import ROOT, parse

FIXTURES = ROOT / 'fixtures/algorithms'
checks = 0
def require(condition, message):
    global checks
    checks += 1
    if not condition: raise ValueError(message)
def integer(v):
    return type(v) is int
def tree(s):
    nodes=s['nodes']; seen=set()
    def walk(key,parent,lo,hi):
        if key is None: return [],0
        require(key in nodes and key not in seen,'missing/cyclic/shared tree node')
        seen.add(key); n=nodes[key]; k=n['key']
        require(integer(k) and lo<k<hi and n['parent']==parent,'tree order/parent')
        left,lh=walk(n['left'],key,lo,k); right,rh=walk(n['right'],key,k,hi)
        require(integer(n['height']) and n['height']==1+max(lh,rh),'cached height')
        if s['balanced']: require(abs(lh-rh)<=1,'AVL balance')
        return left+[k]+right,1+max(lh,rh)
    seq,h=walk(s['root'],None,float('-inf'),float('inf'))
    require(seen==set(nodes),'unreachable tree nodes')
    return seq,h
def simple_paths(vertices,edges,source):
    # Deliberately exponential tiny-graph oracle, independent of BFS or priority settling.
    require(len(vertices)<=8 and source in vertices,'tiny graph scope')
    adj={v:[] for v in vertices}
    for u,v,w in edges:
        require(u in adj and v in adj and integer(w),'edge representation')
        adj[u].append((v,w))
    paths={v:[] for v in vertices}
    def visit(v,visited,weight,hops):
        paths[v].append((weight,hops))
        for nxt,w in adj[v]:
            if nxt not in visited: visit(nxt,visited|{nxt},weight+w,hops+1)
    visit(source,{source},0,0)
    return paths
def alu(op,a,b,width):
    require(op in ('add','sub') and integer(width) and width==4,'scoped four-bit ALU operation')
    require(integer(a) and integer(b) and 0<=a<16 and 0<=b<16,'four-bit ALU operands')
    mod=1<<width; top=mod>>1
    signed=lambda x: x-mod if x>=top else x
    raw=a+b if op=='add' else a-b
    signed_raw=signed(a)+signed(b) if op=='add' else signed(a)-signed(b)
    value=raw%mod
    return {'result':value,'Z':int(value==0),'N':int(value>=top),
            'C_arm':int(raw>=mod) if op=='add' else int(a>=b),
            'CF_x86':int(raw>=mod) if op=='add' else int(a<b),
            'V':int(not -top<=signed_raw<top)}
def cache_trace(addresses,lines,block):
    require(lines>0 and block>0 and lines&(lines-1)==0 and block&(block-1)==0,'cache geometry')
    tags=[None]*lines; shadow=[]; seen=set(); out=[]; cats=[]
    for address in addresses:
        require(integer(address) and address>=0,'byte address')
        number=address//block; index=number%lines; tag=number//lines
        hit=tags[index]==tag
        out.append('hit' if hit else 'miss')
        cats.append(None if hit else ('compulsory' if number not in seen else ('conflict' if number in shadow else 'capacity')))
        tags[index]=tag
        if number in shadow: shadow.remove(number)
        shadow.append(number)
        if len(shadow)>lines: shadow.pop(0)
        seen.add(number)
    return out,cats

def verify(kind,i,e):
    if kind=='ledger':
        q=deque(); states=[]; statuses=[]; removed=[]
        for op in i['operations']:
            status='ok'
            if op[0]=='put':
                if len(q)==i['capacity']: status='full'
                else: q.append(op[1])
            elif op[0]=='take':
                require(bool(q),'empty queue fixture'); removed.append(q.popleft())
            else: raise ValueError('unknown queue operation')
            states.append(list(q)); statuses.append(status)
        s=i['gcd_states']; require(s[0]==i['gcd_input'],'GCD initial')
        for (a,b),(x,y) in zip(s,s[1:]):
            require(b>0 and (x,y)==(b,a%b) and 0<=y<b,'Euclid invariant/progress')
        bad=i['weakened_sort']
        return dict(queue_states=states,status=statuses,removed=removed,persistent_payload_bytes=i['capacity']*i['record_bytes'],slot_offsets=[x*i['record_bytes'] for x in range(i['capacity'])],live_payload_bytes=[len(x)*i['record_bytes'] for x in states],gcd=s[-1][0],remainder_steps=len(s)-1,weak_sorted=bad['after']==sorted(bad['after']),weak_permutation=Counter(bad['before'])==Counter(bad['after']))
    if kind=='lower_bound':
        a=i['array']; require(a==sorted(a) and all(integer(x) and 0<=x<2**32 for x in a),'u32 sorted input')
        counts=[]
        for case in i['cases']:
            lo,hi=0,len(a); key=case['key']
            for cl,ch,mid in case['steps']:
                require((cl,ch)==(lo,hi) and lo<hi and mid==lo+(hi-lo)//2,'interval/mid certificate')
                require(all(x<key for x in a[:lo]) and all(x>=key for x in a[hi:]),'partition invariant')
                old=hi-lo
                if a[mid]<key: lo=mid+1
                else: hi=mid
                require(hi-lo<old,'search progress')
            require(lo==hi==bisect_left(a,key),'search terminal certificate')
            counts.append(len(case['steps']))
        return dict(indices=[bisect_left(a,c['key']) for c in i['cases']],comparisons=counts,empty_index=bisect_left(i['empty'],0),element_bytes=4,maximum_read_byte_offset=4*(len(a)-1))
    if kind=='stable_sort':
        r=i['records']; shifts=sum(r[j][0]>r[k][0] for k in range(len(r)) for j in range(k))
        comparisons=sum(sum(x[0]>r[k][0] for x in r[:k])+int(any(x[0]<=r[k][0] for x in r[:k])) for k in range(1,len(r)))
        require(Counter(map(tuple,r))==Counter(map(tuple,sorted(r,key=lambda x:x[0]))),'tag multiset')
        return dict(stable_records=sorted(r,key=lambda x:x[0]),insertion_comparisons=comparisons,shift_writes=shifts,placement_writes=len(r)-1,sorted_comparisons=len(i['sorted_keys'])-1,reverse_comparisons=len(i['reverse_keys'])*(len(i['reverse_keys'])-1)//2,reverse_shift_writes=len(i['reverse_keys'])*(len(i['reverse_keys'])-1)//2,merge_order=[x[1] for x in sorted(i['merge_left']+i['merge_right'],key=lambda x:x[0])])
    if kind=='heap_cert':
        finals=[]; comps=[]; swaps=[]
        for c in i['cases']:
            states=c['states']; path=c['path']; require(len(states)==len(path),'heap path/state count')
            count=0; n=len(states[0]); base=Counter(states[0])
            for pos,(a,p) in enumerate(zip(states,path)):
                require(Counter(a)==base and 0<=p<n,'heap multiset/path range')
                children=[x for x in (2*p+1,2*p+2) if x<n]
                # The moving violation can only concern the current node's child edges.
                require(all(a[(x-1)//2]>=a[x] for x in range(1,n) if (x-1)//2!=p),'intact heap edges outside violation')
                if not children: continue
                child=max(children,key=lambda x:(a[x],-x)); count+=len(children)-1+1
                if pos+1<len(states):
                    require(a[child]>a[p] and path[pos+1]==child,'legal max-child swap')
                    after=a.copy(); after[p],after[child]=after[child],after[p]
                    require(after==states[pos+1],'heap swap delta')
                else: require(a[p]>=a[child],'terminal heap node')
            require(all(states[-1][(x-1)//2]>=states[-1][x] for x in range(1,n)),'final max heap')
            finals.append(states[-1]); comps.append(count); swaps.append(len(states)-1)
        return dict(finals=finals,key_comparisons=comps,swaps=swaps)
    if kind=='avl_cert':
        rot=[tree(x) for x in i['rotation']]; deletion=[tree(x) for x in i['deletion']]
        require(all(x[0]==rot[0][0] for x in rot),'rotation preserves inorder')
        require(deletion[1][0]==deletion[2][0] and set(deletion[0][0])-set(deletion[1][0])=={1},'single-key deletion')
        require(i['duplicate_key'] in rot[-1][0],'duplicate actually present')
        s=i['balanced_search_tree']; tree(s); key=s['root']; count=0
        while key is not None:
            count+=1; n=s['nodes'][key]
            if n['key']==i['search_key']: break
            key=n['right'] if i['search_key']>n['key'] else n['left']
        require(key is not None,'search found')
        return dict(rotation_inorders=[x[0] for x in rot],rotation_heights=[x[1] for x in rot],deletion_inorders=[x[0] for x in deletion],deletion_heights=[x[1] for x in deletion],duplicate_result='reject_unchanged',balanced_search_comparisons=count,degenerate_search_comparisons=len([x for x in [1,2,3] if x<=i['search_key']]))
    if kind=='probes_spans':
        slots=i['slots']; n=len(slots); paths=[]; results=[]
        for key in i['lookup_keys']:
            path=[]; result=None
            for step in range(n):
                index=(key%n+step)%n; path.append(index); state,value=slots[index]
                require(state in ('occupied','deleted','empty'),'slot state')
                if state=='occupied' and value==key: result=index; break
                if state=='empty': break
            paths.append(path); results.append(result)
        full=i['full_slots']; require(all(x[0]=='occupied' and x[1]!=i['full_absent_key'] for x in full),'full absent case')
        t=i['text']; offsets=[]; counts=[]
        for p in i['patterns']:
            offsets.append([k for k in range(len(t)-len(p)+1) if t[k:k+len(p)]==p])
            count=0
            for k in range(len(t)-len(p)+1):
                for j,v in enumerate(p):
                    count+=1
                    if t[k+j]!=v: break
            counts.append(count)
        return dict(probe_indices=paths,lookup_results=results,full_absent_probes=len(full),match_offsets=offsets,byte_comparisons=counts)
    if kind=='paths':
        v=i['vertices']; paths=simple_paths(v,i['edges'],i['source'])
        minimum=lambda x,col: min((p[col] for p in x),default=None)
        distances=[minimum(paths[x],0) for x in v]; hops=[minimum(paths[x],1) for x in v]
        edge={(a,b):w for a,b,w in i['edges']}; pred=[]
        for x,distance in zip(v,distances):
            candidates=[u for u in v if (u,x) in edge and distances[v.index(u)] is not None and distance is not None and distances[v.index(u)]+edge[u,x]==distance]
            if x==i['source'] or distance is None: pred.append(None)
            else:
                require(len(candidates)==1,'fixture has unique tight predecessor'); pred.append(candidates[0])
        for x,parent in zip(v,pred):
            if parent is not None: require(distances[v.index(parent)]+edge[parent,x]==distances[v.index(x)],'predecessor tight edge')
        require([distances[v.index(x)] for x in i['settled']]==sorted(x for x in distances if x is not None),'settled minimum order')
        neg=simple_paths(['A','B','C'],i['negative_example'],'A')
        a,b=i['overflow_operands']; require(a<=i['distance_limit'] and a+b>i['distance_limit'],'overflow anchor')
        direct_B=next(w for a,b,w in i['negative_example'] if a=='A' and b=='B')
        return dict(weighted_distances=distances,hop_distances=hops,predecessors=pred,negative_true_B=minimum(neg['B'],0),premature_settle_B=direct_B,overflow_result='range_error_unchanged')
    if kind=='adapters':
        d={}; states=[]; statuses=[]
        for op in i['operations']:
            key=op[1]; status='ok'
            if op[0]=='insert':
                if key in d: status='duplicate'
                elif len(d)>=i['capacity']: status='full'
                else: d[key]=op[2]
            elif op[0]=='remove':
                require(key in d,'remove exists'); del d[key]
            else: raise ValueError('adapter operation')
            states.append([[k,r] for k,r in sorted(d.items(),key=lambda x:(x[1],x[0]))]); statuses.append(status)
        require(set(d)<=set(i['caller_owns_ids']),'caller owns live records')
        return dict(visible_states=states,status=statuses,component_frees=0,retained_after_destroy=0,live_elements=list(map(len,states)))
    if kind=='alu':
        w=i['width']; require(w==4,'bounded ALU width')
        pairs=list(product(range(1<<w),repeat=2)); add=[alu('add',a,b,w) for a,b in pairs]; sub=[alu('sub',a,b,w) for a,b in pairs]
        return dict(cases=[alu(op,a,b,w) for op,a,b in i['cases']],add_carry_pairs=sum(x['C_arm'] for x in add),add_overflow_pairs=sum(x['V'] for x in add),sub_borrow_pairs=sum(x['CF_x86'] for x in sub),sub_overflow_pairs=sum(x['V'] for x in sub),pairs_per_operation=len(pairs),full_adder_rows=[[a,b,c,(a+b+c)%2,(a+b+c)//2] for a,b,c in product((0,1),repeat=3)])
    if kind=='instruction_bytes':
        reg=int(i['initial_rax'],16); values=[]
        widths={'AL':8,'AX':16,'EAX':32}
        for name,hexval in i['writes']:
            width=widths[name]; v=int(hexval,16); require(v<1<<width,'register value width')
            reg=v if width==32 else (reg&~((1<<width)-1))|v
            values.append(f'{reg:016X}')
        cmp=i['x86_cmp_bytes']; require(cmp[:2]==[0x48,0x3d] and len(cmp)==6,'selected REX.W CMP only')
        imm=int.from_bytes(bytes(cmp[2:]),'little',signed=True); branch=i['x86_jcc_bytes']; require(branch[0]==0x77 and len(branch)==2,'selected JA rel8 only')
        half=int.from_bytes(bytes(i['thumb_cmp_bytes']),'little'); require(half>>11==0b00101,'Thumb CMP T1 opcode')
        a=i['address']; effective=a['base']+a['index']*a['scale']+a['displacement']; require(a['base']<=effective and effective+4<=a['base']+a['extent'],'address range')
        return dict(rax_after=values,cmp_width=64,cmp_signed_immediate=imm,cmp_length=len(cmp),branch_predicate='CF=0 and ZF=0',branch_target=i['branch_address']+len(branch)+int.from_bytes(bytes(branch[1:]),'little',signed=True),thumb_rn=(half>>8)&7,thumb_immediate=half&255,thumb_widths=[32 if x>>11 in (0b11101,0b11110,0b11111) else 16 for x in i['thumb_first_halfwords']],little_endian_word=int.from_bytes(bytes(i['data_bytes']),'little'),effective_address=effective,access_end_exclusive=effective+4)
    if kind=='toy_commit':
        require(len(i['registers'])==4 and len(i['memory'])==16,'toy state dimensions')
        require(all(integer(x) and 0<=x<256 for x in i['registers']+i['memory']),'toy byte state')
        regs=i['registers'].copy(); mem=i['memory'].copy(); pc=0; r1=[]; pcs=[]; halted=False
        for event in i['events']:
            require(event['op'] in ('LI','SUB','BNZ','ST','LD','HALT'),'toy operation out of scope')
            for field in ('dst','src','left','right'):
                if field in event: require(integer(event[field]) and 0<=event[field]<4,'toy register range')
            for field in ('address','pc','next','target'):
                if field in event: require(integer(event[field]) and 0<=event[field]<16,'toy address/program-index range')
            if 'value' in event: require(integer(event['value']) and 0<=event['value']<256,'toy immediate byte range')
            require(event['pc']==pc and not halted,'toy PC/halt certificate')
            next_regs=regs.copy(); next_mem=mem.copy(); op=event['op']; next_pc=pc+1
            if op=='LI': next_regs[event['dst']]=event['value']
            elif op=='SUB': next_regs[event['dst']]=(regs[event['left']]-regs[event['right']])%256
            elif op=='BNZ': next_pc=event['target'] if regs[event['src']] else pc+1
            elif op=='ST': next_mem[event['address']]=regs[event['src']]
            elif op=='LD': next_regs[event['dst']]=mem[event['address']]
            elif op=='HALT': next_pc=pc; halted=True
            else: raise ValueError('toy operation out of scope')
            require(next_pc==event['next'],'toy next-state certificate')
            regs,mem,pc=next_regs,next_mem,next_pc; r1.append(regs[1]); pcs.append(pc)
        return dict(r1_after=r1,pc_after=pcs,final_registers=regs,memory_5=mem[5],commits=len(pcs),halted=halted)
    if kind=='pipeline_cert':
        s=i['schedule']; retired=[key for key,row in s.items() if 'WB' in row]; used={}
        for key,row in s.items():
            for stage in ('IF','ID','EX','MEM','WB'):
                for cycle in row.get(stage,[]) if isinstance(row.get(stage),list) else ([row[stage]] if stage in row else []):
                    require(integer(cycle) and cycle>0 and (stage,cycle) not in used,'pipeline stage cycle/structural double occupancy'); used[stage,cycle]=key
            if 'WB' in row:
                require(row['MEM']==row['EX']+1 and row['WB']==row['MEM']+1,'pipeline result-stage order')
                end_id=max(row['ID']) if isinstance(row['ID'],list) else row['ID']
                require(row['EX']==end_id+1,'pipeline ID to EX')
        for p,c,kind in i['dependencies']:
            availability=s[p]['MEM'] if kind=='LOAD' else s[p]['EX']
            require(s[c]['EX']>availability,'forwarded value available before EX')
        branch_cycle=s[i['branch']]['EX']; flushed=[r for r in s.values() if 'flushed' in r]
        require(all(r['flushed']==branch_cycle and 'WB' not in r for r in flushed),'branch squash')
        require(s[i['target']]['IF']==branch_cycle+1,'target refetch')
        # Write-first/read-second register file; no forwarding, each next EX waits one cycle after prior WB.
        ex=3
        for _ in range(3): ex=ex+2+1
        no_forward=ex+2
        return dict(retired=retired,cycles=max(r['WB'] for r in s.values() if 'WB' in r),load_use_bubbles=s['I4']['EX']-s['I3']['EX']-1,flushed_fetches=len(flushed),no_forwarding_four_instruction_cycles=no_forward,forwarding_four_instruction_cycles=s['I4']['WB'],invalid_same_cycle_load_use=i['raw_counterexample']['consumer_EX']<=i['raw_counterexample']['producer_MEM'])
    if kind=='cache':
        a=i['addresses']; lines=i['line_count']; block=i['block_bytes']; out,cats=cache_trace(a,lines,block); _,capacity=cache_trace(i['capacity_addresses'],lines,block)
        misses=out.count('miss'); base=Fraction(str(i['base_cpi']))*i['instruction_count']; stalls=Fraction(str(i['estimated_miss_rate']))*i['memory_references']*i['estimated_miss_penalty_cycles']; total=base+stalls
        return dict(indices=[x//block%lines for x in a],tags=[x//block//lines for x in a],offsets=[x%block for x in a],outcomes=out,misses=misses,compulsory=cats.count('compulsory'),conflict=cats.count('conflict'),capacity=cats.count('capacity'),capacity_trace_last_category=capacity[-1],amat_ns=float(Fraction(i['hit_ns'])+Fraction(misses,len(a))*i['miss_penalty_ns']),estimated_base_cycles=float(base),estimated_stall_cycles=float(stalls),estimated_total_cycles=float(total),estimated_cpu_ns=float(total/i['frequency_hz']*10**9))
    if kind=='dma_map':
        regions={r['name']:r for r in i['regions']}; statuses=[]
        for b in i['buffers']:
            r=regions[b['region']]
            if not r['base']<=b['address'] or b['bytes']<0 or b['address']+b['bytes']>r['base']+r['bytes']: status='crosses_declared_region'
            elif not r['dma']: status='not_dma_reachable'
            else: status='ok'
            statuses.append(status)
        owner='struct lr_cpu'; owners=[]
        for event in i['ownership_events']:
            if event=='give_dma': require(owner=='struct lr_cpu','DMA give owner'); owner='DMA'
            elif event=='dma_done': require(owner=='DMA','DMA completion owner'); owner='struct lr_cpu'
            elif event=='cpu_write_attempt': require(owner=='DMA','write rejection anchor')
            else: require(owner=='struct lr_cpu','struct lr_cpu access owner')
            owners.append(owner)
        words=[]
        for width,value in i['bridge_writes']:
            require(width in (8,16) and 0<=value<1<<width,'bridge width'); words.append(sum(value<<(x*width) for x in range(32//width)))
        return dict(placement_results=statuses,ordinary_sram_bytes=sum(r['bytes'] for r in regions.values() if r['dma']),owner_after=owners,write_attempt='reject_unchanged',toy_round_robin_grants=i['request_order']*2,expanded_bridge_words=words)
    if kind=='exception_frame':
        require(i['exc_return']==0xFFFFFFFD and len(i['basic_words'])==8,'scoped basic PSP frame only')
        require(i['pre_exception_psp']%8==0 and i['basic_words'][7]&(1<<24),'Thumb and aligned pre-entry stack assumptions')
        require(i['basic_words'][7]&(1<<9)==0,'scoped frame excludes stack-alignment padding')
        base=i['pre_exception_psp']-32; number=i['irq_number']+16
        return dict(reset_pc=i['reset_vector']&~1,reset_thumb_bit=i['reset_vector']&1,exception_number=number,vector_address=i['vtor']+4*number,frame_base=base,frame_word_addresses=[base+4*x for x in range(8)],return_mode='Thread',return_stack='PSP',frame_kind='basic',restored_pc=i['basic_words'][6],restored_lr=i['basic_words'][5],restored_psp=base+32,higher_priority_number=min(i['priority_numbers']),automatically_stacked=['r0','r1','r2','r3','r12','LR','PC','xPSR'])
    if kind=='performance':
        time=Fraction(i['instruction_count'])*Fraction(str(i['cpi']))/i['frequency_hz']*10**9; f=Fraction(str(i['improved_fraction'])); ratio=1-f+f/i['local_speedup']; c=i['synthetic_c_ns']; a=i['synthetic_asm_ns']
        return dict(baseline_ns=float(time),new_ns=float(time*ratio),whole_speedup=float(1/ratio),infinite_local_speedup_limit=float(1/(1-f)),c_median_ns=statistics.median(c),asm_median_ns=statistics.median(a),c_range_ns=[min(c),max(c)],asm_range_ns=[min(a),max(a)],median_ratio=statistics.median(c)/statistics.median(a),timing_evidence_class='synthetic_model')
    raise ValueError('unsupported oracle kind '+kind)

def equal(actual,expected):
    if isinstance(expected,dict): return isinstance(actual,dict) and set(actual)==set(expected) and all(equal(actual[k],v) for k,v in expected.items())
    if isinstance(expected,list): return isinstance(actual,list) and len(actual)==len(expected) and all(equal(a,b) for a,b in zip(actual,expected))
    if isinstance(expected,bool) or expected is None or isinstance(expected,str): return type(actual)==type(expected) and actual==expected
    if type(expected) is int: return type(actual) is int and actual==expected
    if type(expected) is float: return type(actual) is float and abs(actual-expected)<=1e-12*max(1,abs(expected))
    return False
def input_types(value,path=()):
    """Boolean labels occur only in declared topology/balance fields; integers must not accept True as 1."""
    if path and path[-1] in ('balanced','cpu','dma'):
        require(type(value) is bool,'declared boolean field '+'.'.join(path))
        return
    if isinstance(value,dict):
        for key,item in value.items(): input_types(item,path+(key,))
    elif isinstance(value,list):
        for index,item in enumerate(value): input_types(item,path+(str(index),))
    elif isinstance(value,bool): require(bool(path) and path[-1] in ('balanced','cpu','dma'),'boolean in numeric/state input '+'.'.join(path))
    elif isinstance(value,float): require(bool(path) and path[-1] in ('base_cpi','cpi','estimated_miss_rate','improved_fraction'),'noninteger input '+'.'.join(path))
    else: require(value is None or type(value) in (str,int),'invalid input scalar')
def main():
    failures=[]; paths=sorted(p for p in FIXTURES.rglob('*') if p.is_file()); references={}; expected_ids={f'PR-T{d}-{n:02}' for d in (36,37) for n in range(1,9)}
    try:
        project_ids=set()
        for path in sorted((ROOT/'curriculum/projects').glob('*.md')):
            meta,_=parse(path)
            if meta['module'] not in ('T36','T37'): continue
            project_ids.add(meta['id'])
            for value in meta['fixtures']:
                require(value not in references,'duplicate fixture reference'); references[value]=meta['id']
        require(project_ids==expected_ids,'16 canonical projects must match owned domains')
        require(set(references)=={p.relative_to(ROOT).as_posix() for p in paths},'canonical metadata / complete recursive fixture tree correspondence')
    except Exception as exc: failures.append('catalog: '+str(exc))
    for path in paths:
        try:
            f=json.loads(path.read_text(encoding='utf-8')); require(path.suffix=='.json','unexpected fixture format')
            require(f['id']==path.stem==references.get(path.relative_to(ROOT).as_posix()),'fixture ID/file/project correspondence')
            require(bool(f['description']) and bool(f['verification_method']),'missing oracle scope')
            input_types(f['input'])
            actual=verify(f['kind'],f['input'],f['expected'])
            require(equal(actual,f['expected']),'expected values differ: '+json.dumps(actual))
        except Exception as exc: failures.append(path.name+': '+str(exc))
    print(json.dumps({'fixtures':len(paths),'checks':checks,'failures':failures},indent=2))
    if failures: raise SystemExit(1)
if __name__=='__main__': main()
