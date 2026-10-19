"""Independent service accounting and relabeling relations, not a TCP oracle."""
from pathlib import Path
from fractions import Fraction
import argparse,json,re,subprocess
p=argparse.ArgumentParser();p.add_argument('executable');p.add_argument('--trace',type=Path,required=True);a=p.parse_args();r=subprocess.run([a.executable],capture_output=True,text=True,timeout=120);assert r.returncode==0,(r.returncode,r.stdout,r.stderr)
rows=[]
for line in r.stdout.splitlines():
 if line.startswith('NR_SIM '):rows.append({k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',line)})
assert len(rows)==17 and {x['case'] for x in rows}==set(range(12))
by={(x['case'],x['flow']):x for x in rows}
for x in rows:
 assert x['delivered']==8192 and 0<x['at100']<=8192 and x['packets']>=128
 # Whole-block model: initial128 sends plus each retransmission, one per loss event.
 assert x['packets']==128+x['fast']+x['rto']
for f in range(2):
 assert {k:v for k,v in by[5,f].items() if k not in ['case','flow']}=={k:v for k,v in by[8,1-f].items() if k not in ['case','flow']}
assert by[0,0]['rto']==by[9,0]['rto']==0 and by[9,0]['acks']<by[0,0]['acks']
assert by[9,0]['finish']>by[0,0]['finish'] and by[9,0]['at100']<by[0,0]['at100']
assert by[1,0]['fast']>0 and by[1,0]['rto']==0 and by[1,0]['drops']==1
assert by[2,0]['fast']>0 and by[2,0]['rto']>0 and by[2,0]['drops']==2
assert by[3,0]['drops']==0 and by[3,0]['fast']>0
fairness=[]
for case in range(4,9):
 x,y=[by[case,f]['at100'] for f in range(2)];j=Fraction((x+y)**2,2*(x*x+y*y));assert Fraction(1,2)<=j<=1
 fairness.append(dict(case=case,horizon_ms=100,delivered=[x,y],jain_exact=str(j),completed_transfer_jain='1 (vacuous because workloads are equal)'))
d=dict(passed=True,command=[a.executable],returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,rows=rows,fairness=fairness,scope='Fixed1packet/ms bottleneck, finite8192byte flows, fixed service order/latencies; not Internet fairness or physical throughput.')
a.trace.parent.mkdir(parents=True,exist_ok=True);a.trace.write_text(json.dumps(d,indent=2)+'\n');print('NR_SIM_ANALYSIS_PASS twelve_schedules seventeen_flows label_swap delayed_ACK loss_recovery')
