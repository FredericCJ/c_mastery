"""Synthetic positive/negative dossier controls, never physical observations."""
import copy,hashlib,importlib.util,json,pathlib,sys,tempfile
root=pathlib.Path(__file__).resolve().parents[3];p=root/'labs/serial_followthrough/mcp/check_dossier.py';spec=importlib.util.spec_from_file_location('dossier',p);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
with tempfile.TemporaryDirectory() as tmp:
 folder=pathlib.Path(tmp);files=[]
 for name in ['readback','loopback','peer','waveform','changed']:
  q=folder/(name+'.txt');q.write_text('SYNTHETIC FIXTURE; NOT A PHYSICAL OBSERVATION\n'+name);files.append(dict(path=q.name,sha256=hashlib.sha256(q.read_bytes()).hexdigest()))
 d={k:'synthetic-'+k for k in ['board_revision','controller_revision','transceiver_revision','schematic','oscillator','peer_identity','source_identity','build_identity','instrument','probe_points','settings']}
 d.update(kind='synthetic',raw_files=files,spi_readback='readback.txt',loopback='loopback.txt',peer_receive='peer.txt',waveform='waveform.txt',changed_condition='changed.txt',clock_hz=40000000,spi_hz=1000000,fresh_peer_seed=91,fresh_peer_checked=True,physical_gate_waived=False,faults=['disconnected-controller','wrong-spi-mode','missing-ack','filter-reject','queue-overflow','recovery'],runs=[])
 # Independent byte oracle, not imported from the submitted checker.
 for rate,brs in [(1000000,0),(1000000,1),(2000000,0),(2000000,1)]:
  frames=[]
  for i,n in enumerate([0,8,12,16,20,64]):
   payload=bytes(((i*37+j*13)^165)&255 for j in range(n))
   if n:
    payload=bytes([i])+payload[1:];check=0
    for v in payload[:-1]:check^=v
    payload=payload[:-1]+bytes([check])
   frames.append(dict(id=768+i,brs=brs,payload=list(payload)))
  d['runs'].append(dict(configured_data_rate=rate,brs=brs,measured_nominal_rate=500000,measured_data_rate=rate if brs else 500000,sample_ns=10,peer_frames=frames))
 assert module.check(d,folder,True)==[];assert module.check(d,folder,False)
 controls=[]
 for name in ['instrument','changed','raw','sequence','payload','rate','resolution','fresh-peer','required','board','missing-ack','stale-hash']:
  x=copy.deepcopy(d)
  if name in ['instrument','board']:x['instrument' if name=='instrument' else 'board_revision']=''
  elif name=='changed':x['changed_condition']=''
  elif name=='raw':x['raw_files']=[]
  elif name=='sequence':x['runs'][0]['peer_frames'][0]['id']=769
  elif name=='payload':x['runs'][0]['peer_frames'][1]['payload'][0]^=1
  elif name=='rate':x['runs'][3]['measured_data_rate']=500000
  elif name=='resolution':x['runs'][3]['sample_ns']=500
  elif name=='fresh-peer':x['fresh_peer_checked']=False
  elif name=='required':x['physical_gate_waived']=True
  elif name=='missing-ack':x['faults'].remove('missing-ack')
  else:x['raw_files'][0]['sha256']='0'*64
  assert module.check(x,folder,True),name;controls.append(name)
 print(json.dumps(dict(passed=True,synthetic_only=True,controls=controls,physical_observations=0)))
