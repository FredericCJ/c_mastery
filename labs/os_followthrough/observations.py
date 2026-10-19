"""Parse raw owned-guest records and summarize bounded sample uncertainty."""
import argparse,json,re,statistics
from pathlib import Path
def raw_records(text):return [(path,body.strip()) for path,body in re.findall(r'OF_RAW_BEGIN ([^\n]+)\n(.*?)OF_RAW_END [^\n]+',text,re.S)]
def keyed(text):return {k:int(v) for k,v in re.findall(r'^(\w+) (\d+)$',text,re.M)}
def guest(text):
 assert 'OF_PROBE_EXIT=0' in text and 'OF_PASS:guest:namespace:cpu-memory-weight-ancestry-io:cleanup' in text and 'OF_GUEST_DONE' in text and 'OF_GUEST_FAIL' not in text and 'OF_TIMEOUT' not in text
 trials={}
 for name,body in re.findall(r'OF_TRIAL_BEGIN (\S+)\n(.*?)OF_TRIAL_END \1[^\n]*',text,re.S):
  records=raw_records(body);exits=[dict(zip(['pid','raw','exited','code','signal'],map(int,r))) for r in re.findall(r'OF_EXIT pid=(\d+) raw=(\d+) exited=(\d+) code=(-?\d+) signal=(\d+)',body)];assert exits
  assert all((r['signal']==9 and r['exited']==0) if name=='memory' else (r['exited']==1 and r['code']==(12 if name=='allocation-refused' else 0) and r['signal']==0) for r in exits)
  trials[name]={'raw':records,'exits':exits,'compute':[dict(zip(['iterations','checksum','wall_ns','cpu_ns'],map(int,r))) for r in re.findall(r'OF_COMPUTE iterations=(\d+) checksum=(\d+) wall_ns=(\d+) cpu_ns=(\d+)',body)]}
 def values(name,suffix):return [body for path,body in trials[name]['raw'] if path.endswith(suffix)]
 assert set(trials)=={'ungrouped','unlimited','cpu','memory','weighted','ancestry0','ancestry1','io-limit','allocation-refused'}
 assert values('ungrouped','/cgroup')==['0::/','0::/'];assert '0::/of/cpu' in values('cpu','/cgroup')
 assert values('cpu','/cpu.max')==['20000 100000']*2;cpu=[keyed(s) for s in values('cpu','/cpu.stat')];assert cpu[-1]['nr_throttled']>cpu[0]['nr_throttled'] and cpu[-1]['throttled_usec']>cpu[0]['throttled_usec']
 assert 'OF_RLIMIT_AS 33554432 worker-only' in text and 'OF_ALLOCATION_FAILURE errno=12' in text
 assert values('memory','/memory.max')==['16777216']*2;memory=[keyed(s) for s in values('memory','/memory.events')];assert memory[-1]['oom_kill']>memory[0]['oom_kill'] and not trials['memory']['compute']
 assert [v for p,v in trials['weighted']['raw'] if p.endswith('/weighted/cpu.max')]==['30000 100000']*2
 assert [v for p,v in trials['weighted']['raw'] if p.endswith('/a/cpu.weight')]==['100']*2 and [v for p,v in trials['weighted']['raw'] if p.endswith('/b/cpu.weight')]==['400']*2
 for name in ('ancestry0','ancestry1'):assert [v for p,v in trials[name]['raw'] if p.endswith('/child/cpu.max')]==['20000 100000']*2
 assert [v for p,v in trials['ancestry0']['raw'] if p.endswith('/ancestry/cpu.max')]==['max 100000']*2 and [v for p,v in trials['ancestry1']['raw'] if p.endswith('/ancestry/cpu.max')]==['10000 100000']*2
 ancestry=[keyed(v) for p,v in trials['ancestry1']['raw'] if p.endswith('/ancestry/cpu.stat')];assert ancestry[-1]['nr_throttled']>ancestry[0]['nr_throttled']
 io_limits=values('io-limit','/io.max');assert all('rbps=2097152 wbps=2097152' in v for v in io_limits);io=values('io-limit','/io.stat');assert re.search(r'wbytes=4194304\b',io[-1]);assert re.search(r'OF_ENOSPC accepted=2097152 remaining=2097152 errno=28',text)
 rows=[dict(zip(['wrote','read','fsync_ack','size','checksum','wall_ns','cpu_ns','major_delta','minor_delta','sync_each','concurrent'],map(int,r))) for r in re.findall(r'OF_IO wrote=(\d+) read=(\d+) fsync_ack=(\d+) size=(\d+) checksum=(\d+) wall_ns=(\d+) cpu_ns=(\d+) major_delta=(\d+) minor_delta=(\d+) sync_each=(\d+) concurrent=(\d+)',text)];assert len(rows)==9
 for r in rows:assert r['wrote']==r['read']==r['fsync_ack']==r['size'] and r['checksum']==sum(i%251 for i in range(4096))*(r['wrote']//4096)
 pairs=rows[1:7];timings={str(sync):[r['wall_ns'] for r in pairs if r['sync_each']==sync] for sync in (0,1)};assert all(len(v)==3 for v in timings.values());summary={k:{'n':len(v),'min':min(v),'median':statistics.median(v),'max':max(v)} for k,v in timings.items()}
 assert 'OF_NAMESPACE pid=1 ' in text and 'OF_NAMESPACE_PARENT private_file_absent=1 budget_unchanged=1' in text and 'OF_IO_BACKEND_CHANGED tmpfs' in text
 assert len(re.findall(r'OF_OTHER_READER pid=\d+ logical=4194304',text))==2
 return {'passed':True,'trials':trials,'io_rows':rows,'paired_wall_ns':summary,'warm_logical_read_bytes':sum(r['read'] for r in rows),'warm_major_faults':sum(r['major_delta'] for r in rows),'scope':'Actual private guest;3 paired samples are descriptive ranges,not confidence guarantees. Device counters exclude logical cache hits and need scope/metadata attribution.','physical_durability':False,'host_policy_changed':False}
def locality(a,b):
 assert a['passed'] and b['passed'] and a['competitor']==0 and b['competitor']==1 and 0<b['competing_reader_bytes']<=512*1024*1024
 result={}
 for name,record in [('baseline',a),('competing-reader',b)]:
  assert len(record['rows'])==54 and record['mapping']=='MAP_PRIVATE|MAP_ANONYMOUS'
  for r in record['rows']:assert r['allocation_start_ns']<=r['allocation_end_ns']<=r['touch_start_ns'] and r['wall_ns']>0 and r['cpu_ns']>0 and r['checksum']==(0 if not r['write_first'] else sum((i%251+1)*16 for i in range(2048)))
  result[name]={}
  for pattern in range(3):
   for write in range(2):
    values=[r['wall_ns'] for r in record['rows'] if r['pattern']==pattern and r['write_first']==write];assert len(values)==9;result[name][f'{pattern}-{write}']={'n':9,'min':min(values),'median':statistics.median(values),'max':max(values)}
 return {'passed':True,'groups':result,'competing_reader_bytes':b['competing_reader_bytes'],'interpretation':'Observed min/median/max are finite descriptive uncertainty intervals;host isolation and cold-cache control absent.'}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('kind',choices=['guest','locality']);p.add_argument('paths',nargs='+',type=Path);a=p.parse_args();print(json.dumps(guest(a.paths[0].read_text()) if a.kind=='guest' else locality(*[json.loads(v.read_text()) for v in a.paths])))
