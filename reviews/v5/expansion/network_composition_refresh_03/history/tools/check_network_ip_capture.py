"""Check capture structure and bytes; never authenticate a physical observation."""
from pathlib import Path
import argparse,copy,hashlib,json,tempfile
def checksum(p):
 total=sum(int.from_bytes(p[i:i+2].ljust(2,b'\0'),'big') for i in range(0,len(p),2))
 return 65535-(0 if not total else 1+(total-1)%65535)
def validate(d,base):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('observed') is True,'physical capture not observed')
 for key in ['board','driver_qualification','capture_location','offload_observation','reassembly_evidence']:need(bool(d.get(key)),'missing '+key)
 h=d.get('firmware_sha256');need(isinstance(h,str) and len(h)==64 and all(c in '0123456789abcdef' for c in h),'firmware identity')
 raw=base/str(d.get('raw_capture',''));need(raw.is_file(),'missing raw capture')
 if raw.is_file():need(hashlib.sha256(raw.read_bytes()).hexdigest()==d.get('raw_sha256'),'raw capture identity')
 rows=d.get('echoes',[]);need(len(rows)==3,'three actual Echo sizes')
 sizes=[]
 for row in rows:
  try:
   req=bytes.fromhex(row['request_ip_hex']);rep=bytes.fromhex(row['reply_ip_hex']);n=len(req)-28;sizes.append(n)
   need(row.get('request_index')!=row.get('reply_index') and isinstance(row.get('request_index'),int) and isinstance(row.get('reply_index'),int),'capture indices')
   need(len(req)>=28 and len(rep)==len(req),'capture IP extents')
   for packet in [req,rep]:need(len(packet)>=28 and packet[0]==0x45 and int.from_bytes(packet[2:4],'big')==len(packet) and packet[9]==1 and checksum(packet[:20])==0 and checksum(packet[20:])==0,'IP/ICMP fields or checksums')
   need(req[12:20]==bytes.fromhex('c0000201c0000202') and rep[12:20]==bytes.fromhex('c0000202c0000201'),'addresses')
   need(req[20:22]==b'\x08\x00' and rep[20:22]==b'\0\0' and req[24:]==rep[24:],'Echo semantics')
   need(row.get('pre_fcs_length')==max(60,14+len(rep)),'frame length')
  except (KeyError,ValueError,TypeError):errors.append('malformed Echo record')
 need(sorted(sizes)==[5,56,64],'required payload sizes')
 return errors
def selftest():
 with tempfile.TemporaryDirectory() as td:
  root=Path(td);raw=root/'capture.bin';raw.write_bytes(b'synthetic-not-physical');d=dict(observed=True,board='synthetic',driver_qualification='synthetic',capture_location='synthetic',offload_observation='synthetic',reassembly_evidence='synthetic',firmware_sha256='a'*64,raw_capture=raw.name,raw_sha256=hashlib.sha256(raw.read_bytes()).hexdigest(),echoes=[])
  for n in [5,56,64]:
   req=bytearray.fromhex('450000000000400040010000c0000201c00002020800000012340001')+bytearray(range(n));req[2:4]=len(req).to_bytes(2,'big');req[22:24]=checksum(req[20:]).to_bytes(2,'big');req[10:12]=checksum(req[:20]).to_bytes(2,'big');rep=bytearray(req);rep[12:20]=bytes.fromhex('c0000202c0000201');rep[20]=0;rep[22:24]=b'\0\0';rep[22:24]=checksum(rep[20:]).to_bytes(2,'big');rep[10:12]=b'\0\0';rep[10:12]=checksum(rep[:20]).to_bytes(2,'big');d['echoes'].append(dict(request_index=2*n,reply_index=2*n+1,request_ip_hex=req.hex(),reply_ip_hex=rep.hex(),pre_fcs_length=max(60,len(rep)+14)))
  assert not validate(d,root);controls=0
  for key in ['observed','board','driver_qualification','capture_location','offload_observation','reassembly_evidence','firmware_sha256','raw_sha256']:
   q=copy.deepcopy(d);q[key]=None;assert validate(q,root);controls+=1
  for field,value in [('pre_fcs_length',999),('reply_index',10),('reply_ip_hex','00')]:
   q=copy.deepcopy(d);q['echoes'][0][field]=value;assert validate(q,root);controls+=1
  q=copy.deepcopy(d);q['echoes'].pop();assert validate(q,root);controls+=1
  return controls
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('record',nargs='?',type=Path);p.add_argument('--self-test',action='store_true');a=p.parse_args()
 if a.self_test:print(json.dumps(dict(passed=True,controls=selftest(),physical_claim=False)))
 else:
  if a.record is None:p.error('record required')
  errors=validate(json.loads(a.record.read_text()),a.record.parent);print(json.dumps(dict(passed=not errors,errors=errors,physical_claim=False)));raise SystemExit(int(bool(errors)))
