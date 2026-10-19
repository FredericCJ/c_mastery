"""Exact-rational calculation, independent of the C rounded-division formula."""
from fractions import Fraction
import json
rates=[245,300,1200,9600,57600,115200,230400,460800,921600,1000000]
expected=[65306,53333,13333,1667,278,139,69,35,17,16]
rows=[]
for rate,want in zip(rates,expected):
 ideal=Fraction(16000000,rate)
 # Explicit minimization over the complete allowed divisor domain. Ties go up.
 best=min(range(16,65536),key=lambda x:(abs(Fraction(x)-ideal),-x))
 assert best==want
 error=(Fraction(16000000,best)-rate)/rate
 rows.append(dict(baud=rate,brr=best,error_fraction=str(error),nominal_percent=float(error*100)))
print(json.dumps(dict(passed=True,rows=rows,scope='nominal divisor rounding; oscillator and receiver error budgets remain separate'),indent=2))
