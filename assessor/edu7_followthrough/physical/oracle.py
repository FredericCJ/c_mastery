"""Independent exact rational divider/time checks against captured observations."""
from fractions import Fraction
import json,pathlib,sys
text=pathlib.Path(sys.argv[1]).read_text();which=sys.argv[2];keys=['physical','tx','grant','port'] if which=='all' else [which]
if 'physical' in keys:
 assert 'EP_PHYSICAL_VALUES 42000000 365 729 96 10000 650 600 11000' in text
 assert Fraction(96*10*365*1000000,42000000)<10000<Fraction(96*10*729*1000000,42000000)
 assert (650+999)//1000==1 and (200+999)//1000==1
if 'tx' in keys:
 assert 'EP_TX_VALUES 96 12 3 9036' in text
 assert (96+7)//8==12 and 200+95*90+86+200==9036
if 'grant' in keys:
 assert 'EP_GRANT_VALUES 1500 9500 2200 10000' in text
 assert 200<=1500<=2200 and 9500-1500<=10000 and 9500>2200
if 'port' in keys:assert 'EP_PORT_VALUES 139 17 51 2' in text and (16000000+115200//2)//115200==139
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),limits='Exact rational software model; no measured bus time or analog waveform.')))
