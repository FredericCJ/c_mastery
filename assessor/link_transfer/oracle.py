"""Independent finite expected observations, not an independent human review."""
import json,pathlib,sys
EXPECTED={'serial':[sum(m*m for m in range(2,66)),15,0,1],
 'sizes':[], 'borrow':[257*17,258,1,0],'bridge':[6,5,1,2,3],
 'phy':[0x2000,0x5c90,100,10,15],'partial':[0,10+100,50,52]}
n=12
for header,alignment,tag in [(3,8,2),(5,7,1)]:
 padded=((n+header+alignment-1)//alignment)*alignment;n=padded+(padded+15)//16+tag;EXPECTED['sizes'].append(n)
EXPECTED['sizes'] +=[91,92]
t=pathlib.Path(sys.argv[1]).read_text();keys=list(EXPECTED) if sys.argv[2]=='all' else [sys.argv[2]]
for key in keys:
 line='LT_'+key.upper()+'_PASS '+' '.join(map(str,EXPECTED[key]));assert line in t,(key,line)
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),limits='Independent arithmetic and selected hand-derived packets only.')))
