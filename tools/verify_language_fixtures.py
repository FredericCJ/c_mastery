"""Independent arithmetic and declared-state oracles for language/API fixtures.

This verifies fixture data and canonical project/file correspondence. It does not
compile learner C or infer implementation behavior from synthetic examples.
Only the Python standard library is needed; run from any working directory.
"""
from __future__ import annotations
import argparse
from collections import Counter
from fractions import Fraction
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
OWNED = {'T01','T02','T03','T04','T05','T06','T12','T18','T19','T20','T21','T22'}
META = re.compile(r'\A```json\s*\n(.*?)\n```',re.S)

def require(condition, message):
    if not condition:
        raise ValueError(message)

def decimal_string(value):
    # Decimal fixture inputs have denominators made only from 2 and 5; convert
    # their exact rational result without a fixed precision context.
    denominator=value.denominator; twos=fives=0
    while denominator%2==0: denominator//=2; twos+=1
    while denominator%5==0: denominator//=5; fives+=1
    require(denominator==1,'nonterminating decimal outside fixture contract')
    places=max(twos,fives); scaled=abs(value.numerator)*(10**places)//value.denominator
    digits=str(scaled).zfill(places+1)
    text=digits if places==0 else (digits[:-places]+'.'+digits[-places:]).rstrip('0').rstrip('.')
    return ('-' if value<0 else '')+text

