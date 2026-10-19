"""Independent selected record validity and bounded stack arithmetic models."""
def check(text):
 lines=text.splitlines()
 if len(lines)!=3072:raise ValueError('Incomplete record/budget matrix')
 for k,line in enumerate(lines[:1024]):
  valid=17|(2 if k&2 else 0)|(4 if k&16 else 0)|(8 if not k&(32|128|256|512) else 0)
  expected=[k,valid,0xdeadbeef if k&2 else 0,0xcafebabe if k&16 else 0,int(bool(k&1)),int(bool(k&2)),int(bool(k&64)),int(bool(k&4)),0]
  if list(map(int,line.split()))!=expected:raise ValueError('Record validity mismatch '+str(k))
 for k,line in enumerate(lines[1024:]):
  parts=line.split();required=(k&15)*16+((k>>4)&15)*16+(104 if k&256 else 32)+(4 if k&512 else 0)
  expected=[k,8,77,77] if k&1024 else [k,0,required,int(required<=256)]
  if parts[0]!='B' or list(map(int,parts[1:]))!=expected:raise ValueError('Budget mismatch '+str(k))
 return {'record_cases':1024,'budget_cases':2048,'scope':'status/address validity vs readable/basic/complete frame and explicit live-frame bounds; no hardware fault injection'}
