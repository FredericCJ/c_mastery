"""Aggregate byte-conservation oracles, independent of per-ACK C transitions."""
from pathlib import Path
import argparse,json,subprocess
p=argparse.ArgumentParser();p.add_argument('executable');p.add_argument('--trace',type=Path,required=True);a=p.parse_args();commands=[];expected=[]
for n in range(1,13):
 for mask in range(1<<(n-1)):
  commands.append(f'P {n} {mask} 0');expected.append(f'P {2*n} {(0xfffffff8+n)%2**32} 0')
partitions=len(commands)
for flight in range(1,129):
 for mss in range(1,17):
  threshold=max(flight//2,2*mss);commands.append(f'T {flight} {mss} 0');expected.append(f'T {threshold} {threshold} {mss}')
for mss in range(1,33):
 for segments in range(2,9):
  for stride in [1,mss,mss*segments]:
   commands.append(f'Q {mss} {segments} {stride}');expected.append(f'Q {mss*(segments+1)} {(0xfffffff0+mss*segments)%2**32} 0')
for mss in range(1,17):
 for segments in range(4,9):
  for partial in range(2*mss+1):
   commands.append(f'L {mss} {segments} {partial}');expected.append(f'L {max(mss*segments//2,2*mss)} {2*mss-partial} {2*mss-partial}')
r=subprocess.run([a.executable],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120);actual=r.stdout.splitlines();a.trace.parent.mkdir(parents=True,exist_ok=True);a.trace.write_text(json.dumps(dict(commands=commands,expected=expected,actual=actual,returncode=r.returncode,stderr=r.stderr),indent=2)+'\n');assert r.returncode==0,(r.returncode,r.stderr);assert len(actual)==len(expected),(len(actual),len(expected))
for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(commands[i],x,y)
print(f'NR_ORACLE_PASS cases={len(commands)} ACK_partitions={partitions} thresholds=2048 CA_partitions=672 limited_transmit=1440')
