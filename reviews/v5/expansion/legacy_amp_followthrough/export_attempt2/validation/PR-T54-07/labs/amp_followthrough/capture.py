"""Validate supplied evidence structure, never authenticate physical execution."""
from pathlib import Path
import argparse,hashlib,json
CASES={'periodic','overload','m4-interference','moved-compute','drained-restart','unexpected-reset','cached-dma'}
def validate(capture,images,base):
 if capture.get('physical_status')!='observed':raise ValueError('physical evidence not observed')
 for key in ('board_id','silicon_revision','probe_id','boot_configuration'):
  if not isinstance(capture.get(key),str) or not capture[key].strip():raise ValueError('missing '+key)
 expected={x['core']:x['sha256'] for x in images['images']}
 if set(expected)!={'m7','m4'} or capture['image_readback_sha256']!=expected:raise ValueError('flashed image identity mismatch')
 if not capture.get('clock_segments') or not capture.get('measurement_limits'):raise ValueError('missing timing premises')
 for segment in capture['clock_segments']:
  if segment['core'] not in ('m7','m4') or segment['frequency_hz']<=0 or not 0<segment['elapsed_counts']<2**31 or not 0<=segment['probe_min_counts']<=segment['probe_max_counts']<=segment['elapsed_counts']:raise ValueError('invalid clock/probe segment')
 if {x['name'] for x in capture['cases']}!=CASES:raise ValueError('missing experiment case')
 for case in capture['cases']:
  states=[case[k] for k in ('completed_ids','dropped_ids','uncertain_ids')]
  ids=[i for state in states for i in state]
  if len(ids)!=len(set(ids)) or set(ids)!=set(case['accepted_ids']):raise ValueError('unclassified or duplicate accepted identity')
  if not case['raw_file'] or not case['epoch'] or case['sample_drops']<0:raise ValueError('missing case provenance')
  if case['name']=='unexpected-reset' and case['result']!='fail-closed':raise ValueError('unexpected reset is not claimed recovery')
 raw={}
 for entry in capture['raw_files']:
  path=(base/entry['path']).resolve()
  if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=entry['sha256']:raise ValueError('raw file identity mismatch')
  raw[entry['path']]=entry['sha256']
 if not raw or any(x['raw_file'] not in raw for x in capture['cases']):raise ValueError('missing raw capture')
 return {'passed':True,'scope':'Supplied evidence structure/identity/accounting only; does not authenticate measurement or confer learner mastery.'}
def main():
 p=argparse.ArgumentParser();p.add_argument('--capture',type=Path,required=True);p.add_argument('--image-report',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 try:result=validate(json.loads(a.capture.read_text()),json.loads(a.image_report.read_text()),a.capture.resolve().parent)
 except (KeyError,ValueError,TypeError,OSError) as e:result={'passed':False,'error':str(e)}
 a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
