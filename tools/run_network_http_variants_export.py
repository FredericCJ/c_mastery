"""Five actual isolated exports; never discover/copy assessor implementations."""
from datetime import datetime,timezone
from pathlib import Path
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha
from register_network_http_variants import IDS,BASE
sys.path.insert(0,str(ROOT/'labs/network_followthrough/ip'))
from checksum_slice import emit
sys.path.insert(0,str(ROOT))
from cmastery import exercises,learning

def main():
    selected={ident:learning.digest(exercises.exercise(ident)) for ident in IDS}
    files=['tools/run_network_http_variants_export.py','tools/register_network_http_variants.py','cmastery/assessments.py','cmastery/cli.py','cmastery/exercises.py','assessor/network_http_variants/variant.c','assessor/network_http_variants/owned.c','assessor/b045_systems/systems.c','assessor/http_depth/http_depth.c','assessor/hosted_followthrough/http/flow.c','labs/network_followthrough/ip/checksum_slice.py'];specs={}
    for id in IDS:
        brief=f'curriculum/v5/exercises/{id}.md';d=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);specs[id]=d;files += [brief]+d['fixtures']+d['shared_contracts']
    files += [p.relative_to(ROOT).as_posix() for p in (ROOT/'cmastery').glob('*.py')]
    before={p:sha(ROOT/p) for p in files};home=ROOT/'reviews/v5/expansion/network_http_variants'/('export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);rows=[];executions=[]
    checksum=home/'checksum.c';composition=emit(ROOT/'assessor/b045_systems/systems.c',checksum)
    def run(cmd,label,code=0,marker=None):
        cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=400);text=r.stdout+'\n'+r.stderr;log=home/(label+'.log');log.write_text(text,encoding='utf8');executions.append(dict(command=cmd,exit_code=r.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (r.returncode==code if code is not None else r.returncode!=0) and (not marker or marker in text),text
    with tempfile.TemporaryDirectory(prefix='network-http-variant-export-') as tmp:
        tmp=Path(tmp)
        for id in IDS:
            begin=len(executions);d=specs[id];workspace=tmp/id;run([sys.executable,'-m','cmastery','init',d['project'],workspace,'--exercise',id],id+'-init');public=workspace/'labs/network_http_variants';assets={}
            for p in d['fixtures']+d['shared_contracts']:assert sha(workspace/p)==before[p];assets[p]=before[p]
            assert not (workspace/'assessor').exists()
            for kind in ['learner','reference']:
                build=tmp/(id+'-'+kind);cmd=['cmake','-S',public,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
                if kind=='reference':cmd+=['-DNV_SOURCE='+str(ROOT/'assessor/network_http_variants/variant.c'),'-DNV_OWNED_SOURCE='+str(ROOT/'assessor/network_http_variants/owned.c'),'-DNV_CHECKSUM_SOURCE='+str(checksum),'-DNV_DEPTH_SOURCE='+str(ROOT/'assessor/http_depth/http_depth.c'),'-DNV_FLOW_SOURCE='+str(ROOT/'assessor/hosted_followthrough/http/flow.c')]
                run(cmd,id+'-'+kind+'-configure');run(['cmake','--build',build,'--parallel','4'],id+'-'+kind+'-build');run(['ctest','--test-dir',build,'--output-on-failure'],id+'-'+kind+'-test',0 if kind=='reference' else None,'100% tests passed' if kind=='reference' else 'NV_CHECK:')
            run(['cmake','-S',public,'-B',tmp/(id+'-missing'),'-G','Ninja','-DNV_CHECKSUM_SOURCE='+str(tmp/'absent.c')],id+'-missing',None,'Missing explicit provider')
            rows.append(dict(id=id,assets=assets,private_sources_copied=False,executions=list(range(begin,len(executions)))))
    assert all(sha(ROOT/p)==h for p,h in before.items())
    assert all(learning.digest(exercises.exercise(i))==h for i,h in selected.items())
    dump(ROOT/BASE/'export_validation.json',dict(selected_catalog_sha256=selected,passed=True,source_stable=True,input_sha256=before,executions=executions,exercises=rows,private_sources_copied=False,checksum_composition=composition,checksum_sha256=sha(checksum),physical='not-run',independent_assessment='not-run'))
    print(dict(passed=True,commands=len(executions),exercises=len(rows)))
if __name__=='__main__':main()
