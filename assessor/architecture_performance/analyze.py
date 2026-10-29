"""Retain every native sample and block; separate work counts and clock limits."""
from fractions import Fraction as F
import collections,json,pathlib,statistics,sys
path=pathlib.Path(sys.argv[1]);lines=path.read_text().splitlines();samples=[list(map(int,s.split()[1:])) for s in lines if s.startswith('SAMPLE ')];overhead=[list(map(int,s.split()[1:])) for s in lines if s.startswith('OVERHEAD ')];clock=[list(map(int,s.split()[1:])) for s in lines if s.startswith('CLOCK ')];work=[list(map(int,s.split()[1:])) for s in lines if s.startswith('WORK ')];assert len(samples)==248 and len(overhead)==101 and len(clock)==1 and 'PE_MEASURE_PASS raw=248 no_speed_threshold' in lines
def pair(x):return [x.numerator,x.denominator]
assert [i for i,_ in overhead]==list(range(101)) and all(v>=0 for _,v in overhead)
expected={}
for n,comparisons,validation,checksum in work:
 assert n in [16,1024] and n not in expected;values=[(i//2)*8388608 for i in range(n)];total=steps=0
 for j in range(64):
  key=0xffffffff if j==63 else j*0xffffffff//63;total+=sum(v<key for v in values);lo,hi=0,n
  while lo<hi:
   mid=(lo+hi)//2;steps+=1
   if values[mid]<key:lo=mid+1
   else:hi=mid
 assert (steps,64*(n-1),total)==(comparisons,validation,checksum);expected[n]=total
assert set(expected)=={16,1024};groups=collections.defaultdict(list);blocks={}
for n,policy,block,order,k,reps,elapsed,cputime,before,after,checksum in samples:
 assert n in expected and policy in [0,1] and 0<=block<31 and order in [0,1] and k==(order^(block&1)) and reps in [1<<i for i in range(10)] and elapsed>0 and cputime>=0 and checksum==expected[n]*reps
 key=(n,policy,block,k);assert key not in blocks;blocks[key]=(reps,elapsed);groups[n,policy,k].append(F(elapsed,reps*64))
summaries=[]
for n in [16,1024]:
 for policy in [0,1]:
  differences=[];ratios=[]
  for block in range(31):
   c,a=blocks[n,policy,block,0],blocks[n,policy,block,1];assert c[0]==a[0];differences.append(F(c[1]-a[1],c[0]*64));ratios.append(F(c[1],a[1]))
  records=[]
  for k in [0,1]:
   x=groups[n,policy,k];assert len(x)==31;records.append(dict(kernel='checked-C' if k==0 else 'checked-assembly',samples=31,minimum_ns=pair(min(x)),median_ns=pair(statistics.median(x)),maximum_ns=pair(max(x))))
  summaries.append(dict(size=n,preparation='warm' if policy==0 else 'displacement-before-batch',conditions=records,paired_median_difference_ns=pair(statistics.median(differences)),paired_median_ratio=pair(statistics.median(ratios)),unpaired_mean_difference_ns=pair(sum(groups[n,policy,0],F())/31-sum(groups[n,policy,1],F())/31)))
out=dict(status='pass',raw_samples=248,conditions=8,samples_per_condition=31,raw_overhead=overhead,clock_resolution_ns=clock[0][0],calibration_target_ns=clock[0][1],below_calibration_target=sum(s[6]<clock[0][1] for s in samples),below_resolution_or_ten_overhead=sum(s[6]<max(clock[0][0],10*statistics.median(v for _,v in overhead)) for s in samples),endpoint_migrations=sum(s[8]!=s[9] for s in samples),cpu_read_failures=sum(s[8]<0 or s[9]<0 for s in samples),work=work,summaries=summaries,limits=['No sample removed or overhead subtracted.','Endpoint struct lr_cpu equality does not rule out intervening migration; frequency/interrupt/virtualization effects are uncontrolled.','Displacement is an explicit8MiB touch before each batch, not proof of cold cache residency for every query.','Correctness-equivalent checked APIs include sortedness admission on both sides; instrumented profiles are not uninstrumented performance.','Paired blocks reduce only confounding that is approximately shared within a block; no universal speed ranking or MCU timing/energy claim.']);path.with_suffix('.analysis.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out))