def recompute(kind, x):
    if kind == 'trace':
        require(x['width']>0 and 0<=x['decimal']<2**x['width'],'invalid finite representation')
        prefix=[]; total=0
        for v in x['values']:
            total+=v; prefix.append(total)
        bits=[1<<n for n in range(x['width']) if x['mask']&(1<<n)]
        require(len(bits)==2,'truth table requires two flag bits')
        vals=sorted([0,bits[0],bits[1],bits[0]|bits[1]])
        boundaries=[]
        for case in x['boundary_cases']:
            minimum=case['minimum']; maximum=case['maximum']
            require(minimum<=0<=maximum,'invalid signed result bounds')
            checked=0; staged=[]; failure=None
            for index,value in enumerate(case['values']):
                candidate=checked+value # Python integers provide the independent mathematical oracle.
                if not minimum<=value<=maximum or not minimum<=candidate<=maximum:
                    failure=index; break
                checked=candidate; staged.append(checked)
            boundaries.append(dict(status='ok' if failure is None else 'range',failure_index=failure,
                                   published_prefix=staged if failure is None else [],mathematical_sum=sum(case['values'])))
        return dict(prefix=prefix,sum=total,binary=format(x['decimal'],f"0{x['width']}b"),hex=format(x['decimal'],'x'),
                    flag_table=[[v,bool(v&x['mask']),(v&x['mask'])==x['mask']] for v in vals],boundary_results=boundaries)
    if kind == 'bounds':
        n=x['length']; require(n>=0,'negative length')
        return dict(element=[0<=i<n for i in x['indices']],formation=[0<=i<=n for i in x['indices']])
    if kind == 'bounded_sum':
        if len(x['tokens'])>x['capacity']: return dict(status='capacity')
        if any(not re.fullmatch(r'[+-]?[0-9]+',s) for s in x['tokens']): return dict(status='invalid')
        values=[int(s) for s in x['tokens']]
        if any(not x['minimum']<=v<=x['maximum'] for v in values): return dict(status='range')
        total=0
        for value in values:
            candidate=total+value
            if not x['minimum']<=candidate<=x['maximum']: return dict(status='range')
            total=candidate
        return dict(status='ok',count=len(values),sum=total)
    if kind == 'symbols':
        linkage=[]; duration=[]
        require(x.get('entity')=='object','storage duration fixture describes objects only')
        for s in x['symbols']:
            require(s['scope'] in ['file','block'],'unsupported object scope')
            linkage.append(('internal' if s['static'] else 'external') if s['scope']=='file' else 'none')
            duration.append('static' if s['scope']=='file' or s['static'] else 'automatic')
        return dict(linkage=linkage,duration=duration)
    if kind == 'mixed_unsigned':
        require(x['width']>0,'invalid unsigned width')
        converted=x['signed']%(1<<x['width'])
        return dict(converted_signed=converted,less_than=converted<x['unsigned'])
    if kind == 'designated':
        values=[0]*x['length']
        for k,v in x['assignments'].items():
            require(0<=int(k)<len(values),'invalid designated index'); values[int(k)]=v
        return dict(values=values)
    if kind == 'diagnostic':
        return dict(categories=['syntax' if not r['syntax_valid'] else 'constraint' if not r['constraints_valid'] else 'runtime' for r in x['cases']])
    if kind == 'alignment':
        a=x['alignment']; require(a>0,'alignment must be positive')
        padding=[(-p)%a for p in x['offsets']]
        return dict(padding=padding,aligned=[p+d for p,d in zip(x['offsets'],padding)])
    if kind == 'matrix':
        require(x['rows']>=0 and x['columns']>=0,'invalid dimensions')
        valid=[0<=r<x['rows'] and 0<=c<x['columns'] for r,c in x['coordinates']]
        return dict(valid=valid,offsets=[r*x['columns']+c if ok else None for (r,c),ok in zip(x['coordinates'],valid)])
    if kind == 'overlap':
        require(all(a<=b for a,b in x['ranges']),'reversed range')
        out=[]
        for i,j in x['pairs']:
            a,b=x['ranges'][i]; c,d=x['ranges'][j]
            out.append(max(a,c)<min(b,d))
        return dict(overlap=out)
    if kind == 'sizes':
        require(x['prefix']>=0 and x['element_size']>0 and all(n>=0 for n in x['counts']),'invalid size operands')
        values=[x['prefix']+x['element_size']*n for n in x['counts']]
        accepted=[v<=x['limit'] for v in values]
        return dict(sizes=[v if ok else None for v,ok in zip(values,accepted)],accepted=accepted)
    if kind == 'resource_events':
        live=set(); violations=[]
        for i,(op,name) in enumerate(x['events']):
            if op=='acquire':
                if name in live: violations.append([i,'duplicate-acquire',name])
                else: live.add(name)
            elif op=='release':
                if name not in live: violations.append([i,'double-release',name])
                else: live.remove(name)
            elif op=='access':
                if name not in live: violations.append([i,'dead-access',name])
            else: raise ValueError('unknown resource operation')
        return dict(violations=violations,live=sorted(live))
    if kind == 'arithmetic':
        lo,hi=x['minimum'],x['maximum']; require(lo<hi,'invalid arithmetic range')
        out=[]
        for op,a,b in x['operations']:
            require(lo<=a<=hi and lo<=b<=hi,'operand outside synthetic range')
            if op=='div':
                if b==0: out.append(dict(status='domain')); continue
                q=(abs(a)//abs(b))*(-1 if (a<0)!=(b<0) else 1)
                out.append(dict(status='ok',quotient=q,remainder=a-q*b) if lo<=q<=hi else dict(status='range'))
            else:
                require(op in ['add','sub','mul'],'unknown arithmetic operation')
                value={'add':lambda:a+b,'sub':lambda:a-b,'mul':lambda:a*b}[op]()
                out.append(dict(status='ok',value=value) if lo<=value<=hi else dict(status='range'))
        return dict(results=out)
    if kind == 'bits_wire':
        w=x['width']; n=x['rotate']; mask=(1<<w)-1
        require(w>0 and 0<=n<w and 0<=x['word']<=mask,'invalid rotation operands')
        v=x['word']; rotated=v if n==0 else ((v<<n)|(v>>(w-n)))&mask
        require(x['wire_bytes']>0 and 0<=x['wire_value']<256**x['wire_bytes'],'invalid wire value')
        wire=[(x['wire_value']>>(8*i))&255 for i in reversed(range(x['wire_bytes']))]
        return dict(rotated=rotated,population=v.bit_count(),bit_width=v.bit_length(),wire=wire)
    if kind == 'fixed':
        scale=x['scale']; require(scale>0 and x['minimum']<x['maximum'],'invalid fixed contract')
        values=[]; saturated=[]
        for a,b in x['pairs']:
            product=a*b; magnitude=abs(product)
            q,r=divmod(magnitude,scale)
            rounded=(q+(2*r>=scale))*(-1 if product<0 else 1)
            result=max(x['minimum'],min(x['maximum'],rounded))
            values.append(result); saturated.append(result!=rounded)
        return dict(values=values,saturated=saturated)
    if kind == 'decimal_reference':
        total=sum(map(Fraction,x['values']),Fraction(0))
        error=abs(Fraction(x['reference'])-Fraction(x['observed']))
        return dict(exact_sum=decimal_string(total),absolute_error=decimal_string(error),accepted=error<=Fraction(x['absolute_tolerance']))
    if kind == 'authority':
        taxonomy={'published edition':'edition','pre-publication draft':'draft','committee proposal':'proposal','implementation manual':'implementation'}
        return dict(classes=[taxonomy[v] for v in x['labels']])
    if kind == 'capability':
        return dict(usable=[all(r['observed'].get(k,False) for k in r['required']) for r in x['features']])
    if kind == 'edition_applicability':
        return dict(introduced=[r['introduced']<=x['selected'] for r in x['claims']],correction_applies=[r['corrected']<=x['selected'] for r in x['claims']])
    if kind == 'migration':
        return dict(ready=[r['fallback'] and r['isolated'] for r in x['choices']])
    if kind == 'macro_model':
        def expand(token):
            seen=set()
            while token in x['replacements']:
                require(token not in seen,'cyclic restricted replacement model')
                seen.add(token); token=x['replacements'][token]
            return token
        return dict(rescanned=expand(x['token']),direct_string=x['spelling'],expanded_string=expand(x['spelling']))
    if kind == 'registry':
        rows=sorted(x['rows'],key=lambda r:r['id'])
        ids=Counter(r['id'] for r in rows); names=Counter(r['name'] for r in rows)
        return dict(ids=[r['id'] for r in rows],names=[r['name'] for r in rows],duplicate_ids=sorted(k for k,v in ids.items() if v>1),duplicate_names=sorted(k for k,v in names.items() if v>1))
    if kind == 'schema':
        masks=[]; used=0; overlap=False
        for f in x['fields']:
            require(f['offset']>=0 and f['width']>0,'invalid bit interval')
            mask=((1<<f['width'])-1)<<f['offset']
            overlap=overlap or bool(used&mask); used|=mask; masks.append(mask)
        enabled=set(x['enabled'])
        valid=not overlap and len({f['name'] for f in x['fields']})==len(x['fields']) and not any(set(pair)<=enabled for pair in x['forbidden'])
        return dict(masks=masks,overlap=overlap,configuration=sorted(enabled),valid=valid)
    if kind == 'bounded_text':
        require(x['text'].isascii(),'fixture length is ASCII bytes')
        require(all(c>=0 for c in x['capacities']),'negative capacity')
        n=len(x['text']); stored=[x['text'][:max(c-1,0)] for c in x['capacities']]
        return dict(required=n,stored=stored,written=list(map(len,stored)),terminated=[c>0 for c in x['capacities']],complete=[n<c for c in x['capacities']])
    if kind == 'utf8':
        states=[]; codepoints=[]
        for sequence in x['sequences']:
            raw=bytes(sequence)
            try:
                text=raw.decode('utf-8',errors='strict'); states.append('valid'); codepoints.append(list(map(ord,text)))
            except UnicodeDecodeError as error:
                states.append('incomplete' if error.reason=='unexpected end of data' and error.end==len(raw) else 'invalid')
                codepoints.append([])
        return dict(states=states,codepoints=codepoints)
    if kind == 'parse_decimal':
        results=[]
        for s in x['tokens']:
            if not re.fullmatch(r'[+-]?[0-9]+',s): results.append(dict(status='invalid')); continue
            value=int(s)
            results.append(dict(status='ok',value=value) if x['minimum']<=value<=x['maximum'] else dict(status='range'))
        return dict(results=results)
    if kind == 'stream_trace':
        output=''; chunks=0; terminal=None
        for e in x['events']:
            require(terminal is None,'event after terminal stream result')
            if e['kind']=='data': output+=e['bytes']; chunks+=1
            else:
                require(e['kind'] in ['eof','error'],'unknown stream result'); terminal=e['kind']
        require(terminal is not None,'unterminated fixture stream')
        return dict(bytes=output,chunks=chunks,terminal=terminal)
    if kind == 'sort_search':
        values=sorted(x['values'])
        return dict(sorted=values,present=[q in values for q in x['queries']])
    if kind == 'append':
        n=len(x['initial'])+len(x['append']); out=[]
        for c in x['capacities']:
            require(c>=len(x['initial']),'invalid initial capacity')
            ok=n<=c; value=x['initial']+x['append'] if ok else x['initial']
            out.append(dict(status='ok' if ok else 'capacity',bytes=value,length=len(value)))
        return dict(results=out)
    if kind == 'generations':
        n=x['owner_generation']+x['reset_increment']
        return dict(before=[h==x['owner_generation'] for h in x['handles']],after=[h==n for h in x['handles']],next_generation=n)
    if kind == 'machine':
        state=x['initial']; states=[]; illegal=[]
        for i,e in enumerate(x['events']):
            k=f'{state}:{e}'
            if k not in x['transitions']: illegal.append(i)
            else: state=x['transitions'][k]
            states.append(state)
        return dict(states=states,illegal=illegal,final=state)
    if kind == 'context_compatibility':
        old=x['old']
        return dict(compatible=[r['timing']==old['timing'] and all(not r[k] or old[k] for k in ['allocate','block','retain']) for r in x['new']])
    if kind == 'callbacks':
        calls=list(x['registered']); removed=set()
        require(len(calls)==len(set(calls)),'duplicate callback identity')
        for name in calls:
            if name in x['remove_during']:
                target=x['remove_during'][name]; require(target in calls,'unknown removal target'); removed.add(target)
        return dict(calls=calls,remaining=[n for n in calls if n not in removed])
    if kind == 'variants':
        rendered=[]; valid=[]
        for r in x['items']:
            tag=r['tag']; ok=tag in ['integer','text','error']; valid.append(ok)
            if not ok: rendered.append(None)
            else:
                require((tag=='text' and isinstance(r['value'],str)) or (tag!='text' and type(r['value']) is int),'tag/payload mismatch')
                rendered.append(f"{'int' if tag=='integer' else tag}:{r['value']}")
        return dict(rendered=rendered,valid=valid)
    if kind == 'iteration':
        visited=[]
        for v in x['values']:
            if v>x['stop_above']: break
            visited.append(v)
        return dict(visited=visited,sum=sum(visited),stale=x['cursor_epoch']!=x['owner_epoch_after'])
    if kind == 'framed_stream':
        require(len(x['payload'])<=255 and all(0<=v<=255 for v in x['payload']),'invalid byte frame')
        frame=[len(x['payload'])]+x['payload']+[sum(x['payload'])%256]
        chunks=[]; offset=0
        for q in x['quotas']:
            require(q>=0,'negative transport quota')
            chunk=frame[offset:offset+q]; chunks.append(chunk); offset+=len(chunk)
        return dict(frame=frame,chunks=chunks,pending=frame[offset:])
    if kind == 'architecture_graph':
        graph={}; cycles=[]; active=[]; done=set()
        for a,b in x['edges']:
            graph.setdefault(a,[]).append(b); graph.setdefault(b,[])
        def visit(n):
            if n in active:
                cycles.append(active[active.index(n):]+[n]); return
            if n in done: return
            active.append(n)
            for child in sorted(graph[n]): visit(child)
            active.pop(); done.add(n)
        for node in sorted(graph): visit(node)
        return dict(acyclic=not cycles,cycles=cycles,transformed=[v*2 for v in x['values']])
    if kind == 'layers':
        mapping={'register read':'driver','pin assignment':'BSP','threshold decision':'service','generic device operation':'HAL'}
        return dict(layers=[mapping[k] for k in x['responsibilities']])
    if kind == 'queue':
        require(x['capacity']>=0 and x['service']>=0 and all(a>=0 for a in x['arrivals']),'invalid queue parameters')
        depth=0; processed=[]; dropped=[]; pending=[]
        for arrivals in x['arrivals']:
            accepted=min(arrivals,x['capacity']-depth); depth+=accepted
            dropped.append(arrivals-accepted); count=min(depth,x['service']); depth-=count
            processed.append(count); pending.append(depth)
        return dict(processed=processed,dropped=dropped,pending=pending)
    if kind == 'supervision':
        require(x['threshold']>0,'invalid timeout threshold')
        streak=0; modes=[]; mode='normal'
        for outcome in x['outcomes']:
            require(outcome in ['ok','timeout','fault'],'unknown adapter result')
            if mode=='stopped': modes.append(mode); continue
            if outcome=='ok': streak=0; mode='normal'
            elif outcome=='fault': streak=0; mode='stopped'
            else:
                streak+=1
                if streak>=x['threshold']: mode='degraded'
            modes.append(mode)
        return dict(modes=modes,final_streak=streak)
    if kind == 'rollback':
        require(0<=x['fail_at']<=len(x['resources']),'invalid failure point')
        acquired=x['resources'][:x['fail_at']]
        return dict(acquired=acquired,released=list(reversed(acquired)),live=[])
    if kind == 'transaction_cancel':
        ok=len(x['proposed'])<=x['capacity']; state=list(x['proposed'] if ok else x['initial'])
        phase='idle'; completions=[]; stale=[]
        for e in x['events']:
            if e=='start': require(phase=='idle','duplicate start'); phase='running'
            elif e=='timeout': require(phase=='running','timeout outside running'); phase='cancelling'
            elif e=='cancel_ack':
                if phase=='cancelling': completions.append('cancelled'); phase='terminal'
                else: stale.append(e)
            elif e in ['complete','late_complete']:
                if phase in ['running','cancelling']: completions.append('completed'); phase='terminal'
                else: stale.append(e)
            else: raise ValueError('unknown cancellation event')
        return dict(update_status='ok' if ok else 'capacity',state=state,completions=completions,stale_events=stale)
    if kind == 'ring':
        require(x['capacity']>0,'ring capacity must be positive')
        return dict(retained=x['records'][-x['capacity']:],dropped=max(0,len(x['records'])-x['capacity']))
    if kind == 'fault_record':
        require(all(0<=v<=255 for v in x['bytes']),'non-byte fault record')
        checksum=sum(x['bytes'])%256
        return dict(checksum=checksum,valid_checksums=[v==checksum for v in x['checksums']],matching_builds=[b==x['expected_build'] for b in x['record_builds']])
    if kind == 'repo_roles':
        rules=[('public/','public'),('src/','private'),('tests/','test'),('examples/','example'),('tools/','tool')]
        roles=[]
        for path in x['paths']:
            matches=[role for prefix,role in rules if path.startswith(prefix)]
            require(len(matches)==1,'unclassified repository path'); roles+=matches
        return dict(roles=roles,counts=dict(Counter(roles)))
    if kind == 'traceability':
        requirements=set(x['requirements']); covered=set(); orphan=[]; unknown=set()
        for test,links in x['tests'].items():
            known=set(links)&requirements; covered|=known; unknown|=set(links)-requirements
            if not known: orphan.append(test)
        return dict(uncovered=sorted(requirements-covered),orphan_tests=sorted(orphan),unknown_requirements=sorted(unknown))
    if kind == 'allocator_ownership':
        owners={}; mismatch=[]
        for i,(op,name,who) in enumerate(x['events']):
            if op=='create': require(name not in owners,'duplicate create'); owners[name]=who
            elif op=='borrow': require(name in owners,'borrow of dead resource')
            elif op=='release':
                require(name in owners,'release of dead resource')
                if owners[name]!=who: mismatch.append([i,name,owners[name],who])
                else: del owners[name]
            else: raise ValueError('unknown ownership operation')
        return dict(mismatches=mismatch,live=sorted(owners))
    if kind == 'release_diff':
        old,new=x['old'],x['new']; common=old.keys()&new.keys()
        return dict(added=sorted(new.keys()-old.keys()),removed=sorted(old.keys()-new.keys()),changed=sorted(k for k in common if old[k]!=new[k]),unchanged=sorted(k for k in common if old[k]==new[k]))
    raise ValueError('unknown oracle: '+kind)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,help='Optional verification receipt path; stdout always reports results.')
    args=parser.parse_args()
    failures=[]; expected_paths={}; seen_ids=set(); count=0; checked_values=0; kinds=Counter()
    for path in sorted((ROOT/'curriculum/projects').glob('PR-T*.md')):
        try:
            match=META.match(path.read_text(encoding='utf-8-sig')); require(match is not None,'missing project metadata')
            meta=json.loads(match[1])
            if meta['module'] not in OWNED: continue
            require(meta['id'] not in seen_ids,'duplicate project id'); seen_ids.add(meta['id'])
            paths=[p for p in meta['fixtures'] if p.startswith('fixtures/language/')]
            require(bool(paths),'owned project has no language fixture')
            for relative in paths:
                require(relative not in expected_paths,'fixture reused without explicit verifier policy')
                target=(ROOT/relative).resolve()
                require(target.is_relative_to((ROOT/'fixtures/language').resolve()),'fixture path escaped language directory')
                expected_paths[relative]=meta['id']
        except Exception as error:
            failures.append(f'{path.name}: {error}')
    actual={str(p.relative_to(ROOT)).replace('\\','/') for p in (ROOT/'fixtures/language').rglob('*') if p.is_file()}
    for relative in sorted(set(expected_paths)-actual): failures.append('missing fixture: '+relative)
    for relative in sorted(actual-set(expected_paths)): failures.append('unreferenced fixture: '+relative)
    for relative in sorted(set(expected_paths)&actual):
        try:
            f=json.loads((ROOT/relative).read_text(encoding='utf-8'))
            require(f['id']==expected_paths[relative],'fixture/project ID mismatch')
            require(all(k in f for k in ['description','oracle','input','expected','verification_method']),'incomplete fixture contract')
            result=recompute(f['oracle'],f['input'])
            # Serialized comparison also distinguishes bool from int and checks every field.
            require(json.dumps(result,sort_keys=True)==json.dumps(f['expected'],sort_keys=True),f'expected mismatch: computed {result!r}')
            count+=1; checked_values+=len(result); kinds[f['oracle']]+=1
        except Exception as error:
            failures.append(f'{relative}: {error}')
    require(bool(expected_paths),'no canonical language projects found')
    report=dict(passed=not failures,projects=len(seen_ids),fixtures=count,expected_fixtures=len(expected_paths),expected_top_level_fields=checked_values,oracle_kinds=dict(sorted(kinds.items())),failures=failures,
                scope='Exact arithmetic, Python strict UTF-8 comparison and declared synthetic state models; canonical file correspondence. No learner-C execution, full-language proof or hardware validation.')
    if args.output:
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,indent=2))
    return 0 if report['passed'] else 1

if __name__=='__main__':
    raise SystemExit(main())
