"""Retain raw valid-only timing samples and bounded descriptive comparisons."""
import csv,io,json,pathlib,statistics,subprocess,sys
p=subprocess.run([sys.argv[1]],capture_output=True,text=True,timeout=90)
assert p.returncode==0,p.stdout+p.stderr
rows=[list(map(int,r)) for r in csv.reader(io.StringIO(p.stdout))];assert len(rows)==240
groups={}
for variant,size,pattern,offset,sample,repetitions,elapsed in rows:
    assert variant in [0,1] and size in [64,4096] and pattern in [0,1] and offset in [0,1] and 0<=sample<15 and elapsed>=0
    assert repetitions==(1000 if size==4096 else 10000)
    key=(variant,size,pattern,offset);groups.setdefault(key,[]).append((sample,elapsed/(size*repetitions)))
assert len(groups)==16 and all(sorted(i for i,_ in samples)==list(range(15)) for samples in groups.values())
report=dict(status='pass',command=[sys.argv[1]],returncode=p.returncode,raw_stdout=p.stdout,raw_stderr=p.stderr,samples=len(rows),groups=[dict(variant=k[0],bytes=k[1],pattern=k[2],offset=k[3],median_ns_per_byte=statistics.median(v for _,v in samples),minimum_ns_per_byte=min(v for _,v in samples),maximum_ns_per_byte=max(v for _,v in samples)) for k,samples in sorted(groups.items())],limits=['Same disjoint byte arrays, XOR mapping and admitted lengths for both loops; explicit offset0/1 and zero/pseudorandom distributions.','Host clock measurements include scheduling/noise/cache effects; no universal speedup or latency bound.','No invalid restrict call is timed; code size and emitted calls/loops are inspected separately.'])
pathlib.Path(sys.argv[2]).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(status='pass',samples=len(rows),groups=len(groups))))
