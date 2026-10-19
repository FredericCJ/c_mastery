"""Independent rational/ordinal certificate for the original and changed model."""
from fractions import Fraction as F
import json,pathlib,sys
lines=pathlib.Path(sys.argv[1]).read_text().splitlines();case=sys.argv[2]
if case=='equation':
 rows=[list(map(int,s.split()[1:])) for s in lines if s.startswith('EQUATION ')];assert len(rows)==5
 base=1200000*F(3,2)/2000000000*10**9
 for row,communication in zip(rows,[0,359999,360000,360001,500000]):
  new=base*(1-F(3,5)+F(3,5)/3)+communication;speed=base/new;limit=base/(base*F(2,5)+communication);assert row==[communication,int(base),int(new),speed.numerator,speed.denominator,limit.numerator,limit.denominator]
 small=F(10**9,21);new=small*(1-F(1,7)+F(1,7)/F(5,2));assert [s for s in lines if s.startswith('FRACTION ')]==[f'FRACTION {small.numerator} {small.denominator} {new.numerator} {new.denominator}'];packets=6
elif case=='stats':assert 'STATS 100 95 96 110 93 120 2 1' in lines;packets=1
elif case=='equal':assert 'EQUAL 5125' in lines;packets=5125
else:raise AssertionError(case)
print(json.dumps(dict(status='pass',packets=packets)))
