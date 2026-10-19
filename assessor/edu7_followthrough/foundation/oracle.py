"""Independent polynomial division and explicit wire syntax, not the C recurrence."""
import json,pathlib,sys
text=pathlib.Path(sys.argv[1]).read_text();case=sys.argv[2]
def crc(p):
 value=(65535<<(8*len(p)))^(int.from_bytes(p,'big')<<16)
 while value.bit_length()>16:value^=0x11021<<(value.bit_length()-17)
 return value
assert crc(b'123456789')==0x29b1
def frame(version):
 body=bytes([version,0,1,2,40])+bytes([0x7e,0x7d])*20
 body+=crc(body).to_bytes(2,'big');wire=b'\x7e'
 for byte in body:wire+=bytes([0x7d,byte^(0x20 if version==1 else 0x40)]) if byte in [0x7d,0x7e] else bytes([byte])
 return (wire+b'\x7e').hex()
packets={'ledger':'EF_LEDGER_PASS schema1=33,40,2,16 schema2=45,52,3,4 envelope=80,87,4,15 link=47,96 radio_keeps_address_grant_crc','framing':'EF_FRAMING_PASS all_splits_noise_escape_error_two_completions_valid_crc_forgery_no_authentication','router':'EF_ROUTER_PASS copy_poison reverse_next_hop hop1_local borrow_release_generation busy_atomic route68_profile2','codec':'EF_CODEC_PASS f060 capacity_atomic UTF8_16 widened_i32 schema2_no_v1_authority display_octets_only'}
keys=list(packets) if case=='all' else [case]
for k in keys:assert text.count(packets[k])==1,(k,text)
if 'framing' in keys:
 for version in [1,2]:assert 'EF_V'+str(version)+'\n'+frame(version)+'\n' in text
if 'codec' in keys:
 ordinary=bytes([1,1,1,2,2,2,0,42,3,2])+(-4000).to_bytes(2,'big',signed=True)+bytes([4,1,1]);wide=bytes([2,1,1,2,2,2,0,42,3,4])+(-2**31).to_bytes(4,'big',signed=True)+bytes([4,1,1,5,3])+b'%n%'+bytes([6,8])+b'j'*8
 assert 'EF_SET\n'+ordinary.hex()+'\n' in text and 'EF_WIDE\n'+wide.hex()+'\n' in text
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),method='Polynomial remainder and Python signed serialization independent of C loops; bounded literal transition witnesses.')))
