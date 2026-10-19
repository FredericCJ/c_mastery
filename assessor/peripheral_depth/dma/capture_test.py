from pathlib import Path
import copy,json,sys
ROOT=Path(__file__).resolve().parents[3];sys.path.insert(0,str(ROOT/'labs/peripheral_depth/dma'));from board_capture import validate
base=dict(schema_version=1,provenance='synthetic-test',board='H745',silicon_revision='model',image_sha256='1'*64,raw_artifacts=[dict(path='synthetic-not-board',sha256='2'*64)],counter_hz=1000000,cache_policy='write-back',observations=[dict(tick=i,token=1,kind=k,enabled=0,fifo_empty=1,flags=32,total=32,complete=32,possible=32,guards_unchanged=True) for i,k in enumerate(['submit','cancel','quiescent','finalize','publish','take','ack','reuse'])])
assert validate(base)['passed'];controls=[]
for k in ['pending','no-raw','no-timebase','dirty-guard','enabled','fifo','no-tc','overcount','early-reuse','no-ack','bad-time','missing-finalize','cache','blank']:
 x=copy.deepcopy(base)
 if k=='pending':x['provenance']='pending'
 elif k=='no-raw':x['raw_artifacts']=[]
 elif k=='no-timebase':x['counter_hz']=0
 elif k=='dirty-guard':x['observations'][3]['guards_unchanged']=False
 elif k in ('enabled','fifo','no-tc'):x['observations'][2][{'enabled':'enabled','fifo':'fifo_empty','no-tc':'flags'}[k]]={'enabled':1,'fifo':0,'no-tc':0}[k]
 elif k=='overcount':x['observations'][3]['complete']=33
 elif k=='early-reuse':x['observations'][2]['kind']='reuse'
 elif k=='no-ack':x['observations'].pop(6)
 elif k=='bad-time':x['observations'][3]['tick']=-1
 elif k=='missing-finalize':x['observations'].pop(3)
 elif k=='cache':x['cache_policy']='write-through'
 else:x=json.loads((ROOT/'labs/peripheral_depth/dma/capture_template.json').read_text())
 assert not validate(x)['passed'],k;controls.append(k)
print(json.dumps(dict(passed=True,synthetic_valid=1,invalid_captures_rejected=len(controls),controls=controls,physical_observations=0)))
