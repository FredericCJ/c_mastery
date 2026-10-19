"""Independent synthetic changed-condition analysis, never native timing data."""
from fractions import Fraction as F
import json,statistics
c=[100+100*i for i in range(31)];a=[95+100*i for i in range(31)];paired=[x-y for x,y in zip(c,a)];all_c_first=[100+100*i for i in range(31)];all_a_later=[95+100*(31+i) for i in range(31)];unpaired=F(sum(all_c_first)-sum(all_a_later),31)
assert set(paired)=={5} and unpaired==-3095
within_block=[(100+100*i+(10 if i%2 else 0))-(95+100*i+(0 if i%2 else 10)) for i in range(31)];assert statistics.median(within_block)==-5
print(json.dumps(dict(status='pass',classification='synthetic additive-drift counterexample',paired_differences=paired,unpaired_mean_difference=[unpaired.numerator,unpaired.denominator],differential_drift_differences=within_block,differential_drift_median=-5,limits=['Blocking cancels modeled common drift, not differential within-block drift.','Unpaired schedule changes when observations were made; no causal guarantee follows solely from averaging.'])))
