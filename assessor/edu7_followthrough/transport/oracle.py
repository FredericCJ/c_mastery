"""Independent byte construction, fragment arithmetic and accepted-clock ledger."""
import json,pathlib,struct,sys
t=pathlib.Path(sys.argv[1]).read_text();case=sys.argv[2];keys=['reassembly','retry','expiry'] if case=='all' else [case]
if 'reassembly' in keys:
 f=bytes([1,7,8])+struct.pack('>H',1)+bytes([0,2,1])+bytes(range(24));wire=bytes([1,17,51,4,4,len(f)])+f
 assert 'ET_PACKET_V1\n'+wire.hex() in t and 'ET_LENGTHS 240 full_messages two_profiles duplicate_each_fragment' in t
 assert sum((n+23)//24 for n in range(1,121))==360
if 'retry' in keys:
 assert 'accepted250 350 450 550 terminal650 no_offer_clock' in t
 assert [250+100*i for i in range(5)]==[250,350,450,550,650]
if 'expiry' in keys:
 f=struct.pack('>H',1)+bytes([1,7,8])+struct.pack('>H',1)+bytes([0,2,1])+bytes(24);wire=bytes([1,17,51,4,0x84,len(f)])+f
 assert 'ET_PACKET_V2\n'+wire.hex() in t and 'deadline500 no_duplicate_renewal protocol2_epoch' in t
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),limits='Exact original/explicit-extension byte and bounded ledger oracles; no peer authentication or physical observations.')))
