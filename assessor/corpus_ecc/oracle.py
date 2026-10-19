"""Parity-equation codebook and nearest-codeword oracle, independent of XOR-index decoder."""
import json
import subprocess
import sys

def codebook(n):
    # Search the whole bit-vector space, satisfying each parity equation.
    words=[]
    for word in range(1<<n):
        if word.bit_count()%2:continue
        checks=[]
        power=1
        while power<n:
            checks.append(sum((word>>(p-1))&1 for p in range(1,n) if p&power)%2)
            power*=2
        if not any(checks):words.append(word)
    return words

def data(n,w):
    positions=[p for p in range(1,n) if p.bit_count()!=1]
    return sum(((w>>(p-1))&1)<<i for i,p in enumerate(positions))

def verify(text):
    books={n:codebook(n) for n in [8,16]};seen=set()
    assert len(books[8])==16 and len(books[16])==2048
    # Linearity lets distances from zero establish the minimum distance;
    # also exhaust every small-code pair independently.
    assert min(w.bit_count() for w in books[16] if w)==4
    assert min((a^b).bit_count() for a in books[8] for b in books[8] if a!=b)==4
    for line in text.splitlines():
        fields=list(map(int,line.split()));n=fields[0]
        if n==8:
            _,received,status,value,corrected=fields
            assert (n,received) not in seen;seen.add((n,received))
            distances=[((received^w).bit_count(),w) for w in books[8]]
            distance=min(d for d,w in distances);nearest=[w for d,w in distances if d==distance]
            if distance<=1:
                assert len(nearest)==1
                assert (status,value,corrected)==(int(distance!=0),data(8,nearest[0]),nearest[0])
            else:assert status==2 and value==65535 and corrected==65535
        else:
            _,payload,word=fields
            assert n==16 and (n,payload) not in seen;seen.add((n,payload))
            assert word in books[16] and data(n,word)==payload
    assert len(seen)==2304
    return dict(passed=True,small_nearest_received=256,changed_codewords=2048,minimum_distance=4,
                limits='Small nearest sphere exhaustive;16-bit minimum distance uses proved linear code and exhaustive zero distances.')
if __name__=='__main__':
    result=verify(subprocess.check_output([sys.argv[1],'oracle'],text=True));print(json.dumps(result))
