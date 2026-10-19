"""Logical tuple identity, independent of the C bitmask implementation."""
def check(text):
 rows=text.splitlines();assert len(rows)==32768;seen=set()
 for row in rows:
  e,r,ident,ge,gr,result=map(int,row.split());key=(e,r,ident,ge,gr)
  assert key not in seen;seen.add(key)
  assert all(x in (0,1) for x in (e,r,ge,gr)) and ident in range(2048)
  assert result==int((e,r,0x123)==(ge,gr,ident)),key
 return dict(cases=len(rows),independent_representation='format/request/identifier tuple equality')
