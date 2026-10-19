"""Rational endpoint oracle and independently enumerated finite observations."""
from fractions import Fraction as F
import json,pathlib,sys
def expected():
 e=[]
 for c in [100,400]:e.append(('nominal',c,967,int(F(300*10000,8473)*1000/c),0 if c==100 else 2))
 for v in range(1800,5001,800):
  for tol in range(0,250001,50000):
   for leak in range(0,201,50):
    lo=F((v+100-400)*1000,3000-leak)/F(1000000-tol,1000000)
    # Minimum charging current throughout30..70% must exceed C*dV/t.
    hi=F(300*v,1)/(F(2*v*100,5*300)+leak)/F(1000000+tol,1000000)
    e.extend([('guaranteed',v,tol,leak,-(-lo.numerator//lo.denominator)),('upper',v,tol,leak,int(hi))])
 e.append(('electrical',150,10000,750000000,2))
 return dict(electrical=e,arbitration=[('arbitration',65536,7,3,5)],transaction=[('transaction',1,90,204,1),('late',1,90,34,2),('transaction',2,90,195,1),('late',2,90,195,2)],failure=[('failure',k,s,0 if k==3 else 1,77) for k,s in enumerate([5,6,5,7,4])],recovery=[('recovery',0,0,0,0)]+[('recovery',n,0,n,8+2*n) for n in range(1,10)]+[('recovery',10,8,9,22),('held',4,0,1,1)],late_publication=[('late_publication',3,17,9,0)],mapped=[('mapped',4294967108,100,17,90)])
def main():
 text=pathlib.Path(sys.argv[1]).read_text();key=sys.argv[2];e=expected();keys=list(e) if key=='all' else [key];want=[r for k in keys for r in e[k]]
 got=[tuple([s.split()[1],*map(int,s.split()[2:])]) for s in text.splitlines() if s.startswith('OBS ')]
 assert got==want,(key,len(got),len(want),next(((a,b) for a,b in zip(got,want) if a!=b),None))
 print(json.dumps(dict(status='pass',keys=keys,packets=len(got),method='Exact rational corner inequalities and separate expected event packets')))
if __name__=='__main__':main()
