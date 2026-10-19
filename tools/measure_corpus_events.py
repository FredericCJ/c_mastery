"""Observe typed storage under the two equal-content event contracts."""
import json
from pathlib import Path
import subprocess
import tempfile
from common import ROOT,sha
def main():
    paths=['tools/measure_corpus_events.py','labs/corpus_events/events.h','labs/corpus_events/CONTRACTS.md','labs/corpus_events/tests.c','assessor/corpus_events/events.c']
    pins={p:sha(ROOT/p) for p in paths};rows=[];observations=[]
    with tempfile.TemporaryDirectory(prefix='events-cost-') as name:
        for capacity in [16,1]:
            exe=Path(name)/str(capacity)
            for argv in [['gcc','-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-DEV_TEXT='+str(capacity),'-Ilabs/corpus_events','assessor/corpus_events/events.c','labs/corpus_events/tests.c','-o',str(exe)],[str(exe),'parser']]:
                r=subprocess.run(argv,cwd=ROOT,text=True,capture_output=True,timeout=90)
                rows.append(dict(command=argv,exit_code=r.returncode,stdout=r.stdout+r.stderr,passed=r.returncode==0));assert r.returncode==0,r.stdout+r.stderr
            line=next(x for x in r.stdout.splitlines() if x.startswith('EV_RESOURCE:'))
            fields={k:int(v) for k,v in (x.split('=') for x in line.split(':')[1:])};assert fields['text']==capacity
            observations.append(fields)
    assert all(sha(ROOT/p)==h for p,h in pins.items())
    d=dict(passed=True,source_stable=True,input_sha256=pins,executions=rows,observations=observations,model={'input_work':'O(N+K*name_limit), plus each explicitly repeated callback attempt','parser_storage':'O(depth_limit*name_limit+text_capacity), independent of already consumed document length','consumer_storage':'O(depth_limit), two copied queue events are additional','comparison':'Same logical language and semantic answers, differing TEXT event segmentation and storage.'},limits=['Actual native ABI sizes; target emitted frames/sections are separately recorded by the main campaign.','No physical timing, arbitrary concurrent callback safety, full XML conformity or complete-domain speed claim.'])
    out=ROOT/'reviews/v5/expansion/corpus_events/cost_validation.json';out.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(rows),observations=observations)))
if __name__=='__main__':main()
