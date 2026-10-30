"""Independent original wire decode and application expectations."""
import json,pathlib,struct,sys
text=pathlib.Path(sys.argv[1]).read_text();which=sys.argv[2];keys=['exchange','identities','faults','adversarial'] if which=='all' else [which]
def decode(wire):
 assert wire[0]==wire[-1]==126
 out=bytearray();escape=False
 for b in wire[1:-1]:
  assert b!=126
  if escape:assert b in [93,94];out.append(b^32);escape=False
  elif b==125:escape=True
  else:out.append(b)
 assert not escape and len(out)==7+out[4] and out[0]==1 and out[1]==0
 crc=65535
 for b in out[:-2]:
  crc^=b<<8
  for _ in range(8):crc=((crc<<1)^0x1021 if crc&0x8000 else crc<<1)&65535
 assert crc==int.from_bytes(out[-2:],'big')
 return out
if 'exchange' in keys:assert 'EW_EXCHANGE_VALUES -4000 1800 1 3 0' in text
if 'identities' in keys:
 frames=[]
 lines=text.splitlines()
 for i in range(4):frames.append(decode(bytes.fromhex(lines[lines.index('EW_WIRE'+str(i))+1])))
 for i,f in enumerate(frames):
  forward=i in [0,2];assert f[2:4]==bytes([1,2] if forward else [2,1]);p=f[5:-2]
  assert p[:6]==bytes([1,17 if forward else 51,51 if forward else 17,4 if i in [0,3] else 3,4,30])
  assert p[6:14]==bytes([1,7 if forward else 8,8 if forward else 7,0,2,0,1,1])
  assert p[14:21]==bytes([3,0,7,0,1,0,1])
  expected=bytes([1,1,1,2 if forward else 3,2,2])+struct.pack('>H',42)+bytes([3,2])+struct.pack('>h',-4000)+bytes([4,1,1])
  assert p[21:]==expected
 assert frames[0][-2:]!=frames[2][-2:] and frames[1][-2:]!=frames[3][-2:]
 assert 'EW_IDENTITIES_VALUES 17 51 7 8 42 1 4 3' in text
if 'faults' in keys:assert 'EW_FAULTS_VALUES 1 2 0 0' in text
if 'adversarial' in keys:assert 'EW_ADVERSARIAL_VALUES 0 0 3 0' in text
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),physical='not-observed',limits='Independent byte/CRC/application oracle; CPU/model captures are not bench evidence.')))
