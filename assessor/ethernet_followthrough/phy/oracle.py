"""Independent exact observations from protocol and finite bus-effect counts."""
import json,pathlib,sys
p,key=sys.argv[1:];actual=[]
for line in pathlib.Path(p).read_text().splitlines():
 if line.startswith('OBS '):actual.append(line.split()[1:])
expected=dict(access=['phy_access','145','102','64','3'],lifecycle=['phy_lifecycle','32768','4608','100','10'],bounded=['phy_bounded','2','1000','0','0'],mapped=['phy_mapped',str(0x40028010),str(0x40028014),'100','8'],leases=['phy_leases','3','4','1','0'],faults=['phy_faults','78','78','0','0'])
# With one busy read, each transaction has6 fallible bus callback points.
# Boot: reset write1 + reset reads3 + restart write1 + restart reads3 + query5.
assert 6*(1+3+1+3+5)==78
wanted=list(expected.values()) if key=='all' else [expected[key]];assert actual==wanted,(actual,wanted)
print(json.dumps(dict(status='pass',packets=len(wanted),keys=list(expected) if key=='all' else [key],limits='Exact finite model packets, not physical PHY or universal conformance')))
