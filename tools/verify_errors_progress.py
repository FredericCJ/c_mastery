"""Independent product-state and epoch admission tables, not C result copies."""
import argparse
import hashlib
import itertools
import json
import subprocess

def run(driver):
    rows=[];expected=[];sizes={}
    for length,next,count,code in itertools.product(range(6),range(7),range(7),list(range(9))+[99]):
        offered=max(0,length-next);calls=0 if next>=length else 1
        if next>length:status,value=2,next
        elif next==length:status,value=0,next
        elif count>offered or code not in [0,4,5] or (code==0 and count==0):status,value=7,next
        else:status,value=code,next+count
        rows.append(f'0 {length} {next} {count} {code} 0');expected.append(f'{status} {value} {calls} {offered if calls else 0}')
    sizes['transfer_cases']=len(rows)
    for length,count,code in itertools.product(range(6),range(9),list(range(9))+[99]):
        offered=length+2;bad=count>offered or code not in [0,4,5] or (code==0 and count==0)
        status=10 if bad else 5 if code==5 else 8 if count==offered and code==0 else 4
        rows.append(f'1 {length} 0 {count} {code} 0');expected.append(f'{status} {0 if bad else count} 0 {2 if bad else 0} 1 {offered}')
    sizes['composed_cases']=len(rows)-sizes['transfer_cases'];start=len(rows)
    for length in range(6):
        for accepted,prefix,code,epoch in itertools.product(range(length+3),range(10),range(4),range(3)):
            durable=ep=unknown=0;calls=1 if accepted else 0
            if not accepted:status=0
            elif code==2:status=7;unknown=1
            elif code>1 or prefix>accepted or (epoch!=(1 if prefix else 0)) or (code==0 and prefix==0):status=10;unknown=1
            else:status=5 if code==1 else 0;durable=prefix;ep=epoch
            rows.append(f'2 {length} {accepted} {prefix} {code} {epoch}');expected.append(f'{status} {accepted} {durable} {ep} {unknown} {calls}')
    sizes['epoch_cases']=len(rows)-start
    p=subprocess.run([driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(expected)
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,**sizes,trace_sha256=hashlib.sha256(('\n'.join(rows+expected)+'\n').encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
