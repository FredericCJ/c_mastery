"""Check emitted centroids with high-precision scale arithmetic and raw-data ranks."""
import argparse,bisect,decimal,json,math,random,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
D=decimal.Decimal;decimal.getcontext().prec=50

def size(delta,n,left,weight):
    if weight==1:return D(0)
    if left==0 or left+weight==n:return D('Infinity')
    right=left+weight
    return D(delta)/(4*(D(n)/D(delta)).ln()+24)*((D(right)/D(left)).ln()+(D(n-left)/D(n-right)).ln())

def components(centroids):
    atoms={};intervals=[]
    for mean,w in centroids:
        if w==1:atoms[mean]=atoms.get(mean,0)+1
    for (a,wa),(b,wb) in zip(centroids,centroids[1:]):
        mass=(wa/2 if wa>1 else 0)+(wb/2 if wb>1 else 0)
        if a==b:atoms[a]=atoms.get(a,0)+mass
        elif mass:intervals.append((a,b,mass))
    return atoms,intervals

def reference_quantile(centroids,q,n):
    if q==0:return centroids[0][0]
    if q==1:return centroids[-1][0]
    atoms,intervals=components(centroids);events=sorted(set(atoms)|{v for a,b,_ in intervals for v in (a,b)})
    target=q*n;seen=0
    for index,x in enumerate(events):
        seen+=atoms.get(x,0)
        if target<=seen:return x
        if index+1<len(events):
            y=events[index+1];density=sum(m/(b-a) for a,b,m in intervals if a<=x and y<=b)
            mass=density*(y-x)
            if mass and target<=seen+mass:return x+(target-seen)/density
            seen+=mass
    return events[-1]

def compress(items,delta):
    # Select maximal admissible sorted prefixes, then compute their weighted
    # means with fsum. This is independent of the C rolling-mean representation.
    items=sorted(items,key=lambda item:item[0]);n=sum(w for _,w in items);out=[];left=0;start=0
    factor=delta/(4*math.log(n/delta)+24) if n else 0
    while start<len(items):
        end=start+1;weight=items[start][1]
        while end<len(items):
            candidate=weight+items[end][1];right=left+candidate
            if left==0 or right==n:break
            k=factor*(math.log(right/left)+math.log((n-left)/(n-right)))
            if k>1-1e-10:break
            weight=candidate;end+=1
        out.append((math.fsum(x*w for x,w in items[start:end])/weight,weight));left+=weight;start=end
    return out

