from pathlib import Path
import copy,json,sys
from fractions import Fraction as F
sys.path.insert(0,str(Path(__file__).resolve().parents[3]/'labs/peripheral_depth/timer'))
from analyze_capture import analyze

def main():
 d=dict(origin='synthetic',timer_hz=16000000,clock_tolerance_ppm=100,psc=15,arr=999,ccr=250,center=False,rise0_ns=0,fall_ns=250000,rise1_ns=1000000,resolution_ns=10,edge_jitter_ns=5,instrument_ppm=20,probe_skew_ns=3,capture_previous=65530,capture_current=994,capture_interval_extra_ticks=2,overcapture=False)
 r=analyze(d);assert all(r[k] for k in ['period_consistent','duty_consistent','capture_consistent'])
 assert r['period_expected_ns']==['10000000000/10001','10000000000/9999']
 assert r['period_observed_ns']==['999960','1000040']
 assert r['duty_observed']==[str(F(249972,1000040)),str(F(250028,999960))]
 assert r['capture_ns']==[str(F(9970000000,10001)),str(F(10030000000,9999))]
 changed=copy.deepcopy(d);changed.update(timer_hz=8000000,arr=499,ccr=125,capture_current=494,clock_tolerance_ppm=10000,resolution_ns=100,instrument_ppm=100)
 c=analyze(changed);assert all(c[k] for k in ['period_consistent','duty_consistent','capture_consistent']);assert F(c['period_expected_ns'][1])-F(c['period_expected_ns'][0])>F(r['period_expected_ns'][1])-F(r['period_expected_ns'][0])
 incorrect=copy.deepcopy(d);incorrect['fall_ns']=500000;assert not analyze(incorrect)['duty_consistent']
 incorrect=copy.deepcopy(d);incorrect['rise1_ns']=1100000;assert not analyze(incorrect)['period_consistent']
 rejected=[]
 for key,value in [('origin','physical'),('overcapture',True),('clock_tolerance_ppm',1000000),('capture_current',32762),('arr',0),('ccr',1000),('timer_hz',16e6),('fall_ns',-1),('resolution_ns',1000000)]:
  x=copy.deepcopy(d);x[key]=value
  try:analyze(x)
  except (ValueError,KeyError):rejected.append(key)
  else:raise AssertionError(('accepted invalid',key))
 return dict(passed=True,ordinary=r,changed=c,negative_controls=rejected,disagreement_controls=['wrong duty','wrong period'],limits='All examples synthetic; no measured board values')
if __name__=='__main__':print(json.dumps(main()))
