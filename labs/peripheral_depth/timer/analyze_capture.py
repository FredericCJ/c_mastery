"""Interval arithmetic for a declared timer/instrument uncertainty model.

Inputs are decimal strings or integers, never binary floating point. Synthetic
results cannot become physical observations by passing this calculation.
"""
from fractions import Fraction as F
from pathlib import Path
import argparse,json,hashlib

def analyze(d):
 def number(k):
  v=d[k]
  if isinstance(v,(bool,float)) or not isinstance(v,(str,int)):raise ValueError(k)
  return F(v)
 clock=number('timer_hz');ppm=number('clock_tolerance_ppm');psc=number('psc');arr=number('arr');ccr=number('ccr');center=d['center']
 if type(center) is not bool or clock<=0 or clock>180000000 or not 0<=ppm<1000000:raise ValueError('clock/mode')
 if psc.denominator!=1 or not 0<=psc<=65535 or arr.denominator!=1 or not 1<=arr<=(65535 if center else 65534):raise ValueError('registers')
 n=arr+(not center)
 if ccr.denominator!=1 or not 0<=ccr<=n:raise ValueError('compare')
 t0=number('rise0_ns');fall=number('fall_ns');t1=number('rise1_ns');resolution=number('resolution_ns');jitter=number('edge_jitter_ns');timebase=number('instrument_ppm');skew=number('probe_skew_ns')
 if not t0<fall<t1 or min(resolution,jitter,timebase,skew)<0 or timebase>=1000000:raise ValueError('measurement')
 if not 0<ccr<n:raise ValueError('zero/full duty requires DC-level protocol; edge interval unavailable')
 scale=F(1000000);tick_lo=(psc+1)*10**9/(clock*(1+ppm/scale));tick_hi=(psc+1)*10**9/(clock*(1-ppm/scale));ticks=2*arr if center else arr+1
 period_expected=(ticks*tick_lo,ticks*tick_hi)
 edge=resolution/2+jitter
 def interval(delta,extra=F(0)):
  return (delta*(1-timebase/scale)-2*edge-extra,delta*(1+timebase/scale)+2*edge+extra)
 period=interval(t1-t0);high=interval(fall-t0,skew)
 if period[0]<=0 or high[0]<0 or high[1]>period[1]:raise ValueError('unresolved interval')
 duty=(high[0]/period[1],high[1]/period[0]);expected_duty=ccr/n
 previous=number('capture_previous');current=number('capture_current');extra=number('capture_interval_extra_ticks')
 if any(x.denominator!=1 for x in (previous,current,extra)) or not 0<=previous<=65535 or not 0<=current<=65535 or not 0<=extra<=32767 or d['overcapture'] is not False:raise ValueError('capture')
 delta=(current-previous)%65536
 if not 0<delta<32768 or delta<=1+extra:raise ValueError('ambiguous capture')
 capture=((delta-1-extra)*tick_lo,(delta+1+extra)*tick_hi)
 def overlap(a,b):return max(a[0],b[0])<=min(a[1],b[1])
 if d['origin'] not in ('synthetic','physical'):raise ValueError('origin')
 if d['origin']=='physical':
  for k in ['board_identity','firmware_sha256','instrument_identity','calibration_record','probe_identity','clock_source','raw_capture']:
   if not isinstance(d.get(k),str) or not d[k].strip():raise ValueError('missing physical metadata '+k)
  raw=Path(d['raw_capture'])
  if not raw.is_file() or hashlib.sha256(raw.read_bytes()).hexdigest()!=d.get('raw_capture_sha256'):raise ValueError('raw capture identity')
 result=dict(origin=d['origin'],period_expected_ns=period_expected,period_observed_ns=period,duty_observed=duty,duty_expected=expected_duty,capture_ns=capture,frequency_expected_hz=(10**9/period_expected[1],10**9/period_expected[0]),period_consistent=overlap(period_expected,period),duty_consistent=duty[0]<=expected_duty<=duty[1],capture_consistent=overlap(period,capture),limits='Declared deterministic bounds, not statistical confidence or independent calibration; shared clock error can cancel in loopback. Physical metadata is not independently authenticated.')
 return {k:([str(x) for x in v] if isinstance(v,tuple) else str(v) if isinstance(v,F) else v) for k,v in result.items()}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('capture',type=Path);a=p.parse_args()
 print(json.dumps(analyze(json.loads(a.capture.read_text())),indent=2))
