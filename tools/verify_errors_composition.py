"""Independent tabulated partial functions, bounded error products and owner counts."""
import argparse
import hashlib
import itertools
import json
import subprocess

def run(driver):
    maps=[[(x+i)%8 for x in range(8)] for i in range(8)]+[[x^i for x in range(8)] for i in range(8)]
    binds=[[(0,(x+add)%8) if x<threshold else (3,100+x) for x in range(8)] for threshold in range(9) for add in range(8)]
    rows=[];expected=[];counts={}
    for kind,tables in [('M',maps),('B',binds)]:
        before=len(rows)
        for a,b,tag,x in itertools.product(range(len(tables)),range(len(tables)),[0,3,5,99],range(8)):
            state=(tag,x) if tag<=8 else (2,0);f=g=ft=gt=0
            if state[0]==0:
                f=1;ft=state[1]+1;v=tables[a][state[1]];state=(0,v) if kind=='M' else v
            if state[0]==0:
                g=1;gt=state[1]+1;v=tables[b][state[1]];state=(0,v) if kind=='M' else v
            line=f'{state[0]} {state[1]} {f} {g} {ft} {gt}'
            rows.append(f'{kind} {a} {b} {tag} {x}');expected.append(line+' '+line)
        counts['map_cases' if kind=='M' else 'bind_cases']=len(rows)-before
    before=len(rows)
    for n,domain,code,offset in itertools.product(range(6),range(3),range(3),[0,7,4294967295]):
        rows.append(f'W {n} {domain} {code} {offset}')
        status=1 if n not in range(1,5) or not domain or not code else 4 if n==4 else 0
        expected.append(f'0 {n+1} 1 2 {offset} {domain} 77' if status==0 else f'{status} 99 91 92 93 0 0')
    counts['cause_cases']=len(rows)-before;before=len(rows)
    for fail,value in itertools.product(range(4),range(8)):
        rows.append(f'O {fail} 0 0 {value}')
        if fail==1:expected.append('4 1 0 91 0 1 0 0')
        elif fail==2:expected.append('0 3 0 91 1 2 1 0')
        else:expected.append(f'0 0 1 {-value} 2 2 2 0')
    counts['owner_cases']=len(rows)-before
    p=subprocess.run([driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True,timeout=120);assert p.returncode==0,p.stderr
    actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,**counts,trace_sha256=hashlib.sha256(('\n'.join(rows+expected)+'\n').encode()).hexdigest(),model='16 total map tables and72 partial result tables over0..7; all listed pairs/states, not every conceivable function')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
