"""Reject incomplete CAN FD physical claims; cannot authenticate apparatus."""
import argparse,hashlib,json,pathlib
def payload(index):
 n=[0,8,12,16,20,64][index];p=[((index*37+j*13)^0xa5)&255 for j in range(n)]
 if n:
  p[0]=index;c=0
  for x in p[:-1]:c^=x
  p[-1]=c
 return p
def check(d,folder,allow_synthetic=False):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('kind')=='physical' or (allow_synthetic and d.get('kind')=='synthetic'),'physical provenance missing')
 for key in ['board_revision','controller_revision','transceiver_revision','schematic','oscillator','peer_identity','source_identity','build_identity','instrument','probe_points','settings']:
  need(isinstance(d.get(key),str) and bool(d[key].strip()),'missing '+key)
 files=d.get('raw_files',[]);need(len(files)>=3,'raw captures/readback/peer required')
 for r in files:
  p=folder/r.get('path','');need(p.is_file() and folder.resolve() in p.resolve().parents,'contained raw file')
  if p.is_file():need(hashlib.sha256(p.read_bytes()).hexdigest()==r.get('sha256'),'raw hash')
 for key in ['spi_readback','loopback','peer_receive','waveform','changed_condition']:
  need(isinstance(d.get(key),str) and d[key] in [r.get('path') for r in files],'missing '+key+' raw binding')
 need(d.get('clock_hz')==40000000 and d.get('spi_hz',0)>0 and d.get('spi_hz',2**32)<=1000000,'selected clock/SPI route')
 runs=d.get('runs',[]);need(len(runs)==4,'BRS/rate changed runs')
 for run,(rate,brs) in zip(runs,[(1000000,0),(1000000,1),(2000000,0),(2000000,1)]):
  need(run.get('configured_data_rate')==rate and run.get('brs')==brs,'changed rate/BRS')
  expected_rate=rate if brs else 500000;measured=run.get('measured_data_rate',0);nominal=run.get('measured_nominal_rate',0);resolution=run.get('sample_ns',0)
  need(abs(measured-expected_rate)*100<=expected_rate*5 and abs(nominal-500000)*100<=500000*5,'measured rate outside declared5percent bound')
  need(isinstance(resolution,(int,float)) and 0<resolution<=1e9/expected_rate/10,'capture resolution')
  frames=run.get('peer_frames',[]);need(len(frames)==6,'peer sequence count')
  for i,f in enumerate(frames[:6]):need(f.get('id')==0x300+i and f.get('payload')==payload(i) and f.get('brs')==brs,'peer sequence/content/BRS')
 need(d.get('fresh_peer_seed') not in [None,0] and d.get('fresh_peer_checked') is True,'independent fresh peer challenge')
 need(set(d.get('faults',[]))=={'disconnected-controller','wrong-spi-mode','missing-ack','filter-reject','queue-overflow','recovery'},'failure/recovery captures')
 need(d.get('physical_gate_waived') is False,'required physical gate cannot be waived')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('dossier',type=pathlib.Path);p.add_argument('--synthetic',action='store_true');a=p.parse_args();d=json.loads(a.dossier.read_text());errors=check(d,a.dossier.parent,a.synthetic)
 print(json.dumps(dict(passed=not errors,evidence_kind=d.get('kind'),physical_authentication='not-established-by-validator',errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
