"""Read a declared synthetic or physical digital trace; no hardware interaction."""
from pathlib import Path
import argparse,hashlib,json

def analyze(d,base=Path('.')):
 def need(ok,msg):
  if not ok:raise ValueError(msg)
 need(d.get('origin') in ['synthetic','physical'],'origin')
 mode=d.get('mode');tx=d.get('tx');rx=d.get('rx');events=d.get('events');u=d.get('uncertainty_ns')
 need(type(mode)==int and 0<=mode<=3,'mode')
 need(isinstance(tx,list) and isinstance(rx,list) and 1<=len(tx)<=16 and len(tx)==len(rx) and all(type(v)==int and 0<=v<=255 for v in tx+rx),'octets')
 need(type(u)==int and u>=0 and all(type(d.get(k))==int and d[k]>=0 for k in ['setup_ns','hold_ns','half_period_min_ns']),'timing bounds')
 need(isinstance(events,list) and len(events)>3,'events')
 if d['origin']=='physical':
  need(all(d.get(k) for k in ['board_revision','chip_revision','partner_source_sha256','firmware_sha256','instrument','calibration','routing','clock_record','raw_path','raw_sha256']),'physical provenance')
  raw=base/d['raw_path'];need(raw.is_file() and hashlib.sha256(raw.read_bytes()).hexdigest()==d['raw_sha256'],'raw identity')
 last=None;fall=rise=None;edges=[];out=[[],[]];cs_intervals=0
 for e in events:
  need(set(e)=={'ns','sck','mosi','miso','cs'} and type(e['ns'])==int and e['ns']>=0 and all(type(e[k])==int and e[k] in [0,1] for k in ['sck','mosi','miso','cs']),'event shape')
  if last is None:need(e['cs']==1 and e['sck']==mode//2,'initial idle')
  else:
   need(e['ns']>last['ns'],'monotonic capture')
   if e['cs']!=last['cs']:
    need(e['sck']==last['sck']==mode//2,'CS clock idle')
    if e['cs']==0:cs_intervals+=1;fall=e['ns']
    else:rise=e['ns']
   if e['sck']!=last['sck']:
    need(e['cs']==last['cs']==0 and fall is not None and rise is None,'clock outside transaction')
    leading=e['sck']!=mode//2
    if leading==(mode%2==0):
     out[0].append(e['mosi']);out[1].append(e['miso'])
    edges.append(e['ns'])
  last=e
 need(cs_intervals==1 and fall is not None and rise is not None and last['cs']==1,'single completed CS interval')
 need(len(edges)==16*len(tx) and len(out[0])==8*len(tx),'wire length')
 decode=lambda bits:[int(''.join(map(str,bits[i:i+8])),2) for i in range(0,len(bits),8)]
 need(decode(out[0])==tx and decode(out[1])==rx,'sampled bytes')
 setup=edges[0]-fall-2*u;hold=rise-edges[-1]-2*u;half=min(b-a for a,b in zip(edges,edges[1:]))-2*u
 need(setup>=d['setup_ns'] and hold>=d['hold_ns'] and half>=d['half_period_min_ns'],'timing lower bound')
 return dict(passed=True,origin=d['origin'],frames=len(tx),transitions=len(edges),setup_lower_ns=setup,hold_lower_ns=hold,half_period_lower_ns=half,limits='Digital sampled trace only; no analog-level, aperture/metastability, DMA-memory ownership or capture-authenticity proof.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('capture',type=Path);a=p.parse_args();print(json.dumps(analyze(json.loads(a.capture.read_text()),a.capture.parent),indent=2))