def reference_state(case):
    values=case['values'];n=len(values);parts=case['parts'];delta=case['delta'];batch=case['batch'];leaves=[]
    for i in range(parts):
        state=[];begin=n*i//parts;end=n*(i+1)//parts
        for j in range(begin,end,batch):state=compress(state+[(float(v),1) for v in values[j:min(end,j+batch)]],delta)
        leaves.append(state)
    if case['mode']==2:
        stride=1
        while stride<parts:
            for i in range(0,parts,2*stride):
                if i+stride<parts:leaves[i]=compress(leaves[i]+leaves[i+stride],delta)
            stride*=2
        return compress(leaves[0],case['target'])
    result=[]
    for state in leaves[::(-1 if case['mode'] else 1)]:result=compress(result+state,case['target'])
    return result

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver');a=parser.parse_args()
    fixture=json.loads((ROOT/'fixtures/corpus_tdigest/cases.json').read_text());rng=random.Random(fixture['seed']);n=fixture['sample_count'];queries=fixture['queries']
    workloads={'ramp':list(range(n)),'squares':[i*i for i in range(n)],'repeated':[i%7 for i in range(n)],'tail':[0 if i<n-8 else (i-n+9)*1000000 for i in range(n)],'symmetric':[(-1 if i%2 else 1)*i*i for i in range(n)]}
    commands=[];cases=[]
    for name,original in workloads.items():
        for order in ['ascending','descending','shuffled']:
            values=sorted(original,reverse=order=='descending')
            if order=='shuffled':rng.shuffle(values)
            for delta in fixture['leaf_compression']:
                for target in fixture['target_compression']:
                    if target>delta:continue
                    for batch in fixture['batches']:
                        for parts in fixture['partitions']:
                            for mode in range(3):
                                if parts==1 and mode:continue
                                commands.append(' '.join(map(str,[delta,target,batch,parts,mode,n,*values])))
                                cases.append(dict(workload=name,order=order,delta=delta,target=target,batch=batch,parts=parts,mode=mode,values=values))
            # Fresh one-batch construction has no previous rounded-centroid
            # ordering. Check its entire greedy partition independently.
            for delta in fixture['leaf_compression']:
                small=values[:64]
                commands.append(' '.join(map(str,[delta,delta,64,1,2,len(small),*small])))
                cases.append(dict(workload=name,order=order,delta=delta,target=delta,batch=64,parts=1,mode=2,values=small,exact_prefix_probe=True))
    assert len(cases)==570
    if not a.driver:
        print(json.dumps(dict(passed=True,executed=False,cases=len(cases),queries=queries)));return
    run=subprocess.run([a.driver],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=180)
    assert run.returncode==0,(run.returncode,run.stderr)
    lines=run.stdout.splitlines();assert len(lines)==len(cases),(len(lines),len(cases))
    comparisons=0;max_error=0.;error_row=None;max_value_error=0.;centroid_counts=[];rows=[];order_witness=None;groups={};prefix_probes=0;rounding_divergences=0;rounding_example=None
    for line,case in zip(lines,cases):
        n=len(case['values'])
        tokens=line.split();used=int(tokens[0]);count=int(tokens[1]);assert 0<used<=128 and count==n;centroid_counts.append(used)
        centroids=[(float(tokens[2+2*i]),int(tokens[3+2*i])) for i in range(used)];tail=list(map(float,tokens[2+2*used:]));assert len(tail)==2*len(queries)
        reference=reference_state(case)
        matched=len(reference)==used and all(w==rw and math.isclose(x,rx,rel_tol=3e-13,abs_tol=1e-7) for (x,w),(rx,rw) in zip(centroids,reference))
        if case.get('exact_prefix_probe'):
            assert matched,(case,centroids,reference);prefix_probes+=1;comparisons+=2*used
        elif not matched:
            rounding_divergences+=1
            if rounding_example is None:rounding_example=dict(configuration={k:v for k,v in case.items() if k!='values'},actual_centroids=centroids,fsum_prefix_centroids=reference,meaning='Different rounding paths can order nearly equal stored means differently, changing later greedy groups. This comparison is an investigation, not a failed exact cross-arithmetic contract.')
        assert [x for x,_ in centroids]==sorted(x for x,_ in centroids)
        assert sum(w for _,w in centroids)==n and centroids[0][1]==centroids[-1][1]==1
        raw=sorted(case['values']);assert centroids[0][0]==raw[0] and centroids[-1][0]==raw[-1]
        assert math.isclose(math.fsum(x*w for x,w in centroids),math.fsum(raw),rel_tol=2e-14,abs_tol=1e-6)
        left=0
        for mean,w in centroids:
            assert w>0 and math.isfinite(mean) and size(case['target'],n,left,w)<=1+D('3e-13'),(case,mean,w,left,size(case['target'],n,left,w))
            left+=w;comparisons+=1
        atoms,intervals=components(centroids)
        fitted=[]
        for i,q in enumerate(queries):
            value,cdf=tail[2*i:2*i+2];want=reference_quantile(centroids,q,n)
            assert math.isclose(value,want,rel_tol=2e-13,abs_tol=1e-8),(case,q,value,want)
            want_cdf=(sum(w for x,w in atoms.items() if x<=value)+sum(m*min(1.,max(0.,(value-lo)/(hi-lo))) for lo,hi,m in intervals))/n
            assert math.isclose(cdf,want_cdf,rel_tol=2e-13,abs_tol=1e-13),(case,q,cdf,want_cdf)
            assert raw[0]<=value<=raw[-1] and 0<=cdf<=1
            rank_lo=bisect.bisect_left(raw,value)/n;rank_hi=bisect.bisect_right(raw,value)/n
            error=max(rank_lo-q,q-rank_hi,0)
            exact=raw[max(0,math.ceil(q*n)-1)];value_error=abs(value-exact)
            if error>max_error:max_error=error;error_row={k:v for k,v in case.items() if k!='values'}|dict(q=q,estimate=value,rank_interval=[rank_lo,rank_hi],error=error)
            max_value_error=max(max_value_error,value_error);fitted.append(value);comparisons+=2
        assert fitted==sorted(fitted)
        key=tuple(case[k] for k in ['workload','order','delta','target','batch','parts']);signature=tuple(fitted)
        if key in groups and groups[key]['quantiles']!=signature and order_witness is None:
            previous=groups[key];order_witness=dict(configuration=dict(zip(['workload','order','delta','target','batch','parts'],key)),first_mode=previous['mode'],second_mode=case['mode'],queries=queries,first_quantiles=list(previous['quantiles']),second_quantiles=fitted,raw_values=case['values'])
        groups.setdefault(key,dict(mode=case['mode'],quantiles=signature))
        rows.append(dict(configuration={k:v for k,v in case.items() if k!='values'},centroids=used,quantiles=fitted))
    assert order_witness and max_error>0 and min(centroid_counts)<64 and prefix_probes==30 and rounding_example
    print(json.dumps(dict(passed=True,executed=True,cases=len(cases),comparisons=comparisons,exact_fresh_prefix_probes=prefix_probes,reference_arithmetic_divergences=rounding_divergences,reference_arithmetic_example=rounding_example,high_precision_scale_checks=True,rank_error_definition='distance from requested q to exact empirical [strict-less,less-or-equal] rank interval at returned value',maximum_observed_rank_error=max_error,maximum_observed_value_error=max_value_error,worst_rank_case=error_row,centroid_count_range=[min(centroid_counts),max(centroid_counts)],merge_order_counterexample=order_witness,limits='These are observed finite errors, not a worst-case error theorem or unbiasedness claim. Centroid invariant checks do not recover lost samples. Complete independent construction equality is asserted for fresh batches; different-rounding history divergence is retained explicitly.')))
if __name__=='__main__':main()
