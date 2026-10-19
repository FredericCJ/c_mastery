"""Original finite derivations for explicitly analysis-only T15 obligations.

These are reference-assessor artifacts, not independent learner evidence.
Python big integers/Fraction are host-side proof and oracle tools only.
"""
from fractions import Fraction as F
import itertools,json,math

def midpoint():
 maximum=(1<<64)-1;lo=maximum-8;hi=maximum-1
 safe=lo+(hi-lo)//2;wrapped=((lo+hi)&maximum)//2
 assert lo<=safe<hi and wrapped<lo
 return dict(lo=lo,hi=hi,safe=safe,wrapped=wrapped,real_array_claim=False)

def rank_models():
 # Worst-case first-element distinct pivots: one-item progress is compatible
 # with the abstract partition contract. This recurrence is not a benchmark.
 costs=[sum(range(1,n)) for n in range(1,65)]
 assert costs[-1]==2016
 # Three-way equality is essential when transferring the distinct-key source
 # proof to this curriculum's duplicate domain.
 medians=[]
 for values in itertools.product(range(3),repeat=5):
  x=sorted(values)[2];assert sum(v<=x for v in values)>=3 and sum(v>=x for v in values)>=3;medians.append(x)
 # Ideal recurrence:1/5 +7/10 =9/10 leaves linear slack. Rounding and small
 # subproblems still require a separate additive/base-case constant.
 assert F(1,5)+F(7,10)==F(9,10)
 return dict(worst_distinct_partition_cost_n64=costs[-1],five_group_assignments=len(medians),linear_slack='1/10',randomized_claim='Requires fresh pivot distribution; deterministic ad_select does not acquire expected-linear evidence from random-looking tests.')

def storage_costs():
 # Ten records, each key+label conceptual8 bytes; size_t counter8 bytes in
 # the declared host model. Exact sizeof/alignment belongs in measured ledger.
 small=16*8+10*8;large=(1<<32)*8+10*8
 assert small==208 and large==34359738448
 # An irrevocable sorted-output stream cannot emit5 after prefix[5] while a
 # future1 remains admissible. Retention/replay/external runs change contract.
 assert sorted([5,1])!=[5,1]
 return dict(counting_bytes_range16=small,counting_bytes_range2pow32=large,radix4_counters=16,radix8_counters=256,stream_counterexample=[[5],[5,1]])

def paths():
 edges=[(0,1,2),(0,2,5),(2,1,-10)]
 def bellman(n,edges):
  distance=[None]*n;distance[0]=0
  for _ in range(n-1):
   for u,v,w in edges:
    if distance[u] is not None and (distance[v] is None or distance[u]+w<distance[v]):distance[v]=distance[u]+w
  cycle=any(distance[u] is not None and (distance[v] is None or distance[u]+w<distance[v]) for u,v,w in edges)
  return distance,cycle
 def finalizing_dijkstra(n,edges):
  distance=[None]*n;distance[0]=0;done=set()
  while True:
   candidates=[v for v in range(n) if v not in done and distance[v] is not None]
   if not candidates:break
   u=min(candidates,key=lambda v:(distance[v],v));done.add(u)
   for a,b,w in edges:
    if a==u and b not in done and (distance[b] is None or distance[u]+w<distance[b]):distance[b]=distance[u]+w
  return distance
 correct,cycle=bellman(3,edges);wrong=finalizing_dijkstra(3,edges)
 assert correct==[0,-5,5] and not cycle and wrong==[0,2,5]
 assert bellman(3,edges+[(1,2,1)])[1]
 assert not bellman(4,[(0,1,2),(2,3,-1),(3,2,0)])[1]
 return dict(nonnegative_precondition_counterexample=dict(edges=edges,bellman=correct,finalizing_dijkstra=wrong),reachable_cycle_rejected=True,unreachable_cycle_distinguished=True,complexity='Edge-list Bellman-Ford O(VE); scanning all adjacency slots each pass adds O(V^2).')

def greedy():
 intervals=[(0,2,1),(2,4,1),(0,4,10)]
 greedy_value=2;weighted_optimum=max(sum(intervals[i][2] for i in range(3) if mask>>i&1) for mask in range(8) if all(not(mask>>i&1 and mask>>j&1) or intervals[i][1]<=intervals[j][0] or intervals[j][1]<=intervals[i][0] for i in range(3) for j in range(i+1,3)))
 assert weighted_optimum==10>greedy_value
 return dict(weighted_counterexample=intervals,earliest_finish_weight=greedy_value,weighted_optimum=weighted_optimum,exchange='Replacing the first selected positive-length interval by one finishing no later preserves every later feasible interval and cardinality, but not arbitrary total weight.')

