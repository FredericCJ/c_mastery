"""Original signed-pair and resampling verification, with independent rank counts."""
import argparse,collections,importlib.util,itertools,json,math,re
from pathlib import Path
HERE=Path(__file__).resolve().parent
def load(path):
 spec=importlib.util.spec_from_file_location('candidate',path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
def rank(values,k):
 counts=collections.Counter(values);seen=0
 for value in sorted(counts):
  seen+=counts[value]
  if seen>k:return value
 raise AssertionError('invalid rank')
def expected_resamples(data,repetitions,seed):
 state=seed;result=[]
 for _ in range(repetitions):
  counts=collections.Counter()
  for _ in data:
   state=(state*1664525+1013904223)%(2**32);counts[data[state*len(data)//2**32]]+=1
  remaining=(len(data)-1)//2
  for value in sorted(counts):
   remaining-=counts[value]
   if remaining<0:result.append(value);break
 return sorted(result)
def main():
 p=argparse.ArgumentParser();p.add_argument('raw',type=Path);p.add_argument('--candidate',type=Path,default=HERE/'bootstrap.py');a=p.parse_args();m=load(a.candidate);text=a.raw.read_text();pairs=[]
 for line in text.splitlines():
  match=re.fullmatch(r'PAIR (\d+) (AB|BA) (\d+) (\d+)',line)
  if match:pairs.append((int(match[1]),match[2],int(match[3]),int(match[4])))
 assert len(pairs)==32 and [x[0] for x in pairs]==list(range(32))
 assert collections.Counter(x[1] for x in pairs)=={'AB':16,'BA':16}
 differences=[b-a for _,_,a,b in pairs];summary=re.search(r'PAIRED median=(-?\d+) wins=(\d+) consumed=(\d+)',text);assert summary
 assert int(summary[1])==rank(differences,15) and int(summary[2])==sum(x<0 for x in differences)
 observations={}
 for name,data in [('actual',differences),('outlier',[-10,-9,-8,-7,9900]),('bounds',[-(2**32-1),0,2**32-1]),('constant',[7]),('even',[-3,1,8,12])]:
  low,high,medians=m.interval(data,1999,0x51ce);expected=expected_resamples(data,1999,0x51ce)
  assert medians==expected and (low,high)==(expected[49],expected[1948])
  observations[name]={'lower':low,'upper':high,'replicates':len(medians),'unique_medians':len(set(medians))}
 # Exact small bootstrap distributions, independently checked by binomial CDF.
 exact=[]
 for values in [(-1,2),(-5,0,9),(2,2,7)]:
  n=len(values);distribution=collections.Counter(rank(sample,(n-1)//2) for sample in itertools.product(values,repeat=n));threshold=(n-1)//2+1
  for cutoff in sorted(set(values)):
   good=sum(x<=cutoff for x in values);cdf=sum(math.comb(n,k)*good**k*(n-good)**(n-k) for k in range(threshold,n+1));assert sum(count for value,count in distribution.items() if value<=cutoff)==cdf
  exact.append({'values':values,'count':n**n,'distribution':dict(distribution)})
 for bad in [([],100,1),([1]*33,100,1),([1],39,1),([1],4097,1),([1],100,-1)]:
  try:m.interval(*bad)
  except ValueError:pass
  else:raise AssertionError('accepted invalid bootstrap input')
 by_order={order:rank([b-a for _,o,a,b in pairs if o==order],7) for order in ['AB','BA']}
 result={'passed':True,'pairs':pairs,'paired_lower_median':int(summary[1]),'wins':int(summary[2]),'order_specific_lower_medians':by_order,'order_contrast_BA_minus_AB':by_order['BA']-by_order['AB'],'bootstrap':observations,'exact_small_distributions':exact,'limits':['Selected actual run; no required speedup, significance or WCET.','Finite paired percentile resampling with declared seed/rounding; serial dependence and nonstationarity remain assumptions, not proven95% coverage.','Source public benchmark includes and retains actual provider s_paired output.']}
 print(json.dumps(result,indent=2));return 0
if __name__=='__main__':
 try:raise SystemExit(main())
 except (AssertionError,NotImplementedError) as e:print('BOOTSTRAP_CHECK:',type(e).__name__,str(e));raise SystemExit(1)
