"""Bounded paired percentile bootstrap; no universal coverage claim."""
def lower_median(values):return sorted(values)[(len(values)-1)//2]
def interval(differences,replicates,seed):
 if not 1<=len(differences)<=32 or not 40<=replicates<=4096 or not 0<=seed<2**32:raise ValueError('bounds')
 state=seed;medians=[]
 for _ in range(replicates):
  sample=[]
  for _ in differences:
   state=(1664525*state+1013904223)&0xffffffff;sample.append(differences[(state*len(differences))>>32])
  medians.append(lower_median(sample))
 medians.sort()
 return medians[(replicates-1)*25//1000],medians[(replicates-1)*975//1000],medians
