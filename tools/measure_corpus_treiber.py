"""Observe actual typed stack storage with ordinary and exhausted generation limits."""
import json
from pathlib import Path
import subprocess
import tempfile
from common import ROOT,sha
def main():
    paths=['tools/measure_corpus_treiber.py','labs/corpus_treiber/treiber.h','labs/corpus_treiber/CONTRACTS.md','labs/corpus_treiber/tests.c','assessor/corpus_treiber/treiber.c']
    pins={p:sha(ROOT/p) for p in paths};rows=[];observations=[]
    with tempfile.TemporaryDirectory(prefix='treiber-cost-') as name:
        for capacity in [1073741823,3]:
            exe=Path(name)/str(capacity)
            for argv in [['gcc','-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-DTS_GEN_MAX='+str(capacity),'-Ilabs/corpus_treiber','assessor/corpus_treiber/treiber.c','labs/corpus_treiber/tests.c','-pthread','-o',str(exe)],[str(exe),'reclaim']]:
                r=subprocess.run(argv,cwd=ROOT,text=True,capture_output=True,timeout=90)
                rows.append(dict(command=argv,exit_code=r.returncode,stdout=r.stdout+r.stderr,passed=r.returncode==0));assert r.returncode==0,r.stdout+r.stderr
            line=next(x for x in r.stdout.splitlines() if x.startswith('TS_PASS '))
            fields={k:int(v) for k,v in (x.split('=') for x in line.split()[1:])};assert fields['gen']==capacity
            observations.append(fields)
    assert all(sha(ROOT/p)==h for p,h in pins.items())
    d=dict(passed=True,source_stable=True,input_sha256=pins,executions=rows,observations=observations,model={'head_work':'O(1) uncontended; retries depend on interference','core_storage':'O(N+P), zero allocation','claim_collect':'O(N) claim scan, O(NP) collection','comparison':'Same LIFO contract; generation3 refuses reuse after four lifetimes instead of wrapping.'},limits=['Actual native ABI sizes; target emitted frames/sections are separately recorded by the main campaign.','No physical timing, arbitrary-history proof, weakest-order claim or complete-domain speed claim.'])
    out=ROOT/'reviews/v5/expansion/corpus_treiber/cost_validation.json';out.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(rows),observations=observations)))
if __name__=='__main__':main()
