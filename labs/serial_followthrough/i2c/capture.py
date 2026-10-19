"""Host-side evidence admission, never a target dependency or hardware attestation."""
from fractions import Fraction
def assess(capture,mutant=False):
 mode=capture['mode'];limit={'standard':1000,'fast':300}.get(mode)
 if limit is None:raise ValueError('unsupported mode')
 fields=['rise_ns','uncertainty_ns','scl_low_ns','scl_high_ns']
 if any(not isinstance(capture[k],int) or isinstance(capture[k],bool) or capture[k]<0 for k in fields):raise ValueError('invalid interval')
 rise=capture['rise_ns']+capture['uncertainty_ns']
 low,high=(4700,4000) if mode=='standard' else (1300,600)
 timing=rise<=limit and capture['scl_low_ns']-capture['uncertainty_ns']>=low and capture['scl_high_ns']-capture['uncertainty_ns']>=high
 if mutant:timing=capture['decoded']==[0xa0,7,0xa1,0x5a]
 # A declared boolean alone cannot authenticate raw measurements or instrument calibration.
 return dict(timing_window_pass=timing,physical_gate='requires raw capture and assessor verification',max_rise_ns=limit,observed_upper_rise_ns=rise)
def nominal_rise(resistance_ohm,capacitance_pf):
 return Fraction(8473*resistance_ohm*capacitance_pf,10000000)
