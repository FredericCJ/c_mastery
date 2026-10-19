"""Original local counter target: arbitrary-precision independent state oracle."""
import argparse,hashlib,json,random,struct,subprocess

def cases():
    for authority in [0,1]:
        for old in [-2**31,-1,0,1,10,2**31-1]:
            for flags in range(4):
                for count in [0,1,2,255,65535]:
                    for value in [-2**31,-1,0,1,3,2**31-1]:
                        yield struct.pack('>BBHi',1,flags,count,value),authority,old
    rng=random.Random(0xB0135)
    for _ in range(1024):
        yield bytes(rng.randrange(256) for _ in range(rng.randrange(10))),rng.randrange(2),rng.randrange(-2**31,2**31)
    for n in range(10):yield bytes([1,0,0,2,0,0,0,3,0][:n]),0,10

def verify(driver):
    requests=[];expected=[]
    for wire,authority,old in cases():
        requests.append(' '.join(map(str,[len(wire),authority,old,*wire])))
        value=old
        if len(wire)!=8:status=2
        elif wire[0]!=1 or wire[1]>3:status=1
        else:
            count=int.from_bytes(wire[2:4],'big');step=int.from_bytes(wire[4:8],'big',signed=True)
            proposed=old+count*step
            if not -2**31<=proposed<2**31:status=2
            elif not authority:status=8
            else:status=0;value=proposed
        expected.append(f'{status} {value}')
    run=subprocess.run([str(driver)],input='\n'.join(requests)+'\n',text=True,capture_output=True,timeout=60)
    assert run.returncode==0,run.stderr
    actual=run.stdout.splitlines();assert len(actual)==len(expected)
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(requests[i],a,b)
    return dict(passed=True,cases=len(expected),structured=1440,random_cases=1024,seed=0xB0135,trace_sha256=hashlib.sha256(('\n'.join(expected)+'\n').encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('driver');args=p.parse_args();print(json.dumps(verify(args.driver)))
