"""Enumerate candidate prefix ends against independent regexes, not DFA tables."""
import itertools,json,random,re,subprocess,sys
SEED=150020
rules=[re.compile(r'abc\Z'),re.compile(r'(?:abc)*d\Z'),re.compile(r'abc\Z')]
def expected(s):
    begin=0;tokens=[]
    while begin<len(s):
        candidates=[(end,rule) for end in range(begin+1,len(s)+1) for rule,rx in enumerate(rules) if rx.fullmatch(s[begin:end])]
        if not candidates:return '1 '+str(begin)
        end,rule=min(candidates,key=lambda v:(-v[0],v[1]));tokens.append(f'{begin}:{end}:{rule}');begin=end
    return '0 '+str(len(tokens))+(' '+' '.join(tokens) if tokens else '')
def main():
    r=random.Random(SEED);values=[''.join(t) for n in range(7) for t in itertools.product('abcd',repeat=n)]
    values += [''.join(r.choice(['abc','d','abcd']) for _ in range(r.randrange(1,30))) for _ in range(500)]
    values += ['abc'*n+suffix for n in [1,2,3,20,85] for suffix in ['', 'd', 'ab','x'] if 3*n+len(suffix)<=256]
    cases=[(m,s) for m in [0,1] for s in values];p=subprocess.run([sys.argv[-1]],input=''.join(f'{m} {s}\n' for m,s in cases),capture_output=True,text=True,timeout=120)
    assert p.returncode==0,p.stderr
    lines=p.stdout.splitlines();assert len(lines)==len(cases)
    for (mode,s),line in zip(cases,lines):assert line==expected(s),(mode,s,line,expected(s))
    print(json.dumps(dict(passed=True,seed=SEED,observations=len(cases),exhaustive_max_length=6,scope='Independent Python regex enumeration over every candidate end, both modes; finite alphabet/string bound.')))
if __name__=='__main__':main()
