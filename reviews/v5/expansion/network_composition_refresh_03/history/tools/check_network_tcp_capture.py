"""Structural/timing gate only; never authenticates physical provenance."""
from pathlib import Path
import argparse,copy,hashlib,json,tempfile
SCENARIOS={'normal','loss','zero','close','reset','quiescence'}
ENV={'board','phy','clock_hz','uncertainty_ms','firmware_sha256','adapter_sha256','capture_point','offload_policy','snaplen','fcs','timestamp_domain'}
def validate(d,base):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('observed') is True,'physical capture not observed');env=d.get('environment',{});need(ENV<=set(env),'environment fields')
 need(isinstance(env.get('clock_hz'),int) and env.get('clock_hz',0)>0,'clock rate');need(isinstance(env.get('uncertainty_ms'),(int,float)) and env.get('uncertainty_ms',-1)>=0,'clock uncertainty');need(env.get('snaplen',0)>=1514,'snaplen')
 for key in ['firmware_sha256','adapter_sha256']:need(isinstance(env.get(key),str) and len(env.get(key,''))==64 and all(c in '0123456789abcdef' for c in env.get(key,'')),'source identity '+key)
 artifacts=d.get('artifacts',[]);need({x.get('kind') for x in artifacts}>={'pcap','decoded-packet-table','event-log','firmware-elf'},'required raw/analysis artifacts')
 for r in artifacts:
  p=base/r.get('path','');need(p.is_file() and hashlib.sha256(p.read_bytes()).hexdigest()==r.get('sha256'),'artifact identity '+str(p))
 need(set(d.get('scenarios',[]))==SCENARIOS,'six mandatory scenarios');events=d.get('events',[]);need(bool(events),'event records')
 accepted={};last=-1
 for r in events:
  now=r.get('ms',-1);need(isinstance(now,int) and last<=now<=2**64-1-240000,'monotonic headroom');last=now
  kind=r.get('kind');key=(r.get('epoch'),r.get('serial'))
  need(all(isinstance(v,int) and 0<v<=2**32-1 for v in key),'epoch serial')
  if kind=='copied':
   need(key not in accepted,'duplicate copy ticket');rto=r.get('rto',0);need(1000<=rto<=60000,'RTO bound');need(r.get('due')==now+rto,'timer origin');accepted[key]=r
  elif kind=='refused':need(not r.get('flight_started',True),'refusal started timer')
  elif kind=='dma-reaped':need(key in accepted,'DMA before copy');need(r.get('tcp_retired') is False,'DMA confused with ACK')
  elif kind=='retransmitted':
   previous=accepted.get((r.get('previous_epoch'),r.get('previous_serial')));need(previous is not None,'missing original copy')
   if previous:need(now>=previous['due'],'early retransmit')
  elif kind not in {'acked','consumed','aborted','quiescent'}:errors.append('unknown event')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('record',nargs='?',type=Path);p.add_argument('--self-test',action='store_true');a=p.parse_args()
 if a.self_test:
  with tempfile.TemporaryDirectory() as temp:
   base=Path(temp);(base/'raw').write_bytes(b'synthetic control only');h=hashlib.sha256(b'synthetic control only').hexdigest()
   d=dict(observed=True,environment={**{k:'synthetic' for k in ENV},'clock_hz':1000,'uncertainty_ms':1,'snaplen':1514,'firmware_sha256':h,'adapter_sha256':h},artifacts=[dict(kind=k,path='raw',sha256=h) for k in ['pcap','decoded-packet-table','event-log','firmware-elf']],scenarios=sorted(SCENARIOS),events=[dict(kind='copied',epoch=1,serial=1,ms=100,rto=1000,due=1100),dict(kind='dma-reaped',epoch=1,serial=1,ms=101,tcp_retired=False)])
   assert not validate(d,base);controls=[]
   for kind in ['unobserved','identity','scenario','clock','snaplen','raw','origin','dma','ticket','backwards','refusal','early']:
    x=copy.deepcopy(d)
    if kind=='unobserved':x['observed']=False
    elif kind=='identity':x['environment']['adapter_sha256']='0'
    elif kind=='scenario':x['scenarios'].pop()
    elif kind=='clock':x['environment']['clock_hz']=0
    elif kind=='snaplen':x['environment']['snaplen']=64
    elif kind=='raw':x['artifacts'][0]['sha256']='0'*64
    elif kind=='origin':x['events'][0]['due']=1000
    elif kind=='dma':x['events'][1]['tcp_retired']=True
    elif kind=='ticket':x['events'][1]['serial']=0
    elif kind=='backwards':x['events'][1]['ms']=99
    elif kind=='refusal':x['events'].append(dict(kind='refused',epoch=1,serial=2,ms=102,flight_started=True))
    else:x['events'].append(dict(kind='retransmitted',epoch=1,serial=2,ms=1099,previous_epoch=1,previous_serial=1))
    assert validate(x,base),kind;controls.append(kind)
   print(json.dumps(dict(passed=True,controls=controls,physical_claim=False)));return 0
 if not a.record:p.error('record required')
 errors=validate(json.loads(a.record.read_text()),a.record.parent);print(json.dumps(dict(passed=not errors,errors=errors,physical_claim=False)));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
