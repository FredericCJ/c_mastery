"""Independent symbolic slot authority histories; never reads C representation."""
from itertools import product
import argparse,hashlib,json,subprocess

def histories():
    alphabet=[('A',0),('A',1),('L',0),('L',1),('G',0),('G',1),('F',0),('I',1)]
    yield from product(alphabet,repeat=5)
    # A complete finite generation cycle supplements short exhaustive histories.
    yield tuple([('A',0),('G',0),('L',0)]*10+[('A',1),('G',0),('L',0)])

def requests(maximum):
    lines=[];expected=[];count=0;steps=0
    for seq in histories():
        # Model identity is mathematical index and issue count, not C addresses.
        free=[True]*3;issued=[0]*3;contents=[None]*3;registers=[(maximum,0)]*2
        lines.append(str(len(seq)));expected.append('R');count+=1
        for n,(action,r) in enumerate(seq):
            steps+=1;value=n+1;i,g=registers[r]
            valid=i<3 and not free[i] and issued[i]==g
            if action=='A':
                lines.append(f'A {r} {value}');available=[j for j in range(3) if free[j] and issued[j]<3]
                if available:
                    i=available[0];issued[i]+=1;free[i]=False;contents[i]=(value,value+100);registers[r]=(i,issued[i]);status=0
                else:status=7
                i,g=registers[r];expected.append(f'A {status} {i} {g}')
            elif action=='L':
                lines.append(f'L {r}');expected.append(f'L {0 if valid else 6}')
                if valid:free[i]=True
            else:
                lines.append(f'{action} {r}');success=action=='G' and valid
                a,b=contents[i] if success else (917,919)
                expected.append(f'{action} {0 if success else 6} {a} {b}')
    return lines,expected,count,steps

def verify(executable):
    identity=subprocess.run([str(executable)],input='',text=True,capture_output=True,check=True)
    maximum=int(identity.stdout.strip().split()[1])
    lines,want,count,steps=requests(maximum)
    p=subprocess.run([str(executable)],input='\n'.join(lines)+'\n',text=True,capture_output=True,timeout=90)
    assert p.returncode==0,p.stderr
    got=p.stdout.splitlines();assert got.pop(0)==f'SIZE_MAX {maximum}'
    assert len(got)==len(want),(len(got),len(want))
    for i,(a,b) in enumerate(zip(got,want)):assert a==b,(i,a,b)
    return dict(passed=True,histories=count,transitions=steps,trace_sha256=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest())

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('driver');a=p.parse_args();print(json.dumps(verify(a.driver)))
