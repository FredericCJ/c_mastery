"""Independent host mathematical oracles; never linked into the target component."""
import itertools, re, subprocess, sys
commands, expected, counts = [], [], {}
for x in range(-128,128):
    for y in range(-128,128):
        product=x*y
        quotient,remainder=divmod(abs(product),128)
        magnitude=quotient+(2*remainder>=128)
        answer=max(-128,min(127,-magnitude if product<0 else magnitude))
        commands.append(f'q {x} {y}')
        expected.append(f'0 {answer}')
counts['q7_pairs']=len(commands)
grammar=re.compile(rb'(?:0|-?[1-9][0-9]*)\Z')
tokens=[bytes(p) for n in range(5) for p in itertools.product(b'019-+ \0x',repeat=n)]
tokens += [s.encode() for s in ('2147483647','2147483648','-2147483648','-2147483649','999999999999999999999999x')]
for token in tokens:
    value=12345
    if not grammar.fullmatch(token): status=1
    else:
        number=int(token)
        status=0 if -(2**31)<=number<2**31 else 2
        if status==0:value=number
    commands.append('p '+str(len(token))+' '+' '.join(map(str,token)))
    expected.append(f'{status} {value}')
counts['parser_spans']=len(tokens)
counts['stable_sequences']=0
for n in range(8):
    for keys in itertools.product((-1,0,1),repeat=n):
        commands.append('s '+str(n)+' '+' '.join(map(str,keys)))
        expected.append(' '.join(map(str,[0]+sorted(range(n),key=lambda i:keys[i]))))
        counts['stable_sequences']+=1
model={}
state=1
for step in range(20000):
    state=(1664525*state+1013904223)%(2**32)
    key=((state>>8)%21)*7
    action='prg'[state%3]
    value=step
    answer=12345
    if action=='p':
        status=0 if key in model or len(model)<7 else 3
        if status==0:model[key]=value
    elif action=='r':
        status=0 if key in model else 4
        model.pop(key,None)
    else:
        status=0 if key in model else 4
        if status==0:answer=model[key]
    commands.append(f'm {action} {key} {value}')
    expected.append(f'{status} {len(model)} {answer}')
counts['map_transitions']=20000
r=subprocess.run([sys.argv[1]],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=45)
if r.returncode:raise SystemExit('B02_ORACLE: driver failed: '+r.stderr)
observed=r.stdout.splitlines()
if len(observed)!=len(expected):raise SystemExit('B02_ORACLE: output count mismatch')
for i,(a,b) in enumerate(zip(observed,expected)):
    if a!=b:raise SystemExit(f'B02_ORACLE: {commands[i]} expected {b!r}, observed {a!r}')
print('independent mathematical oracle passed',counts)
