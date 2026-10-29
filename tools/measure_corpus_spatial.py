"""Equal-contract geometric work comparisons, with raw actual outputs retained."""
import datetime
import json
import pathlib
import subprocess
from common import ROOT,sha

def main():
    sources=['labs/corpus_spatial/spatial.h','labs/corpus_spatial/tests.c','assessor/corpus_spatial/spatial.c','tools/measure_corpus_spatial.py']
    pins={p:sha(ROOT/p) for p in sources};build=ROOT/'build/spatial-measure'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S');build.mkdir(parents=True)
    records=[];observations=[]
    def run(args):
        args=[str(x) for x in args];r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=120)
        records.append(dict(command=args,exit_code=r.returncode,stdout=r.stdout+r.stderr,passed=r.returncode==0));assert r.returncode==0,r.stderr
        return r.stdout
    for cc in ['gcc','clang']:
        exe=build/cc;run([cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I',ROOT/'labs/corpus_spatial',ROOT/'assessor/corpus_spatial/spatial.c',ROOT/'labs/corpus_spatial/tests.c','-o',exe])
        datasets={}
        for kind in range(1,6):
            raw=run([exe,str(kind),'dump']);states=[]
            for line in raw.splitlines():
                row=line.split()
                if row[0]=='S':states.append(dict(dim=int(row[2]),items=[],queries=[]))
                elif row[0]=='I':states[-1]['items'].append(list(map(int,row[1:])))
                elif row[0]=='Q':states[-1]['queries'].append(list(map(int,row[1:])))
                elif row[0]=='M':observations.append(dict(compiler=cc,experiment='fixed32-object moved BVH',refit_visits=int(row[2]),rebuild_visits=int(row[4]),identical_queries=int(row[6])))
            datasets[kind]=states
        for a,b in [(1,3),(2,5)]:
            for index,(left,right) in enumerate(zip(datasets[a],datasets[b])):
                assert left['dim']==right['dim'] and left['items']==right['items']
                assert [r[:7] for r in left['queries']]==[r[:7] for r in right['queries']]
                observations.append(dict(compiler=cc,dimension=left['dim'],state=index,kinds=[a,b],query_count=len(left['queries']),identical_masks=True,
                    visits=[sum(r[7] for r in d['queries']) for d in [left,right]],exact_item_tests=[sum(r[8] for r in d['queries']) for d in [left,right]]))
        probe=build/(cc+'-size.c');probe.write_text('#include "spatial.h"\n#include <stdio.h>\nint main(void){printf("%zu %zu %zu\\n",sizeof(struct sp_index),sizeof(struct sp_node),sizeof(struct sp_ref));}\n',encoding='utf8')
        out=build/(cc+'-size');run([cc,'-std=c17','-I',ROOT/'labs/corpus_spatial',probe,'-o',out]);sizes=list(map(int,run([out]).split()))
        observations.append(dict(compiler=cc,experiment='actual hosted ABI sizeof',index=sizes[0],node=sizes[1],reference=sizes[2],two_caller_indices=sizes[0]*2,probe_sha256=sha(probe)))
    assert all(sha(ROOT/p)==h for p,h in pins.items())
    report=ROOT/'reviews/v5/expansion/corpus_spatial/cost_validation.json';report.write_text(json.dumps(dict(passed=True,source_stable=True,input_sha256=pins,executions=records,observations=observations,limits=['Exact visits/item tests for identical admitted geometry and query contracts, not universal latency or speed bounds.','Grid occupied-reference visits and hierarchy node visits have different per-visit costs.','Fixed backing plus distinct scratch are both charged; target per-function frames are in the main audit.','No physical cache, memory traffic or whole-recursion stack measurement.']),indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,commands=len(records),observations=len(observations))))
if __name__=='__main__':main()
