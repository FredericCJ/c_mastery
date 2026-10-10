"""Finite arithmetic/history evidence for T45; does not execute learner C or hardware."""
from pathlib import Path
import copy,json,re,sys,tempfile
from common import META

ROOT=Path(__file__).resolve().parents[1]
FIXTURES=ROOT/'fixtures/memory_lab'

def require(ok,message):
    if not ok: raise ValueError(message)

def keys(value,names):
    require(type(value) is dict and set(value)==set(names),'object fields')

def integer(value,minimum=None,maximum=None):
    require(type(value) is int,'exact integer required')
    require(minimum is None or value>=minimum,'integer lower bound')
    require(maximum is None or value<=maximum,'integer upper bound')
    return value

def boolean(value):
    require(type(value) is bool,'exact boolean required');return value

def choice(value,options):
    require(type(value) is str and value in options,'unknown enum');return value

def array(value):
    require(type(value) is list,'array required');return value

def octets(value):
    require(type(value) is str and re.fullmatch(r'(?:[0-9a-f]{2})*',value) is not None,'canonical octet hex')
    return bytes.fromhex(value)

def strict_json(text):
    def unique(pairs):
        result={}
        for key,value in pairs:
            require(key not in result,'duplicate JSON object key')
            result[key]=value
        return result
    def invalid_constant(value):
        raise ValueError('non-JSON numeric constant: '+value)
    return json.loads(text,object_pairs_hook=unique,parse_constant=invalid_constant)

def metadata(path):
    text=path.read_text(encoding='utf-8-sig');match=META.match(text)
    require(match is not None,'fenced project/module contract')
    return strict_json(match[1])

def comparison(actual,expected):
    require(type(actual) is type(expected),'expected JSON type differs')
    if type(actual) is dict:
        require(set(actual)==set(expected),'expected fields differ')
        return sum(comparison(v,expected[k]) for k,v in actual.items())
    if type(actual) is list:
        require(len(actual)==len(expected),'expected array length differs')
        return sum(comparison(a,b) for a,b in zip(actual,expected))
    require(actual==expected,'expected value differs');return 1

