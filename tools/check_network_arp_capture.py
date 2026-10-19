"""Check capture structure/byte relations, never manufacture physical provenance."""
from pathlib import Path
import argparse,copy,hashlib,json,tempfile
def validate(d,root):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('observed') is True,'physical capture not observed')
 for k in ['operator','board_revision','phy_revision','driver_contract','instrument']:need(isinstance(d.get(k),str) and bool(d[k].strip()),'missing '+k)
 h=d.get('firmware_sha256');need(isinstance(h,str) and len(h)==64 and all(c in '0123456789abcdef' for c in h),'firmware identity')
 c=d.get('calibration',{});need(c.get('units')=='milliseconds' and type(c.get('maximum_uncertainty_ms')) is int and 0<=c['maximum_uncertainty_ms']<=100 and bool(c.get('derivation')),'calibrated finite uncertainty')
 raw=d.get('raw_files',[]);need(bool(raw),'raw capture absent')
 for r in raw:
  p=root/r.get('path','missing');need(p.is_file() and hashlib.sha256(p.read_bytes()).hexdigest()==r.get('sha256'),'raw capture identity')
 try:local=bytes.fromhex(d['local_mac']);lip=bytes.fromhex(d['local_ip']);peer=bytes.fromhex(d['peer_ip']);assert len(local)==6 and len(lip)==len(peer)==4
 except (KeyError,ValueError,TypeError,AssertionError):return errors+['address encoding']
 last=-1;mac=None;expiry=0;learned=set();sent=set();events=d.get('events',[])
 for r in events:
  now=r.get('local_ms');need(type(now) is int and now>=0 and now>=last,'monotonic trace')
  if type(now) is not int:continue
  last=now
  try:p=bytes.fromhex(r['frame_hex'])
  except (KeyError,ValueError,TypeError):errors.append('frame encoding');continue
  need(42<=len(p)<=1514,'bounded frame')
  if len(p)<42:continue
  if r.get('direction')=='rx' and p[12:14]==b'\x08\x06':
   ok=p[:6] in [local,b'\xff'*6] and p[14:20]==b'\x00\x01\x08\x00\x06\x04' and p[20:22]==b'\x00\x02' and p[6:12]==p[22:28] and p[28:32]==peer and p[32:38]==local and p[38:42]==lip and any(p[6:12]) and not p[6]&1
   need(ok,'association binding')
   if ok:mac=p[6:12];expiry=now+30000;learned.add(mac)
  elif r.get('direction')=='tx' and p[12:14]==b'\x08\x00':
   need(mac is not None and now<expiry and p[:6]==mac and p[6:12]==local,'current learned destination')
   if mac is not None and now<expiry and p[:6]==mac:sent.add(mac)
  else:errors.append('unsupported selected summary event')
 need(len(learned)>=2 and learned<=sent,'two changed associations reach serialization')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('capture',nargs='?',type=Path);p.add_argument('--self-test',action='store_true');a=p.parse_args();controls=[]
 if a.self_test:
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);raw=root/'raw';raw.write_bytes(b'synthetic fixture, not physical capture');d=dict(observed=True,operator='synthetic',board_revision='synthetic',phy_revision='synthetic',driver_contract='copy',instrument='synthetic',firmware_sha256='a'*64,calibration=dict(units='milliseconds',maximum_uncertainty_ms=1,derivation='synthetic'),raw_files=[dict(path='raw',sha256=hashlib.sha256(raw.read_bytes()).hexdigest())],local_mac='020000000002',local_ip='c0000202',peer_ip='c0000201',events=[])
   for time,tail in [(0,7),(30000,99)]:
    m=bytes([2,0,0,0,0,tail]);local=bytes.fromhex(d['local_mac']);arp=local+m+b'\x08\x06\x00\x01\x08\x00\x06\x04\x00\x02'+m+bytes.fromhex(d['peer_ip'])+local+bytes.fromhex(d['local_ip']);ip=m+local+b'\x08\x00'+bytes(46)
    d['events'] += [dict(local_ms=time,direction='rx',frame_hex=arp.hex()),dict(local_ms=time+1,direction='tx',frame_hex=ip.hex())]
   assert not validate(d,root)
   for kind in ['unobserved','missing-raw','bad-hash','no-operator','no-calibration','backwards','truncated','wrong-sha','wrong-peer','stale','wrong-destination','missing-change']:
    x=copy.deepcopy(d)
    if kind=='unobserved':x['observed']=False
    elif kind=='missing-raw':x['raw_files']=[]
    elif kind=='bad-hash':x['raw_files'][0]['sha256']='0'*64
    elif kind=='no-operator':x['operator']=None
    elif kind=='no-calibration':x['calibration']={}
    elif kind=='backwards':x['events'][1]['local_ms']=-1
    elif kind=='truncated':x['events'][0]['frame_hex']='00'
    elif kind in ['wrong-sha','wrong-peer','wrong-destination']:
     i=1 if kind=='wrong-destination' else 0;b=bytearray.fromhex(x['events'][i]['frame_hex']);b[{'wrong-sha':27,'wrong-peer':31,'wrong-destination':5}[kind]]^=1;x['events'][i]['frame_hex']=b.hex()
    elif kind=='stale':x['events'][1]['local_ms']=30000
    else:x['events']=x['events'][:2]
    controls.append(dict(control=kind,rejected=bool(validate(x,root))))
   result=dict(passed=all(c['rejected'] for c in controls),controls=controls,physical_claim=False)
 else:
  if not a.capture:p.error('capture or --self-test required')
  errors=validate(json.loads(a.capture.read_text()),a.capture.parent);result=dict(passed=not errors,errors=errors,physical_claim=False,limits='Structure, identities and finite selected byte relations only; original raw evidence and operator/clock/driver review remain required.')
 print(json.dumps(result,indent=2));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
