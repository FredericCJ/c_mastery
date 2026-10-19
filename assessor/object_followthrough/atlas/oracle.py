"""Independent finite placement enumeration and canonical byte calculation."""
import json,pathlib,subprocess,sys
exe=sys.argv[1];prefix=json.loads(sys.argv[3]) if len(sys.argv)>3 else [];packets=[]
def run(arg):
    p=subprocess.run([*prefix,exe,arg],capture_output=True,text=True,timeout=30);packets.append(dict(command=[*prefix,exe,arg],returncode=p.returncode,stdout=p.stdout,stderr=p.stderr));assert p.returncode==0,p.stderr;return p.stdout
rows=run('--table').splitlines();assert len(rows)==3690
for line in rows:
    start,length,width,alignment,rc,out=map(int,line.split(','))
    admitted=alignment>0 and alignment&(alignment-1)==0 and start<=32 and length<=32-start
    places=[p for p in range(start,start+length+1) if admitted and p%alignment==0 and p+width<=start+length]
    expected=(0,places[0]) if places else (-1,99)
    assert (rc,out)==expected,(line,expected)
text=run('3');expected=bytes([ord('z')])+((-2)%(1<<32)).to_bytes(4,'big')+bytes.fromhex('3ff8000000000000');assert 'OA_CANONICAL:'+expected.hex() in text
pathlib.Path(sys.argv[2]).write_text(json.dumps(dict(packets=packets),indent=2)+'\n')
print(json.dumps(dict(status='pass',placement_cases=3690,canonical=expected.hex(),limits='finite offset enumeration and explicitly chosen binary64 encoding; no lifetime/provenance proof')))
