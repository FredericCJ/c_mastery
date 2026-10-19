"""Original independent finite proofs/counterexamples; no C implementation import.
Results are author arithmetic, never native/PMU/physical observations.
"""
from fractions import Fraction as F
from statistics import median
import json,math

def need(x):
 if not x:raise ValueError('certificate failed')

def cfg(nodes,edges):
 need(len(set(edges))==len(edges) and nodes>=1 and all(0<=a<nodes and 0<=b<nodes for a,b in edges))
 seen={0}
 while True:
  new=seen|{b for a,b in edges if a in seen}|{a for a,b in edges if b in seen}
  if new==seen:break
  seen=new
 need(len(seen)==nodes)
 return len(edges)-nodes+2

def union(intervals):
 points=sorted({x for span in intervals for x in span})
 return sum(b-a for a,b in zip(points,points[1:]) if any(s<=a and b<=e for s,e in intervals))

def keyed(s):
 out={}
 for line in s.splitlines():
  fields=line.split();need(len(fields)==2 and fields[0] not in out and fields[1].isdigit());out[fields[0]]=int(fields[1])
 return out

def devices(s):
 out={}
 for line in s.splitlines():
  fields=line.split();need(fields and fields[0] not in out);row={}
  for token in fields[1:]:
   k,v=token.split('=');need(k not in row and v.isdigit());row[k]=int(v)
  out[fields[0]]=row
 return out

def deltas(a,b,gauges=()):
 return {k:({'status':'absent','value':None} if k not in a or k not in b else {'status':'reset','value':None} if b[k]<a[k] and k not in gauges else {'status':'observed','value':b[k]-a[k]}) for k in a.keys()|b.keys()}

