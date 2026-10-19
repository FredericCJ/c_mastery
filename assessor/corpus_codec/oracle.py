"""Independent codeword-string, prefix-slot and shortest-path checks.

No implementation output supplies an expected decompressed byte. The Huffman
small-alphabet cost oracle exhausts Kraft-feasible lengths, not greedy merges.
"""
import itertools,json,subprocess,sys
from collections import Counter
from fractions import Fraction

def codes(lengths):
    if any(x>15 for x in lengths):raise ValueError('length')
    if sum((Fraction(1,2**x) for x in lengths if x),Fraction())>1:raise ValueError('oversubscribed')
    result={};value=0;previous=0
    for width,symbol in sorted((n,s) for s,n in enumerate(lengths) if n):
        value <<= width-previous;result[symbol]=format(value,'0%db'%width);value+=1;previous=width
    return result

def hdecode(data):
    if not 18<=len(data)<=978:raise ValueError('frame')
    n=int.from_bytes(data[:2],'little')
    if n>512:raise ValueError('count')
    dictionary={v:k for k,v in codes(data[2:18]).items()}
    text=''.join(format(b,'08b') for b in data[18:]);at=0;out=[]
    for _ in range(n):
        matches=[(word,symbol) for word,symbol in dictionary.items() if text.startswith(word,at)]
        if len(matches)!=1:raise ValueError('code')
        word,symbol=matches[0];at+=len(word);out.append(symbol)
    if (at+7)//8!=len(data)-18 or '1' in text[at:]:raise ValueError('suffix')
    return bytes(out)

def hframe(symbols,lengths):
    book=codes(lengths);bits=''.join(book[x] for x in symbols);bits+='0'*((-len(bits))%8)
    return len(symbols).to_bytes(2,'little')+bytes(lengths)+bytes(int(bits[i:i+8],2) for i in range(0,len(bits),8))

def ldecode(tokens):
    if len(tokens)>256:raise ValueError('tokens')
    out=[];it=iter(tokens)
    try:
        for tag in it:
            if tag==0:out.append(next(it))
            elif tag==1:
                length,distance=next(it),next(it)
                if not 3<=length<=16 or not 1<=distance<=min(32,len(out)):raise ValueError('match')
                for _ in range(length):out.append(out[-distance])
            else:raise ValueError('tag')
            if len(out)>128:raise ValueError('budget')
    except StopIteration:raise ValueError('truncated')
    return bytes(out)

def legal_matches(data,i):
    # Build the periodic output of a candidate window, independently of an
    # encoder's comparison of future source positions.
    for distance in range(1,min(i,32)+1):
        seed=data[i-distance:i];prediction=(seed*16)[:16]
        for length in range(3,min(16,len(data)-i)+1):
            if data[i:i+length]==prediction[:length]:yield length,distance

def optimum(data):
    # Forward shortest paths over byte positions; target uses backward DP.
    cost=[10**6]*(len(data)+1);cost[0]=0
    for i in range(len(data)):
        cost[i+1]=min(cost[i+1],cost[i]+2)
        for length,_ in legal_matches(data,i):cost[i+length]=min(cost[i+length],cost[i]+3)
    return cost[-1]

def greedy(data):
    result=[];i=0
    while i<len(data):
        choices=list(legal_matches(data,i))
        if choices:
            length,distance=min(choices,key=lambda x:(-x[0],x[1]));result.extend((1,length,distance));i+=length
        else:result.extend((0,data[i]));i+=1
    return bytes(result)

def run(executable):
    requests=[];checks=[]
    def add(op,data,check):requests.append(op+' '+(data.hex() or '-'));checks.append(check)
    vectors=[b'',bytes(range(128)),b'a'*128,b'xy'*64,bytes((i*31)%256 for i in range(128))]
    vectors += [bytes(x) for n in range(11) for x in itertools.product(range(2),repeat=n)]
    for data in vectors:
        add('le',data,lambda s,b,d=data: s==0 and b==greedy(d))
        add('lo',data,lambda s,b,d=data: s==0 and len(b)==optimum(d) and ldecode(b)==d)
        add('ld',greedy(data),lambda s,b,d=data:s==0 and b==d)
        add('ce',data,lambda s,b,d=data:s==0 and ldecode(bytes((x<<4)|y for x,y in zip(hdecode(b)[::2],hdecode(b)[1::2])))==d)
        add('co',data,lambda s,b,d=data:s==0 and len(hdecode(b))//2==optimum(d) and ldecode(bytes((x<<4)|y for x,y in zip(hdecode(b)[::2],hdecode(b)[1::2])))==d)
    # Independent valid frames include incomplete books and unused symbols.
    for lengths in ([1]+[0]*15,[2,2,2,2]+[0]*12,[3,1,3]+[0]*13):
        alphabet=[i for i,x in enumerate(lengths) if x]
        for n in range(8):
            symbols=bytes(alphabet[i%len(alphabet)] for i in range(n));frame=hframe(symbols,lengths)
            add('hd',frame,lambda s,b,d=symbols:s==0 and b==d)
            for cut in range(len(frame)):add('hd',frame[:cut],lambda s,b:s!=0)
            add('hd',frame+b'\0',lambda s,b:s!=0)
    for n in range(5):
        for values in itertools.product(range(1,4),repeat=n):
            freq=bytes(values)+(b'\0'*(16-n))
            if n>=2:
                best=min(sum(w*l for w,l in zip(values,lengths)) for lengths in itertools.product(range(1,n),repeat=n) if sum(Fraction(1,2**l) for l in lengths)<=1)
            else:best=sum(values)
            add('lengths',freq,lambda s,b,v=values,c=best:s==0 and len(b)==16 and sum(w*l for w,l in zip(v,b))==c and all(x==0 for x in b[len(v):]))
    for lengths in ([1,1,1]+[0]*13,[16]+[0]*15):add('book',bytes(lengths),lambda s,b:s!=0)
    malformed=[b'\x00',b'\x01',b'\x01\x03',b'\x01\x03\x01',b'\x02',bytes([0,7,1,16,0]),bytes([0,7,1,17,1]),bytes([0,7])+bytes([1,16,1])*8]
    for bad in malformed:add('ld',bad,lambda s,b:s!=0)
    run=subprocess.run([str(executable)],input='\n'.join(requests)+'\n',text=True,capture_output=True)
    assert run.returncode==0,run.stderr
    lines=run.stdout.splitlines();assert len(lines)==len(checks),(len(lines),len(checks))
    for index,(line,check) in enumerate(zip(lines,checks)):
        status,payload=line.split(' ',1);data=bytes.fromhex(payload) if payload!='-' else b''
        assert check(int(status),data),(index,requests[index],line)
    witness=next((d.hex() for d in vectors if len(greedy(d))>optimum(d)),None)
    assert witness is not None,'vacuous greedy/optimal comparison'
    result=dict(passed=True,cases=len(checks),raw_vectors=len(vectors),greedy_counterexample=witness,oracle='prefix strings/Kraft exhaustive lengths/forward shortest paths',limits=['Finite original inputs; no arbitrary-format or statistical compression guarantee.'])
    print(json.dumps(result));return result
if __name__=='__main__':run(sys.argv[1])
