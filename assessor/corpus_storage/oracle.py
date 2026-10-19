"""Independent dictionaries/version copies and run records; no C node algorithm."""
import argparse,itertools,json,random,subprocess
parser=argparse.ArgumentParser();parser.add_argument('--driver');args=parser.parse_args()
driver=args.driver;commands=[];expected=[]
def row(family,op,a=0,b=0,c=0):commands.append(f'{family} {op} {a} {b} {c}')
def dump(status,values,versions=None):
    result=[str(status)]+[f'0:{values[k]}' if k in values else '1:0' for k in range(32)]
    if versions is not None:
        for v in versions:result += ['5:0']*32 if v is None else [f'0:{v[k]}' if k in v else '1:0' for k in range(32)]
    expected.append(' '.join(result))
rng=random.Random(13579)
for heights in itertools.product([0,1,3,15],repeat=3):
    row('S',9);values={};dump(0,values)
    for key in [2,0,1]:row('S',0,key,key+10,heights[key]);values[key]=key+10;dump(0,values)
    for key in [1,1,0,2]:row('S',1,key);status=0 if key in values else 1;values.pop(key,None);dump(status,values)
for repeat in range(32):
    row('T',9);values={};versions=[None]*7;dump(0,values,versions)
    for key in rng.sample(range(24),24):
        row('T',0,key,key+repeat);values[key]=key+repeat;dump(0,values,versions)
        row('T',4);dump(0,values,versions)
    for v in range(3):
        row('T',2,v);versions[v]=dict(values);dump(0,values,versions)
        for key in range(v,24,4):
            row('T',1,key);status=0 if key in values else 1;values.pop(key,None);dump(status,values,versions)
        row('T',5);dump(0,values,versions)
    for v in range(3):row('T',3,v);versions[v]=None;dump(0,values,versions)
    row('T',4);dump(0,values,versions)
for repeat in range(40):
    row('L',9);memory={};runs=[None]*3;sequence=0;dump(0,{})
    for step in range(128):
        op=rng.randrange(3);a=rng.randrange(32);b=rng.randrange(128);c=rng.randrange(2)
        if op==0:
            status=2 if a not in memory and len(memory)==4 else 0
            if not status:sequence+=1;memory[a]=(sequence,b,c)
            row('L',0,a,b,c)
        elif op==1:
            status=0 if not memory or None in runs else 2
            if memory and not status:runs[runs.index(None)]=dict(memory);memory={}
            row('L',1)
        else:
            mask=rng.randrange(1,8);drop=rng.randrange(2);active=sum(1<<i for i,r in enumerate(runs) if r is not None);selected=[i for i in range(3) if mask>>i&1]
            status=4 if mask&~active or len(selected)<2 or drop and(mask!=active or memory) else 0
            # SS_INVALID is3 (the status layout is independently fixed here).
            if status:status=3
            else:
                merged={}
                for i in selected:
                    for k,v in runs[i].items():
                        if k not in merged or v[0]>merged[k][0]:merged[k]=v
                    runs[i]=None
                if drop:merged={k:v for k,v in merged.items() if not v[2]}
                runs[selected[0]]=merged
            row('L',2,mask,drop)
        all_records={}
        for source in [memory]+[r for r in runs if r is not None]:
            for k,v in source.items():
                if k not in all_records or v[0]>all_records[k][0]:all_records[k]=v
        dump(status,{k:v[1] for k,v in all_records.items() if not v[2]})
if driver:
    p=subprocess.run([driver],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=60)
    assert p.returncode==0,(p.returncode,p.stderr)
    actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(a,b) in enumerate(zip(actual,expected)):
        assert a==b,dict(line=i,input=commands[i],actual=a,expected=b)
# Exhaustive small-state models falsify confusing period, uniform residues and
# independent samples. They supplement, not replace, the 32-bit proof.
periods=[];biased=[]
for bits in range(2,13):
    modulus=1<<bits; state=0;seen=set()
    for _ in range(modulus):
        state=(1664525*state+1013904223)%modulus
        assert state not in seen;seen.add(state)
    assert state==0 and len(seen)==modulus;periods.append(modulus)
    for bound in range(1,min(32,modulus)+1):
        raw=[sum(x%bound==k for x in range(modulus)) for k in range(bound)]
        threshold=modulus%bound
        accepted=[sum(x%bound==k for x in range(threshold,modulus)) for k in range(bound)]
        assert len(set(accepted))==1
        if len(set(raw))>1:biased.append([bits,bound])
assert biased and all(((1664525*x+1013904223)&1)!=(x&1) for x in range(256))
# A finite durability countermodel: publishing a root is insufficient if a
# referenced new page has not reached the declared durable domain. Neither C
# assignment nor the RAM tests above implement these storage transitions.
cuts=[]
for order in [('write-page','flush-page','publish-root'),('write-page','publish-root','flush-page')]:
    for cut in range(4):
        durable_page=False;durable_root=0
        for event in order[:cut]:
            if event=='flush-page':durable_page=True
            elif event=='publish-root':durable_root=1
        safe=not durable_root or durable_page;cuts.append(dict(order=list(order),cut=cut,safe=safe))
assert all(x['safe'] for x in cuts[:4]) and not cuts[6]['safe']
print(json.dumps(dict(passed=True,cases=len(expected),small_state_periods=periods,biased_residue_examples=len(biased),publication_cuts=cuts,scope='Independent finite contents/version/run models and explicit hypothetical durability transitions; no device durability or IID generator claim.')))
