"""Actual selected canonical exercise exports with public assets and explicit providers."""
import json
import pathlib
import subprocess
import sys
import tempfile
from common import ROOT,sha
from envelope_export_inputs import bind_project_exports
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
REPORT=ROOT/'reviews/v5/expansion/corpus_spatial/export_validation.json'

def main():
    catalog=json.loads((ROOT/'curriculum/v5/exercises.json').read_text(encoding='utf8'))
    pins={p for p in (ROOT/'labs/corpus_spatial').rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    pins|={ROOT/p for p in ['tools/run_corpus_spatial_export.py','assessor/corpus_spatial/spatial.c','cmastery/cli.py','cmastery/exercises.py','cmastery/project_assets.py','tools/envelope_export_inputs.py']}
    selected=[];records=[];exports=[];errors=[]
    def run(args,name,negative=None):
        args=[str(x) for x in args];r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+'\n'+r.stderr
        ok=(r.returncode!=0 and negative in out and 'runtime error:' not in out and 'AddressSanitizer' not in out) if negative else r.returncode==0
        records.append(dict(id=name,command=args,exit_code=r.returncode,returncode=r.returncode,expected='semantic rejection' if negative else 0,required_marker=negative,stdout=out,passed=ok));assert ok,(name,out)
    before={p.relative_to(ROOT).as_posix():sha(p) for p in pins}
    with tempfile.TemporaryDirectory(prefix='spatial-export-') as temporary:
        tmp=pathlib.Path(temporary)
        for number in range(6,11):
            ident=f'X-T14-{number:03}';brief=ROOT/'curriculum/v5/exercises'/f'{ident}.md';meta,_=parse_brief(brief)
            entry=next(r for r in catalog['exercises'] if r['id']==ident)
            assert entry['brief_sha256']==sha(brief) and all(entry[k]==v for k,v in meta.items())
            for p,h in entry['bound_files'].items():assert sha(ROOT/p)==h;pins.add(ROOT/p)
            pins.add(brief);selected.append(dict(id=ident,canonical_brief=brief.relative_to(ROOT).as_posix(),brief_sha256=sha(brief),observed_selected_catalog_entry=entry))
            workspace=tmp/ident;run([sys.executable,'-m','cmastery','init','PR-T14-09',workspace,'--exercise',ident],ident+'.init')
            assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
            files={}
            for p in (ROOT/'labs/corpus_spatial').rglob('*'):
                if p.is_file() and '__pycache__' not in p.parts:
                    rel=p.relative_to(ROOT/'labs/corpus_spatial');assert sha(workspace/rel)==sha(p);files[rel.as_posix()]=sha(p)
            text=''.join((workspace/n).read_text(encoding='utf8') for n in ('EXERCISE.md','ACTIVE.md') if (workspace/n).is_file());assert all(r['criterion'] in text for r in meta['rubric'])
            for mode,provider in [('learner',None),('family',ROOT/'assessor/corpus_spatial'),('parent',ROOT/'assessor')]:
                build=tmp/(ident+mode);args=['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
                if provider:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
                run(args,ident+'.'+mode+'.configure');run(['cmake','--build',build,'--parallel','4'],ident+'.'+mode+'.build')
                run(['ctest','--test-dir',build,'--output-on-failure','-R',f'^corpus_spatial.{number-5}$'],ident+'.'+mode+'.ctest','SP_CHECK' if not provider else None)
            run(['cmake','-S',workspace,'-B',tmp/(ident+'missing'),'-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],ident+'.missing-provider','Supply the explicit completed corpus_spatial implementation')
            exports.append(dict(exercise=ident,files=files))
        bind_project_exports(ROOT,pins,records,errors)
        assert not errors,errors
    assert all(sha(ROOT/p)==h for p,h in before.items())
    data=dict(passed=True,source_stable=True,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},selected_contracts=selected,executions=records,exports=exports,private_sources_copied=False,
        limits=['Only selected canonical exercise and project asset contracts are source dependencies; the full derived catalog was read and selected rows verified, but unrelated rows are not consumer inputs.','Actual public init plus compiling stubs and explicit parent/family reference builds; no independent learner or target execution.'])
    REPORT.parent.mkdir(parents=True,exist_ok=True);REPORT.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(records),exports=len(exports))))
if __name__=='__main__':main()
