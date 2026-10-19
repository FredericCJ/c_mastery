"""Independent mathematical set-membership authority oracle, no C implementation calls."""
import argparse,hashlib,itertools,json,subprocess
def cases():
    for pt,t,pr,r,op,e,o,n in itertools.product(range(2),range(3),range(4),range(5),range(2),[0,1,2,4294967295],[0,1,3,4,8,4294967292,4294967295],[0,1,2,4,8,4294967295]):
        yield (pt,t,pr,r,op,e,o,n,2,4)
def expected(c):
    pt,t,pr,r,op,e,o,n,start,length=c;a=list(range(90,106));out=[-9]*8
    # Set membership and mathematical addition, independent of unsigned
    # subtraction/byte indexing in the C implementation.
    if t not in {0,1} or r not in {0,1,2,3} or o not in range(length+1) or n<0 or o+n>length:s=1
    elif e!=1:s=3
    elif t!=pt or (set(i for i in range(2) if r&(1<<i))-set(i for i in range(2) if pr&(1<<i))) or not r&(2 if op else 1):s=2
    else:
        s=0;positions=range(pt*8+start+o,pt*8+start+o+n)
        if op:
            for i in positions:a[i]=7
        else:out[:n]=[a[i] for i in positions]
    return ','.join(map(str,[s,*a,*out]))
def run(driver):
    rows=list(cases());inputs=[' '.join(map(str,c)) for c in rows];expected_rows=[expected(c) for c in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(rows)
    for i,(a,b) in enumerate(zip(actual,expected_rows)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,cases=len(rows),trace_sha256=hashlib.sha256(('\n'.join(inputs+expected_rows)+'\n').encode()).hexdigest(),limits='Trusted parent span2..6, all rights, two root tenants, three request tenants, epochs0/1/2/MAX, boundary/extreme extents. Revocation, byte codec and overlapping decode are separate anchor tests.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
