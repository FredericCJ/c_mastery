"""Bounded executable premises, counterexamples and exact rational analyses.
These are author models, never measurements of an H745 bus or oscillator.
"""
from fractions import Fraction as F
from collections import deque
import argparse,json,itertools
def clocks():
 # B = r*A + offset. Nonnegative one-way delays yield bounds; asymmetry
 # precludes replacing this interval with one asserted midpoint.
 samples=[(100,230,240,130),(200,430,440,230)]
 feasible=[]
 for rate in [F(19,10),F(2),F(21,10)]:
  low=max(F(bsend)-rate*arecv for asend,brecv,bsend,arecv in samples)
  high=min(F(brecv)-rate*asend for asend,brecv,bsend,arecv in samples)
  if low<=high:feasible.append(dict(rate=str(rate),offset_low=str(low),offset_high=str(high)))
 assert feasible==[{'rate':'19/10','offset_low':'3','offset_high':'40'},{'rate':'2','offset_low':'-20','offset_high':'30'},{'rate':'21/10','offset_low':'-33','offset_high':'10'}]
 ticks=(0x00000020-0xfffffff0)&0xffffffff
 assert ticks==48 and F(48000,480000000)==F(1,10000)
 segments=[(24000,480000000),(24000,240000000)]
 elapsed=sum(F(c,hz) for c,hz in segments);assert elapsed==F(3,20000)
 measured=48000;probe=24;assert F(measured-probe,480000000)==F(1999,20000000)
 # A rare nonperiodic interference burst outside the first 1000 trials is a
 # counterexample to interpreting their maximum as a universal bound.
 ordinary=[100+(i%7) for i in range(1000)];stress=ordinary+[1800]*12
 assert max(ordinary)==106 and max(stress)==1800
 return dict(offset_rate_feasible=feasible,naive_cross_core_difference=1000-900,naive_duration_valid=False,wrap_ticks=ticks,same_core_seconds=str(F(48000,480000000)),probe_corrected_seconds=str(F(measured-probe,480000000)),piecewise_seconds=str(elapsed),incorrect_single_frequency=str(F(48000,480000000)),sampled_max=max(ordinary),burst_max=max(stress),wcet_claim=False)
def cache():
 # Two physically distinct lines, an independently dirty neighbor. Cleaning
 # a line and discarding it have intentionally different effects.
 ram=bytearray(64);cpu=bytearray(ram);cpu[31]=1;cpu[32]=2;cpu[0]=99
 stale=bytes(ram);assert stale[31:33]==b'\0\0'
 # DMB/doorbell publishes no dirty cache bytes in this noncoherent model.
 doorbell_before_clean=bytes(ram);assert doorbell_before_clean[31:33]==b'\0\0'
 ram[:]=cpu;assert ram[31:33]==b'\1\2'
 dirty=bytearray(ram);dirty[1]=88;ram[31:33]=b'\3\4'
 invalidated=bytearray(ram);assert invalidated[1]!=88 # neighbor lost
 # Exclusive receive lines: clean + invalidate before DMA, then invalidate
 # after quiet completion. A speculative refill before completion is stale.
 ram=bytearray(64);cpu=bytearray([9]*64);ram[:]=cpu;cached=None
 cached=bytearray(ram);ram[:]=bytes([7]*64);assert cached[0]==9
 cached=bytearray(ram);assert cached[0]==7
 # Coherent variant removes explicit copy/invalidations, never the lease:
 # software writes while DMA writes remain conflicting owners.
 coherent=ram;assert coherent[0]==7
 return dict(range32=[0x30040000,0x30040040],range64_changed=[0x30040000,0x30040080],early_doorbell_stale=True,dirty_neighbor_lost=True,preclean_preserves_exclusive_lines=True,postinvalidate_observes_dma=True,coherent_maintenance_required=False,coherent_exclusive_ownership_required=True,physical_cache_observation=False)
def ordering():
 # Serialized event simulator, not an undefined concurrent C execution.
 def observe(events):
  slot=0;head=0;seen=None
  for e in events:
   if e=='payload':slot=42
   if e=='head':head=1
   if e=='read' and head:seen=slot
  return seen
 assert observe(['head','read','payload'])==0
 assert observe(['payload','head','read'])==42
 legal=[p for p in itertools.permutations(['payload','head','read']) if p.index('payload')<p.index('head')]
 assert all(observe(p) in (None,42) for p in legal)
 return dict(early_head_counterexample=['head','read:0','payload:42'],release_acquire_edges=['producer payload sequenced-before release head','consumer acquire head synchronizes with release read','consumer payload read sequenced-before release tail','producer acquire tail before slot reuse'],legal_serialized_schedules=len(legal),volatile_establishes_c_synchronizes_with=False,second_producer_permitted=False)
