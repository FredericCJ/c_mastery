"""Check decoded timing arithmetic; never infer device legality or physical success."""
import argparse,json
from fractions import Fraction
from pathlib import Path
def timing(d):
 names=['clock_hz','prescaler','tseg1','tseg2','sjw','requested_bitrate']
 if any(type(d.get(k)) is not int or d[k]<=0 for k in names):raise ValueError('positive integer decoded timing parameters required')
 if d['sjw']>d['tseg2']:raise ValueError('SJW exceeds phase segment2 in this teaching contract')
 quanta=1+d['tseg1']+d['tseg2'];rate=Fraction(d['clock_hz'],d['prescaler']*quanta)
 if rate!=d['requested_bitrate']:raise ValueError('nominal bitrate mismatch')
 return dict(nominal_bitrate=str(rate),sample_point=str(Fraction(1+d['tseg1'],quanta)),device_limits_verified=False,physical_validated=False)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--plan',type=Path);p.add_argument('--self-test',action='store_true');a=p.parse_args()
 if a.self_test:
  d=dict(clock_hz=16000000,prescaler=2,tseg1=13,tseg2=2,sjw=1,requested_bitrate=500000)
  assert timing(d)['sample_point']=='7/8'
  for k,v in [('clock_hz',None),('prescaler',0),('sjw',3),('requested_bitrate',250000),('tseg1',True)]:
   bad=dict(d,**{k:v})
   try:timing(bad)
   except ValueError:pass
   else:raise AssertionError(k)
  print(json.dumps(dict(passed=True,synthetic_math_cases=6,device_legality=False,physical_observation=False)));return 0
 if not a.plan:p.error('--plan or --self-test required')
 d=json.loads(a.plan.read_text());print(json.dumps(timing(d['timing'])));return 0
if __name__=='__main__':raise SystemExit(main())
