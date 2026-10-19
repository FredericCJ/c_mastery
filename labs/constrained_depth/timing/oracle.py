"""Lifted-integer chronology and raw observed-host-clock analysis, no target WCET."""
import itertools,json,re
def order(bits,now,deadline):
    modulus=2**bits;half=modulus//2
    candidates=[now-deadline+k*modulus for k in (-1,0,1) if -half<now-deadline+k*modulus<half]
    if not candidates:return 3,99
    if len(candidates)!=1:raise ValueError('Chronology not uniquely admitted')
    distance=candidates[0];return 0,0 if not distance else 1 if distance>0 else 2
def check_replay(text):
    rows=text.splitlines();inputs=[(8,n,d) for n in range(256) for d in range(256)]+[(16,n,d) for n in (0,1,200,32767,32768,65534,65535) for d in (0,1,200,32767,32768,65534,65535)]
    if len(rows)!=len(inputs):raise ValueError('Incomplete chronology matrix')
    for row,case in zip(rows,inputs):
        if row!='O,'+','.join(map(str,(*case,*order(*case)))):raise ValueError('Wrong lifted chronology '+str(case))
    return {'eight_bit_pairs':65536,'sixteen_bit_boundary_pairs':49,'half_range_ambiguous_pairs':256}
def check_measure(text):
    rows=text.splitlines()
    if len(rows)!=52 or rows[-1]!='CT_MEASURE_PASS:actual-host-observations-no-WCET-or-DMA-claim' or not rows[0].startswith('CT_RESOLUTION:'):raise ValueError('Incomplete actual measurement set')
    resolution=re.fullmatch(r'CT_RESOLUTION:wall=(\d+),(\d+) cpu=(\d+),(\d+) units=seconds,nanoseconds',rows[0])
    if not resolution:raise ValueError('Missing actual clock resolution')
    rs=list(map(int,resolution.groups()))
    if any(rs[i]>=1000000000 for i in (1,3)) or not rs[0]+rs[1] or not rs[2]+rs[3]:raise ValueError('Invalid clock resolution')
    patterns=[]
    for pattern in range(2):
        wall=[];cpu=[];samples=rows[1+pattern*25:25+pattern*25]
        for i,row in enumerate(samples):
            kind,p,number,pause,w,c,checksum=row.split(',');p,number,pause,w,c,checksum=map(int,(p,number,pause,w,c,checksum));expected=i+1
            for _ in range(10000):expected=(expected*1664525+1013904223)%2**32
            if kind!='M' or (p,number,pause)!=(pattern,i,int(i>=18) if pattern else int(i%4==0)) or checksum!=expected or w<0 or c<0:raise ValueError('Wrong actual workload identity')
            wall.append(w);cpu.append(c)
        groups=[len(list(g)) for k,g in itertools.groupby(v>1000000 for v in wall) if k];expected=[pattern,min(wall),max(wall),sum(wall),sum(wall)//24,sum(wall)%24,max(groups,default=0),len(groups)]
        if rows[25+pattern*25]!='S,'+','.join(map(str,expected)):raise ValueError('Wrong sample distribution/run summaries')
        patterns.append({'pattern':pattern,'samples':24,'wall_minimum_ns':min(wall),'wall_maximum_ns':max(wall),'wall_sum_ns':sum(wall),'cpu_sum_ns':sum(cpu),'pause_count':6,'exceedance_runs':len(groups),'longest_exceedance_run':max(groups,default=0)})
    return {'clock_resolutions':rs,'patterns':patterns,'limits':'Observed host clocks, explicit sleep/correlated pause; no physical ISR/DMA, IID tail, percentile confidence or WCET guarantee.'}
def interleavings():
    old=(7,0,255,8,1000);new=(7,1,0,8,1000);accepted=broken_mixed=0
    # Reader and writer each have seven SC operations. This finite model is
    # distinct from executing every C/architecture behavior or a weak-order proof.
    for writer_slots in itertools.combinations(range(14),7):
        selected=set(writer_slots);memory=[0,*old];reads=[];w=r=0
        for step in range(14):
            if step in selected:
                if w==0:memory[0]=1
                elif w==6:memory[0]=2
                else:memory[w]=new[w-1]
                w+=1
            else:
                reads.append(memory[0 if r in (0,6) else r]);r+=1
        if reads[0]%2:continue
        snapshot=tuple(reads[1:6])
        if reads[0]==reads[6]:
            accepted+=1
            if snapshot not in (old,new):raise ValueError('SC equal-revision acceptance is torn')
        if snapshot not in (old,new):broken_mixed+=1
    if not broken_mixed:raise ValueError('Missing skip-validation counterexample')
    return {'SC_schedules':3432,'accepted_consistent':accepted,'skip_validation_mixed':broken_mixed,'limits':'One atomic publication and one read attempt; finite SC event model, not weak-order or entire-program model checking.'}
def analysis():return {'conversion_1p5ms_1ms':2,'conversion_1p5ms_250us':6,'eight_bit_order_horizon_ticks_exclusive':128,'sixteen_bit_order_horizon_ticks_exclusive':32768,'sixteen_bit_250us_horizon_ns_exclusive':8192000000,'same_residue_elapsed':[10,266],'epoch_duration_ticks':517,'synthetic_latency':{'n':4,'min':2,'mean_numerator':16,'mean_denominator':4,'max':9},'same_histogram_different_runs':{'sum':37,'n':8,'longest':[1,3],'runs':[3,1]},'epoch_persistence_limit':'A serialized timestamp does not establish boot-incarnation authority or continuity after reset.','publication_model':interleavings()}
if __name__=='__main__':print(json.dumps(analysis(),sort_keys=True))
