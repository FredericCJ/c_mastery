"""Actual T76 struct rph_sha adapter compared with Python hashlib + interval scan."""
import hashlib,json,subprocess,sys
p=subprocess.run([sys.argv[1]],text=True,capture_output=True,timeout=30);assert p.returncode==0,p.stderr
rows=p.stdout.splitlines();assert len(rows)==513
for n,line in enumerate(rows):
    key=int.from_bytes(hashlib.sha256(bytes((i^n)&255 for i in range(n))).digest()[:2],'big')
    expect=min(range(16),key=lambda i:(4096*i-key)%65536)+1
    assert list(map(int,line.split()))==[n,expect],(line,key,expect)
print(json.dumps(dict(passed=True,observations=513,provider='assessor/replicated/hashing/hashing.c',scope='Actual reference adapter, not subsequent learner integration.')))