def derive(i):
    require(type(i) is dict and 'kind' in i,'input object')
    k=choice(i['kind'],['observations','extents','sizes','stack','ram','cleanup','lifetime','initialization','async','diagnosis'])
    if k=='observations':
        keys(i,['kind','trials']);results=[];used=0;labels=set()
        for t in array(i['trials']):
            keys(t,['label','setup','path','outcome']);require(type(t['label']) is str and t['label'] not in labels,'trial identity');labels.add(t['label'])
            setup=choice(t['setup'],['ready','unavailable','failed']);path=choice(t['path'],['negative','repair']);out=choice(t['outcome'],['report','silent','not_run'])
            require((setup=='ready')==(out!='not_run'),'setup/outcome coherence')
            if setup!='ready':verdict='no_observation'
            else:
                used+=1
                verdict=('detected_this_trial' if out=='report' else 'inconclusive') if path=='negative' else ('repair_failed' if out=='report' else 'supports_this_trial')
            results.append(verdict)
        return dict(verdicts=results,observations_used=used,universal_safety_proven=False)
    if k=='extents':
        keys(i,['kind','capacity','initial_hex','cases']);cap=integer(i['capacity'],1,256);initial=octets(i['initial_hex']);require(len(initial)==cap,'destination capacity')
        results=[]
        for c in array(i['cases']):
            op=choice(c.get('op'),['fill','string','scan','logical']);out=initial;first=None;published=False
            if op=='fill':
                keys(c,['op','index','count','value']);at=integer(c['index']);n=integer(c['count'],0,256);value=integer(c['value'],0,255)
                if at<0:status='lower_bound';first=at
                elif at>cap or n>cap-at:status='upper_bound';first=max(at,cap)
                else:status='ok';out=initial[:at]+bytes([value])*n+initial[at+n:];published=True
            elif op=='string':
                keys(c,['op','data_hex']);data=octets(c['data_hex'])
                if 0 in data:status='embedded_nul'
                elif len(data)>=cap:status='capacity';first=cap
                else:status='ok';out=data+b'\0'+initial[len(data)+1:];published=True
            elif op=='scan':
                keys(c,['op','data_hex']);data=octets(c['data_hex']);require(len(data)==cap,'scan extent')
                status='ok' if 0 in data else 'missing_nul';first=None if 0 in data else cap
            else:
                keys(c,['op','index','logical_limit']);at=integer(c['index'],0,cap-1);limit=integer(c['logical_limit'],0,cap)
                status='logical_contract' if at>=limit else 'ok';first=at if at>=limit else None
            results.append(dict(status=status,output_hex=out.hex(),first_invalid_index=first,published=published))
        return dict(results=results)
    if k=='sizes':
        keys(i,['kind','limit','cases']);limit=integer(i['limit'],1,2**32-1);results=[]
        for c in array(i['cases']):
            keys(c,['count','stride','header']);n=integer(c['count']);s=integer(c['stride']);h=integer(c['header']);amount=None
            if not (0<=n<=limit and 1<=s<=limit and 0<=h<=limit):status='input'
            elif n*s>limit:status='multiply'
            elif n*s+h>limit:status='add'
            elif n*s+h==0:status='zero_size'
            else:status='ok';amount=n*s+h
            results.append(dict(status=status,size=amount,allocation_allowed=status=='ok'))
        return dict(results=results)
    if k=='stack':
        keys(i,['kind','frames','interrupts','reserve','extended_index','extended_bytes','pattern_hex','word_bytes','minimum_sp_offset','unknown_dynamic'])
        frames=[integer(x,0,4096) for x in array(i['frames'])];require(frames,'frame path empty');reserve=integer(i['reserve'],1,65536)
        nested=0
        for irq in array(i['interrupts']):
            keys(irq,['saved_frame','callee']);nested+=integer(irq['saved_frame'],0,4096)+integer(irq['callee'],0,4096)
        selected=integer(i['extended_index'],0,len(i['interrupts'])-1);extended=integer(i['extended_bytes'],0,4096)
        basic=sum(frames)+nested;fp=basic-i['interrupts'][selected]['saved_frame']+extended
        pattern=octets(i['pattern_hex']);word=integer(i['word_bytes'],1,8);require(len(pattern)%word==0,'stack word units')
        prefix=next((n for n,b in enumerate(pattern) if b!=165),len(pattern));sp=integer(i['minimum_sp_offset'],0,len(pattern));unknown=boolean(i['unknown_dynamic'])
        return dict(conditional_basic_bytes=basic,basic_margin=reserve-basic,conditional_extended_bytes=fp,extended_margin=reserve-fp,unchanged_prefix_bytes=prefix,reported_free_words=prefix//word,changed_pattern_bytes=len(pattern)-prefix,reserved_by_recorded_sp=len(pattern)-sp,whole_path_bound=None if unknown else basic)
    if k=='ram':
        keys(i,['kind','origin','length','alignment','sections','heap_cases','stack','guard','overflow_bss','startup'])
        origin=integer(i['origin'],0,2**32-1);length=integer(i['length'],1,65536);require(origin+length<=2**32,'32-bit region end');alignment=integer(i['alignment'],1,256);require(alignment&(alignment-1)==0,'power of two alignment')
        sizes=[integer(x,0,65536) for x in array(i['sections'])];require(len(sizes)==3,'data/bss/retained sizes');stack=integer(i['stack'],0,length);guard=integer(i['guard'],0,length)
        def place(s):
            p=origin;r=[]
            for n in s:
                p=((p+alignment-1)//alignment)*alignment;r.append([p-origin,p-origin+n]);p+=n
            return r,p-origin
        sections,end=place(sizes);low=length-stack;gaps=[]
        for h in array(i['heap_cases']):
            h=integer(h,0,65536);gap=low-(end+h);gaps.append(dict(heap_end=end+h,gap=gap,reservation_ok=end<=length and gap>=guard))
        altered=sizes[:];altered[1]=integer(i['overflow_bss'],0,65536);_,overflow_end=place(altered)
        st=i['startup'];keys(st,['initial_hex','load_hex','data_range','bss_range','retained_range','wrong_bss_end'])
        initial=octets(st['initial_hex']);load=octets(st['load_hex'])
        ranges=[]
        for key in ['data_range','bss_range','retained_range']:
            r=array(st[key]);require(len(r)==2,'range pair');a=integer(r[0],0,len(initial));b=integer(r[1],a,len(initial));ranges.append((a,b))
        d,b,r=ranges;require(d[1]-d[0]==len(load),'load range');require(d[1]<=b[0] and b[1]<=r[0],'startup disjoint ranges')
        output=bytearray(initial);output[d[0]:d[1]]=load;output[b[0]:b[1]]=bytes(b[1]-b[0])
        wrong=integer(st['wrong_bss_end'],b[0],len(initial));violation=r[0] if wrong>r[0] else None
        return dict(section_offsets=sections,fixed_bytes=end,stack_low=low,heap_cases=gaps,overflow_end=overflow_end,region_overflow_bytes=max(0,overflow_end-length),startup_hex=output.hex(),wrong_zero_first_retained_index=violation,wrong_zero_rejected=wrong!=b[1])
    if k=='cleanup':
        keys(i,['kind','resource_sizes','failure_indices','graph','retention_budget']);sizes=[integer(x,1,4096) for x in array(i['resource_sizes'])];traces=[]
        for fail in array(i['failure_indices']):
            if fail is not None:integer(fail,0,len(sizes)-1)
            n=len(sizes) if fail is None else fail
            traces.append(dict(acquired=list(range(n)),released=list(reversed(range(n))),peak_bytes=sum(sizes[:n]),live_after_cleanup=0,published=fail is None))
        graph=i['graph'];keys(graph,['nodes','start_roots','interior_roots','edges']);nodes={}
        for n in array(graph['nodes']):
            keys(n,['id','bytes']);require(type(n['id']) is str and n['id'] not in nodes,'node identity');nodes[n['id']]=integer(n['bytes'],1,4096)
        adjacency={n:[] for n in nodes};parents={n:[] for n in nodes}
        for e in array(graph['edges']):
            require(type(e) is list and len(e)==2 and all(type(n) is str and n in nodes for n in e),'graph edge');adjacency[e[0]].append(e[1]);parents[e[1]].append(e[0])
        visiting=set();visited=set()
        def acyclic(n):
            require(n not in visiting,'graph must be acyclic')
            if n in visited:return
            visiting.add(n)
            for child in adjacency[n]:acyclic(child)
            visiting.remove(n);visited.add(n)
        for n in nodes:acyclic(n)
        def reachable(roots):
            roots=array(roots);require(all(type(n) is str and n in nodes for n in roots),'graph roots');seen=set();pending=roots[:]
            while pending:
                n=pending.pop()
                if n not in seen:seen.add(n);pending+=adjacency[n]
            return seen
        start=reachable(graph['start_roots']);interior=reachable(graph['interior_roots'])-start;lost=set(nodes)-start-interior
        classes={n:('reachable' if n in start else 'possible' if n in interior else 'indirect' if any(p in lost for p in parents[n]) else 'definite') for n in nodes}
        totals={name:sum(nodes[n] for n in nodes if classes[n]==name) for name in ['reachable','possible','indirect','definite']};budget=integer(i['retention_budget'],0,4096)
        return dict(cleanup=traces,graph_classes=classes,graph_bytes=totals,reachable_retention_excess=max(0,totals['reachable']-budget))
    if k=='lifetime':
        keys(i,['kind','old_generation','old_address','old_hex','new_size','resize_cases','release_cases']);gen=integer(i['old_generation'],1,100000);address=integer(i['old_address'],0,2**32-1);old=octets(i['old_hex']);size=integer(i['new_size'],1,256);require(old,'nonempty original allocation')
        resize=[]
        for c in array(i['resize_cases']):
            keys(c,['success','new_address']);success=boolean(c['success']);new=integer(c['new_address'],0,2**32-1)
            resize.append(dict(owner_generation=gen+1 if success else gen,owner_address=new if success else address,owner_size=size if success else len(old),old_alias_valid=not success,retained_hex=old[:min(size,len(old))].hex() if success else old.hex(),uninitialized_tail=max(0,size-len(old)) if success else 0))
        releases=[]
        for c in array(i['release_cases']):
            keys(c,['request','live','current_generation','alias_generation','offset']);request=choice(c['request'],['null','owner','alias','foreign']);live=boolean(c['live']);current=integer(c['current_generation'],1,100000);alias=integer(c['alias_generation'],1,100000);offset=integer(c['offset'],0,256)
            status='no_effect' if request=='null' else 'foreign' if request=='foreign' else 'not_live' if not live else 'stale_generation' if request=='alias' and alias!=current else 'borrow_not_owner' if request=='alias' else 'not_base' if offset else 'release'
            releases.append(status)
        return dict(resize=resize,release_results=releases)
    if k=='initialization':
        keys(i,['kind','fields','cases','representation_a_hex','representation_b_hex','semantic_a','semantic_b']);fields=array(i['fields']);require(fields and all(type(x) is str for x in fields) and len(set(fields))==len(fields),'field names');results=[]
        for c in array(i['cases']):
            keys(c,['writes','consume','addressable']);writes=array(c['writes']);require(all(type(x) is str and x in fields for x in writes),'field write');consume=choice(c['consume'],fields);addressable=boolean(c['addressable']);valid=consume in writes
            results.append(dict(initialized=[f in writes for f in fields],consumption_status='unaddressable' if not addressable else 'ok' if valid else 'uninitialized_consumption',publication_allowed=addressable and len(set(writes))==len(fields),address_contract_reports=not addressable))
        for s in [i['semantic_a'],i['semantic_b']]:
            keys(s,fields)
            for n in s.values():integer(n,0,255)
        a=octets(i['representation_a_hex']);b=octets(i['representation_b_hex']);require(len(a)==len(b),'representation sizes')
        return dict(cases=results,fieldwise_equal=i['semantic_a']==i['semantic_b'],provided_representations_equal=a==b)
    if k=='async':
        keys(i,['kind','events','scratch_pairs']);generation=1;active=None;owner_request=None;phase='idle';completed=0;results=[]
        for e in array(i['events']):
            keys(e,['op','request','generation']);op=choice(e['op'],['submit','timeout','cancel_request','reclaim','complete','cancel_ack']);request=integer(e['request'],1,65535);g=integer(e['generation'],1,65535)
            if op=='submit':
                require(active is None and phase=='idle' and g==generation,'submission contract');active=request;owner_request=request;phase='in_flight';verdict='submitted'
            elif op=='reclaim':
                if g!=generation or request!=owner_request:verdict='stale'
                elif active is not None:verdict='busy'
                else:
                    require(phase=='quiescent','reclaim requires quiescent owner');generation+=1;phase='idle';verdict='reclaimed'
            elif active!=request or g!=generation:verdict='stale'
            elif op=='timeout':verdict='waiting_stopped'
            elif op=='cancel_request':phase='cancelling';verdict='cancel_requested'
            else:
                require(op!='cancel_ack' or phase=='cancelling','ack without cancellation');active=None;phase='quiescent';completed+=1;verdict='completed' if op=='complete' else 'cancel_confirmed'
            results.append(dict(verdict=verdict,generation=generation,reclaim_allowed=active is None,completed=completed))
        conflicts=[]
        for p in array(i['scratch_pairs']):
            require(type(p) is list and len(p)==2,'scratch pair');a=integer(p[0],1,64);b=integer(p[1],1,64);conflicts.append(a==b)
        return dict(events=results,scratch_conflicts=conflicts)
    keys(i,['kind','histories']);results=[];labels=set()
    for h in array(i['histories']):
        keys(h,['label','checks','symptom_index']);require(type(h['label']) is str and h['label'] not in labels,'history label');labels.add(h['label']);symptom=integer(h['symptom_index'],0,10000);first=None;reason=None;previous=-1
        for pos,c in enumerate(array(h['checks'])):
            keys(c,['event','invariant','left','right']);event=integer(c['event'],0,symptom);inv=choice(c['invariant'],['less_than','same_generation','initialized','budget']);left=c['left'];right=c['right']
            require(event>previous,'history events must be chronological');previous=event
            if inv=='initialized':
                left=boolean(left);right=boolean(right);ok=left and right
            else:
                left=integer(left,0,65536);right=integer(right,0,65536);ok=left<right if inv=='less_than' else left==right if inv=='same_generation' else left<=right
            if not ok and first is None:first=event;reason=inv
        results.append(dict(label=h['label'],first_invalid_event=first,invariant=reason,symptom_delay=None if first is None else symptom-first))
    return dict(histories=results)

def catalog(folder=FIXTURES,module_path=None,project_folder=None):
    module_path=module_path or ROOT/'curriculum/modules/T45.md';project_folder=project_folder or ROOT/'curriculum/projects'
    expected={f'PR-T45-{n:02}.json' for n in range(1,11)}
    actual={p.relative_to(folder).as_posix() for p in folder.rglob('*') if p.is_file()}
    require(actual==expected,'recursive fixture file set')
    project_expected={f'PR-T45-{n:02}.md' for n in range(1,11)}
    project_actual=set()
    for p in project_folder.rglob('*'):
        if not p.is_file():continue
        if p.name.startswith('PR-T45-') or (p.suffix=='.md' and metadata(p).get('module')=='T45'):
            project_actual.add(p.relative_to(project_folder).as_posix())
    require(project_actual==project_expected,'recursive T45 project file set')
    module=metadata(module_path);require(module['id']=='T45' and module['projects']==[f'PR-T45-{n:02}' for n in range(1,11)],'module project catalog')
    kinds=['observations','extents','sizes','stack','ram','cleanup','lifetime','initialization','async','diagnosis']
    rows=[]
    for n in range(1,11):
        pid=f'PR-T45-{n:02}';path=folder/(pid+'.json');row=strict_json(path.read_text(encoding='utf-8'));keys(row,['id','module','project','description','input','expected','verification_method'])
        require(row['id']==f'FX-MEM-T45-{n:02}' and row['module']=='T45' and row['project']==pid,'fixture metadata binding')
        for key in ['description','verification_method']:
            require(type(row[key]) is str and bool(row[key].strip()),'nonempty oracle metadata string')
        require(type(row['input']) is dict and row['input'].get('kind')==kinds[n-1],'project input kind binding')
        meta=metadata(project_folder/(pid+'.md'));require(meta['id']==pid and meta['module']=='T45' and meta['fixtures']==[f'fixtures/memory_lab/{pid}.json'],'project fixture binding')
        require(meta['topics']==[f'T45.{n:02}'],'project topic binding');rows.append(row)
    require(len({r['id'] for r in rows})==10,'unique fixture IDs');return rows

def controls(rows):
    mutations=[(1,lambda r:r['input'].__setitem__('capacity',True)),(2,lambda r:r['input']['cases'][0].__setitem__('count',True)),(3,lambda r:r['input'].__setitem__('unknown_dynamic',0)),(6,lambda r:r['input']['resize_cases'][0].__setitem__('success',0)),(7,lambda r:r['input']['cases'][0].__setitem__('addressable',1)),(8,lambda r:r['input']['events'][0].__setitem__('request',True)),(9,lambda r:next(c for h in r['input']['histories'] for c in h['checks'] if c['invariant']=='initialized').update(left=False,right=1))]
    rejected=0
    for index,mutate in mutations:
        r=copy.deepcopy(rows[index]);mutate(r)
        try:derive(r['input'])
        except (ValueError,TypeError):rejected+=1
        else:raise ValueError('input type corruption accepted')
    for index,key,value in [(0,'universal_safety_proven',0),(3,'conditional_basic_bytes',True)]:
        r=copy.deepcopy(rows[index]);r['expected'][key]=value
        try:comparison(derive(r['input']),r['expected'])
        except ValueError:rejected+=1
        else:raise ValueError('expected type corruption accepted')
    with tempfile.TemporaryDirectory(prefix='t45_catalog_') as temp:
        target=Path(temp)/'fixtures';target.mkdir();projects=Path(temp)/'projects';projects.mkdir();module=Path(temp)/'T45.md';module.write_bytes((ROOT/'curriculum/modules/T45.md').read_bytes())
        for n,r in enumerate(rows,1):
            pid=f'PR-T45-{n:02}';(target/(pid+'.json')).write_text(json.dumps(r),encoding='utf-8');(projects/(pid+'.md')).write_bytes((ROOT/'curriculum/projects'/f'{pid}.md').read_bytes())
        corrupt=copy.deepcopy(rows[0]);corrupt['id']='FX-MEM-WRONG';(target/'PR-T45-01.json').write_text(json.dumps(corrupt),encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('wrong fixture ID accepted')
        (target/'PR-T45-01.json').write_text(json.dumps(rows[0]),encoding='utf-8')
        text=(projects/'PR-T45-01.md').read_text(encoding='utf-8');(projects/'PR-T45-01.md').write_text(text.replace('fixtures/memory_lab/PR-T45-01.json','fixtures/memory_lab/PR-T45-02.json'),encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('wrong project binding accepted')
        (projects/'PR-T45-01.md').write_text(text,encoding='utf-8');nested=target/'nested';nested.mkdir();(nested/'orphan.json').write_text('{}',encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('nested orphan accepted')
        (nested/'orphan.json').unlink();nested.rmdir()
        extra=projects/'PR-T45-11.md';extra.write_text(text,encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('extra T45 project accepted')
        extra.unlink()
        extra=projects/'PR-EXTRA-01.md';extra.write_text(text,encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('T45 ownership hidden by filename accepted')
        extra.unlink()
        original=json.dumps(rows[0]);duplicate=original.replace('"module": "T45"','"module": "T45", "module": "T45"',1)
        require(duplicate!=original,'duplicate-key control constructed');(target/'PR-T45-01.json').write_text(duplicate,encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('duplicate JSON keys accepted')
        for constant in ['NaN','Infinity','-Infinity']:
            try:strict_json('{"value":'+constant+'}')
            except ValueError:rejected+=1
            else:raise ValueError('non-JSON constant accepted')
        for field,value in [('description',' '),('verification_method',0)]:
            corrupt=copy.deepcopy(rows[0]);corrupt[field]=value;(target/'PR-T45-01.json').write_text(json.dumps(corrupt),encoding='utf-8')
            try:catalog(target,module,projects)
            except ValueError:rejected+=1
            else:raise ValueError('malformed oracle metadata accepted')
        corrupt=copy.deepcopy(rows[0]);corrupt['input']=copy.deepcopy(rows[1]['input']);corrupt['expected']=copy.deepcopy(rows[1]['expected']);(target/'PR-T45-01.json').write_text(json.dumps(corrupt),encoding='utf-8')
        try:catalog(target,module,projects)
        except ValueError:rejected+=1
        else:raise ValueError('swapped project model accepted')
    return rejected

def main():
    try:
        rows=catalog();count=0
        for r in rows:count+=comparison(derive(r['input']),r['expected'])
        rejected=controls(rows)
        print(json.dumps(dict(passed=True,fixtures=len(rows),checked_fields=count,rejected_controls=rejected,learner_c_executed=False,hardware_executed=False)))
        return 0
    except (ValueError,KeyError,TypeError,AssertionError) as exc:
        print(json.dumps(dict(passed=False,error=str(exc))));return 1

if __name__=='__main__':sys.exit(main())
