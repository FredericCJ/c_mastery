"""Revalidate ten exact T67 packets and fill the first four incremental-build observations."""
from datetime import datetime,timezone
import importlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
from common import ROOT,dump,read_json,sha
from source_equivalence import carried_binding
NAMES=['owning_sum','rational_units','flags_negotiation','utf8_stream','relocating_pool','mutable_views','arena_flow','partial_optics','evolution_record','packet_ring']
FOLDERS=['owning_sum','rational_units','flags_negotiation','unicode_stream','relocating_pool','mutable_views','arena_flow','partial_optics','evolution_record','packet_ring']
CONTRACTS=['OWNING_SUM.md','RATIONAL.md','NEGOTIATION.md','UTF8.md','POOL.md','VIEWS.md','FLOW.md','OPTICS.md','EVOLUTION.md','PACKET.md']
REPORT='research/v5/modern/adt_measurement_closure_validation.json'
def run():
    inputs={p:sha(ROOT/p) for p in ['tools/run_adt_measurement_closure.py','tools/common.py','tools/check_replicated_replay_depth.py','curriculum/v5/exercises/modern-cost-contract.md']};reports={};carried=[];packets=[];external={};records=[];observations=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-measurement-closure-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    for name in NAMES:
        path='research/v5/modern/adt_'+name+'_readiness.json';r=read_json(ROOT/path)
        assert len(r['criteria'])==4 and all(x['author_readiness']=='ready-bounded' for x in r['criteria'])
        for p,h in r['input_sha256'].items():
            binding=None if sha(ROOT/p)==h else carried_binding(p,h)
            assert sha(ROOT/p)==h or binding,p
            inputs[p]=sha(ROOT/p)
            if binding:carried.append(binding)
        inputs[path]=sha(ROOT/path)
        for p,h in r['current_reports'].items():
            assert sha(ROOT/p)==h;reports[p]=h
            external.update(read_json(ROOT/p).get('external_input_sha256',{}))
        packets.append(dict(name=name,readiness=path,criteria=[x['id'] for x in r['criteria']],reports=list(r['current_reports'])))
    def execute(command,label,marker=None):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for name in NAMES:
        text=execute([sys.executable,ROOT/('tools/check_adt_'+name+'.py'),'--self-test'],name+'-guard');r=json.loads(text);assert r['passed'];observations.append(dict(kind='current-guard',name=name,result=r))
    for tool in ['gcc','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-adt-build-cost-') as temp:
        for name,source in zip(NAMES[:4],FOLDERS[:4]):
            build=Path(temp)/name;execute(['cmake','-S',ROOT/'labs/adt'/source,'-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],name+'-configure');execute(['cmake','--build',build,'--parallel','4'],name+'-clean');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],name+'-unchanged','ninja: no work to do.');observations.append(dict(kind='incremental',name=name,clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True,limits='Same generated build tree, no source edits; raw wall time includes Ninja/host overhead.'))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    result=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',input_sha256=inputs,current_reports=reports,carried_forward=carried,external_input_sha256=external,source_stable=stable,external_inputs_stable=ext,packets=packets,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Only the original T67 measurement obligation and its ten inspected contracts are reconciled; all original rows survive.','Pool baseline/get samples share serialized values but not concurrent guarantees; arena cached queries require an immutable certificate; optics callback and authenticated relay series perform different work.','No missing mdspan/Haskell runtime, inaccessible published standard, physical timing, independent reviewer or learner result is manufactured.'])
    dump(ROOT/REPORT,result);print(json.dumps(dict(passed=result['passed'],commands=len(records),current_packets=len(packets))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