def toy_float():
 # Original binary format p3, emin=-2, emax=2, unsigned magnitudes. It is NOT
 # IEEE binary32/binary64 or an implementation of production decimal parsing.
 values={F(0):(0,-4)}
 for significand in range(1,4):values[F(significand,16)]=(significand,-4)
 for exponent in range(-2,3):
  for significand in range(4,8):values[F(significand)*F(2)**(exponent-2)]=(significand,exponent-2)
 def brute(x):
  if x>=F(15,2):return None
  distance=min(abs(x-v) for v in values)
  tied=[v for v in values if abs(x-v)==distance]
  return min(tied,key=lambda v:values[v][0]%2)
 def quotient(x):
  if x>=F(15,2):return None
  exponent=-2
  while exponent<2 and x>=F(2)**(exponent+1):exponent+=1
  step=F(2)**(exponent-2);scaled=x/step;q,r=divmod(scaled.numerator,scaled.denominator)
  if r*2>scaled.denominator or (r*2==scaled.denominator and q%2):q+=1
  return q*step
 for n in range(8001):assert brute(F(n,1000))==quotient(F(n,1000))
 for value in values:assert brute(value)==value
 assert brute(F(1,32))==0 and brute(F(3,32))==F(1,8)
 assert brute(F(1,10))==F(1,8) and F(1,10)!=F(1,8)
 assert brute(F('0.1'))==brute(F('0.125'))==F(1,8)
 return dict(format='p3 emin-2 emax2; subnormals step1/16; nearest ties-even; overflow threshold15/2',rational_cases=8001,one_tenth='rounds to1/8',shortest_counterexample='0.1 and0.125 both round to1/8; exact decimal expansion is a different contract',limits='Production conversion, all IEEE exceptional encodings, rounding modes and shortest-string uniqueness are not implemented by this toy dossier.')

def nested():
 # Original grammar: leaf0,value; container1,length,children. Entire input is
 # a sequence. Every byte is charged once and container payloads are bounded.
 def decode(data,max_depth=3,budget=64):
  used=0;leaves=[]
  def sequence(start,end,depth):
   nonlocal used
   if depth>max_depth:raise ValueError('depth')
   at=start
   while at<end:
    if end-at<2:raise ValueError('header')
    tag,count=data[at:at+2];at+=2;used+=2
    if used>budget:raise ValueError('budget')
    if tag==0:leaves.append(count)
    elif tag==1:
     if count>end-at:raise ValueError('extent')
     sequence(at,at+count,depth+1);at+=count
    else:raise ValueError('tag')
  sequence(0,len(data),0);return leaves,used
 def nest(data):return bytes([1,len(data)])+data
 leaf=bytes([0,42]);three=nest(nest(nest(leaf)));four=nest(three)
 assert decode(three)==([42],8)
 for data,depth,budget,reason in [(four,3,64,'depth'),(three,3,7,'budget'),(bytes([1,3,0,2]),3,64,'extent')]:
  try:decode(data,depth,budget)
  except ValueError as e:assert str(e)==reason
  else:raise AssertionError('missed '+reason)
 return dict(depth3_bytes=list(three),independent_limits=['depth','budget','extent'],reset_scope='Pure isolated parser invocation; no hardware reset or network target.')

def representation_and_crc():
 values=range(-127,128);mapping=[value+127 for value in values];assert mapping==list(range(255))
 def reverse(x):return int(f"{x:08b}"[::-1],2)
 def polynomial(data,initial):
  message=0
  for byte in data:message=(message<<8)|byte
  message=(message<<8)^(initial<<(8*len(data)))
  while message.bit_length()>8:message ^= 0x107 << (message.bit_length()-9)
  return message
 # Initial register is shifted through each message octet, not appended twice.
 data=b"123456789";normal=polynomial(data,0);assert normal==0xf4
 reflected=reverse(polynomial(bytes(map(reverse,data)),255))^255;assert reflected==0x2f
 return dict(sign_magnitude_value_map="-127..127 ->0..254 after zero normalization; no fictitious non-two-complement int32_t",normal_crc=normal,reflected_init255_xor255=reflected)

def main():
 print(json.dumps(dict(representation_and_crc=representation_and_crc(),midpoint=midpoint(),selection=rank_models(),storage=storage_costs(),negative_paths=paths(),weighted_greedy=greedy(),decimal_binary_dossier=toy_float(),nested_frame_budget=nested()),sort_keys=True))
if __name__=='__main__':main()