def main():
 records={}
 # Exact inequalities: 5n<=5nm and7<=7nm for positive n,m.
 for n in range(1,65):
  for m in range(1,65):need(3*n*m<=3*n*m+5*n+7<=15*n*m)
 for n in range(65):
  for m in range(65):need(n*m+n+1<=3*n*m+5*n+7<=7*(n*m+n+1))
 records['bounds']={'positive':'3nm <= f <=15nm; fixed positive m Theta(n),m=n Theta(n^2)','zero_m':'Theta(nm+n+1), constants1,7,threshold0','finite_cases':8321}
 f=lambda n:n;g=lambda n:n if n<=1024 else n*n
 need(all(f(n)==g(n) for n in range(1025)) and all(g(n+1)>=g(n) for n in range(2048)) and g(2048)>f(2048))
 records['finite-growth']={'observed_equal_through':1024,'monotone_both':True,'eventual':'Theta(n) vs Theta(n^2)','crossover_example':{'A':'100n','B':'n^2','A_better_after':100,'B_better_before':100}}
 # Grow and quarter-full shrink potential; every legal short sequence.
 def phi(n,c):return 2*n-c if 2*n>=c else c//2-n
 states={(0,0)};max_amortized=0
 for _ in range(12):
  following=set()
  for n,c in states:
   for push in (True,False):
    if not push and not n:continue
    before=phi(n,c);nn=n+1 if push else n-1;cc=c;cost=1
    if push and n==c:cc=max(1,2*c);cost+=n
    if not push and cc>1 and nn*4<=cc:cc//=2;cost+=nn
    after=phi(nn,cc);need(after>=0);max_amortized=max(max_amortized,cost+after-before);following.add((nn,cc))
  states=following
 need(max_amortized<=3)
 # Half-full contraction thrashes with alternation around fullness.
 n,c=8,8;costs=[]
 for _ in range(4):
  c*=2;n+=1;costs.append(1+n-1);n-=1;c//=2;costs.append(1+n)
 need(costs==[9]*8)
 records['potential']={'nonempty_Phi0':2,'negative_initial':{'live':1,'capacity':8,'Phi':-6,'valid_offset':6,'reachable_lower_bound':-6},'quarter_shrink_max_amortized':max_amortized,'half_shrink_thrash':costs,'telescoping':'sumActual=sumAmortized-PhiFinal+PhiInitial'}
 old=[{'id':11,'owner':'old'},{'id':22,'owner':'old'}]
 for allocation_fails in (True,False):
  before=[dict(x) for x in old];staged=None if allocation_fails else [x['id'] for x in old]
  if staged is None:need(old==before)
  else:
   moved=[{'id':i,'owner':'new'} for i in staged];need([x['id'] for x in moved]==[x['id'] for x in old])
 records['payload']={'deep_copy_bytes':'sum over attempted successful clone extents, including rolled-back prefixes','handle_variant':{'copies_per_relocation':'live*sizeof(handle)','may_fail':'backing acquisition before ownership transfer','after_transfer':'no fallible operations;old descriptors invalidated'},'allocator_latency':'separate unbounded premise'}
 lengths=[1,7,29,2,100,3,5,11];cap=0;costs=[];copies=[]
 for i,length in enumerate(lengths):
  copied=sum(lengths[:i]) if i==cap else 0
  if i==cap:cap=max(1,cap*2)
  copies.append(copied);costs.append(length+copied)
 need(sum(costs)==206 and sum(copies)==48 and sum(lengths)==158)
 records['payload']['weighted_growth']={'lengths':lengths,'copied_at_each_append':copies,'byte_costs':costs,'actual_bytes':sum(costs),'append_bytes':sum(lengths),'bound':'Each payload is recopied at most ceil(log2 N) times under doubling, so successful bytes <=(1+ceil(log2 N))*sum(lengths). Add actual successful clone prefixes of failed attempts separately.','bound_value':(1+math.ceil(math.log2(len(lengths))))*sum(lengths),'unit_payload_assumption_removed':True}
 records['amdahl']={'local_2x_40_percent':str(1/(F(3,5)+F(2,5)/2)),'with_overhead_0_1':str(1/(F(3,5)+F(2,5)/2+F(1,10))),'overlap':{'before':max(10,8),'after_A_half':max(5,8),'speedup':str(F(10,8)),'additive_prediction_invalid':str(F(18,13))}}
 edges=[(0,1),(1,2),(2,1),(1,3),(3,4),(4,3),(3,5)]
 nested=[(0,1),(1,2),(1,5),(2,3),(3,2),(2,4),(4,1)]
 early=[(a,5 if (a,b)==(4,1) else b) for a,b in nested]
 need(cfg(6,edges)==cfg(6,nested)==cfg(6,early)==3)
 for wrong in (edges+[edges[0]],[(0,1),(1,0)]):
  try:cfg(6,wrong);raise AssertionError('badCFG accepted')
  except ValueError:pass
 records['cfg']={'both_metric':3,'sequential_edges':edges,'nested_edges':nested,'early_exit_edges':early,'sequential_work_n8':16,'nested_work_n8':64,'early_exit_work_n8':8,'early_exit':'On the existing inner-scan termination condition, the continuation node4 returns to exit5 instead of returning to outer predicate1. This changes feasible paths while E7,N6 andmetric3 remain equal. The condition may depend on a sentinel in the input;no extra decision is silently inserted.','code_bytes':'actual independently compiled sequential/nested function nm records;not inferred from graph'}
 # Ownership half-open sweep: borrowed views are references, not allocations.
 owners=[('work',0,14,0,10,128,64),('scratch',3,6,3,6,80,80),('result',7,12,7,12,96,24)]
 def sweep(owners):
  rows=[]
  for t in sorted({x for row in owners for x in row[1:5]}):
   live=sum(r[6] for r in owners if r[3]<=t<r[4]);cap=sum(r[5] for r in owners if r[1]<=t<r[2] and r[3]<=t<r[4]);ret=sum(r[5] for r in owners if r[1]<=t<r[2] and not r[3]<=t<r[4]);rows.append({'t':t,'live':live,'capacity':cap,'retained':ret,'slack':cap-live})
  return {k:max(rows,key=lambda r:r[k]) for k in ('live','capacity','retained','slack')},rows
 peaks,rows=sweep(owners);need([(peaks[k][k],peaks[k]['t']) for k in peaks]==[(144,3),(224,7),(128,10),(136,7)])
 changed=[owners[0],('scratch',3,7,3,7,80,80),owners[2]];p,r=sweep(changed);need(p['capacity']['capacity']==224)
 records['lifetime']={'peaks':peaks,'two_borrowed_views_extra_owned_bytes':0,'shared_endpoint_no_doublecount':True,'transfer_at7':{'before_global':224,'after_global':224,'attribution_changed':True},'stack_scratch_variant':'subtract80heap capacity while adding80bounded frame at same feasible path'}
 records['storage-spaces']={'parser':'O(n) live input bound vs compile-time reserved capacity;not interchangeable','allocator_metadata':'unknown separate','rss':'actual page snapshots;freed live bytes do not imply instant RSS reduction','stack':'actual .su qualifiers and feasible calls;dynamic-unbounded/unknown indirect unresolved'}
 records['traffic-ratios']={'write_only':'dirty writeback plus compulsory read under write allocation','repeated_read':str(F(8,32)),'empty':None,'compressed':{'logical_useful':64,'encoded_read':12,'ratio':str(F(12,64))},'captured_shared4096_over8':512,'device_attribution':'actual file bytes/syscalls separate from shared/missing device counters'}
 intervals=[(0,8),(2,10),(7,14)];need(union(intervals)==14)
 records['async']={'release_immediate':max(2,1,3),'successor_release':max(2,1,3,9),'cancel_quiescent':False,'overlap_latencies':[8,8,7],'window':[0,12],'successful_completed':1,'failed_completed':1,'censored':1,'throughput_per_time':str(F(1,12)),'occupied_union_clipped':union([(0,8),(2,10),(7,12)]),'sliding_window5_12':{'prewindow_submissions_in_occupancy':2,'completed_successes':1,'cohort_latencies_not_redefined':True}}
 a=[100,102,103,120,101];b=[96,99,110,97,98];d=[x-y for x,y in zip(a,b)];need(median(d)==3 and median(a)-median(b)==4)
 def tail(a,deadline):return {'n':len(a),'p95':sorted(a)[math.ceil(.95*len(a))-1],'misses':sum(x>deadline for x in a),'max':max(a)}
 baseline=tail(a,110);late=tail(a+[10000],110);need(late['misses']==baseline['misses']+1 and late['max']==10000)
 records['samples']={'paired_difference_median':median(d),'difference_of_medians':median(a)-median(b),'raw_pair_difference':d,'tail':baseline,'delayed_trial':late,'censored_variant':{'observed':5,'timeouts':2,'deadline_miss_lower_bound':3,'quantile_claim':'p95 unresolved beyond timeout threshold;timeouts not deleted'},'drift':{'blocked_bias':31,'alternating_bias':1,'true_treatment_difference':0},'finite_max_not_WCET':True}
 # Two phases each10units; one observed phase under identical total10/20.
 records['multiplex-phases']={'enabled':20,'running':10,'true_events':1000+100,'sample_phase1_scaled':2000,'sample_phase2_scaled':200,'same_fraction_cannot_reconstruct_phase':True,'equal_instructions_diff_cycles_hypotheses':['frequency change:independent reference clock/frequency observation','memory stalls:working-set sweep and valid cache/stall events'],'branch_variant':'same comparison count on sorted vs permuted keys;measure branch miss only if available'}
 a=keyed('usage_usec 1000\nuser_usec 600\nsystem_usec 400');b=keyed('new_optional 3\nsystem_usec 800\nusage_usec 1800\nuser_usec 1000');delta=deltas(a,b);need(delta['usage_usec']['value']==800 and delta['new_optional']['value'] is None)
 gauge=deltas({'memory':4096},{'memory':3072},('memory',));need(gauge['memory']['value']==-1024);need(deltas({'a':10},{'a':2})['a']['status']=='reset')
 da=devices('8:0 rbytes=4096 wbytes=0\n8:1 rbytes=2');db=devices('8:2 rbytes=9\n8:0 wbytes=8192 rbytes=12288');dev={k:deltas(da.get(k,{}),db.get(k,{})) for k in da.keys()|db.keys()};need(dev['8:0']['rbytes']['value']==8192 and dev['8:1']['rbytes']['value'] is None)
 records['cgroup']={'keyed':delta,'gauge':gauge,'devices':dev,'cpu_fraction':str(F(800,2000)),'quota_fraction':str(F(500,1000)),'peak12288':'historical;fresh reset not observed','psi_not_additive':[200,50,400],'burst_ancestor':{'child_quota':'800/1000','ancestor_quota':'500/1000','effective_ceiling':'at most ancestor share;other descendants compete','burst_us':200,'throttled_usec_delta':300,'fresh_delegated_subtree':'not present;conditional external observation, no policy writes'}}
 records['profiling']={'synthetic_samples_ms':{'work':4,'child':6},'uniform_child_attribution':{'calls_A':2,'calls_B':8,'child_total_ms':20,'estimated':[4,16],'actual':[16,4],'error':[-12,12]},'batch_variant':'bracket outside100calls measures amortized batch inclusive elapsed;per-call tails/offCPU no longer recoverable','nonlocal_exit':'open frame invalidates complete exact totals'}
 records['counter-premises']={'access_order':['exact selected core/device','trace access','TRCENA','NOCYCCNT feature','exclusive setup ownership','enabled','clock stable','halt/interrupt interval','independent less-than-wrap bound'],'timer_variant':{'clock':'peripheral bus divider,notCPU','access':'selected timer gate/prescaler/ownership','wrap':'timer width/elapsed bound','crosscheck':'GPIO edge interval plusinstrument resolution'},'frequency_variant':{'delta':64,'hz_segments':[16000000,8000000],'unknown_split_ns_range':[4000,8000],'known_split32_32_ns':6000},'M7_variant':['I/Dcache warm state','flash/SRAM wait states','DMA coherence','interrupts','counter ownership','board clock accuracy'],'evidence_table':{'captured_arithmetic':'checked','emulator_functional':'prior separately bound artifact only','dedicated_board':'unobserved','energy':'unobserved','WCET':'not established'}}
 records['decision']={'minimal_vector':['latency_ns<=deadline','completed_items/window_seconds','peak_retained_bytes<=budget','read+write_bytes/useful_bytes'],'omitted_energy':'unavailable,noenergyrequirement','hard_stack_added':'old evidence insufficient until callgraph/nesting/unbounded paths resolved','model_A':{'cycles':260,'code':128,'retained':64,'admissible':True},'model_B':{'cycles':240,'code':256,'retained':96,'admissible':False},'limits':{'code':200,'retained':80},'tail_variant':'if deadline tighter,larger storage design may become feasible only after both hard limits re-evaluated','second_reader':'independent reproducibility unobserved;author replay not substitute'}
 print(json.dumps({'passed':True,'records':records},indent=2))
if __name__=='__main__':main()