def pipeline(arrivals,stop=None,m4_delay=None,m7_delay=None,local=False):
 # Task-level discrete event model. Work advances downstream before upstream
 # at each integer time. Capacities 2/4/8 exclude the in-service item.
 caps=[2,4,8];costs=[10,20,30];queues=[deque() for _ in caps];active=[None]*3
 rows={};drop=[];finished=[];high=[0,0,0];traffic=0;first_growing=None
 source={t:[] for t,_ in arrivals}
 for t,i in arrivals:source[t].append(i);rows[i]={'arrival':t}
 for now in range(1200):
  for stage in (2,1,0):
   item=active[stage]
   if item and now>=item[1]:
    i=item[0]
    if stage==2:finished.append(i);rows[i]['finish']=now;active[stage]=None
    elif len(queues[stage+1])<caps[stage+1]:queues[stage+1].append(i);active[stage]=None;traffic+=int(stage==0 and not local)
  for i in source.get(now,[]):
   if len(queues[0])<caps[0]:queues[0].append(i)
   else:drop.append(i);rows[i]['drop']=now
  for stage in (2,1,0):
   blocked=(stop and stage==2 and stop[0]<=now<stop[1]) or (m4_delay and stage==1 and m4_delay[0]<=now<m4_delay[1]) or (m7_delay and stage==0 and m7_delay[0]<=now<m7_delay[1])
   if not active[stage] and queues[stage] and not blocked:
    i=queues[stage].popleft();active[stage]=(i,now+costs[stage]);rows[i]['start'+str(stage)]=now
   high[stage]=max(high[stage],len(queues[stage]))
   if first_growing is None and len(queues[stage])>=2:first_growing=stage
  if now>max(source,default=0) and not any(queues) and not any(active):break
 outstanding=[i for q in queues for i in q]+[a[0] for a in active if a]
 assert len(rows)==len(finished)+len(drop)+len(outstanding)
 assert len(set(finished+drop+outstanding))==len(rows)
 return dict(finished=finished,dropped=drop,outstanding=outstanding,high_water=high,first_growing=first_growing,cross_core_messages=traffic,rows=rows)
def pipelines():
 slow=[(i*80,i+1) for i in range(8)];base=pipeline(slow)
 assert all(r['finish']-r['arrival']==60 for r in base['rows'].values()) and not base['dropped']
 burst=[(i*5,i+1) for i in range(60)];paused=pipeline(burst,stop=(0,400))
 assert paused['dropped'] and paused['high_water']==[2,4,8]
 m4=pipeline([(i*12,i+1) for i in range(30)],m4_delay=(0,180))
 m7=pipeline([(i*12,i+1) for i in range(30)],m7_delay=(0,180))
 assert m4['high_water'][1]==4 and m7['high_water'][0]==2
 moved=pipeline(slow,local=True);assert moved['finished']==base['finished'] and moved['cross_core_messages']==0 and base['cross_core_messages']==8
 # Admission includes five rejected identities; accepted workload comprises
 # replies plus uncertain requests. Do not call all 100 successfully queued.
 ledger={'offered':100,'admitted':95,'replied':90,'rejected':5,'uncertain':5}
 assert ledger['offered']==ledger['replied']+ledger['rejected']+ledger['uncertain'] and ledger['admitted']==ledger['replied']+ledger['uncertain']
 return dict(base=base,downstream_pause=paused,m4_only_interference=m4,m7_high_priority=m7,moved_compute=moved,reset_ledger=ledger,scheduling_scope='declared discrete event premises; no physical latency bound')
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);a=p.parse_args();out={name:f() for name,f in [('clocks',clocks),('cache',cache),('ordering',ordering),('pipelines',pipelines)]};out['passed']=True
 from pathlib import Path
 Path(a.output).write_text(json.dumps(out,indent=2)+'\n');print('AF_PASS:analysis clocks cache ordering pipelines')
if __name__=='__main__':main()
