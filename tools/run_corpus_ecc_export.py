"""Actual canonical exercise export; unsolved source, explicit providers and no fallback."""
import datetime
import json
import pathlib
import subprocess
import sys
import tempfile
from common import ROOT,sha
from envelope_export_inputs import bind_project_exports
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
REPORT=ROOT/'reviews/v5/expansion/corpus_ecc/export_validation.json'
def main():
    brief='curriculum/v5/exercises/X-T15-008.md';meta,_=parse_brief(ROOT/brief)
    files=[p for p in (ROOT/'labs/corpus_ecc').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [ROOT/p for p in meta['fixtures']+meta['shared_contracts']+[brief,'cmastery/cli.py','cmastery/exercises.py','cmastery/project_assets.py','tools/envelope_export_inputs.py','assessor/corpus_ecc/ecc.c','tools/run_corpus_ecc_export.py']]
    catalog=json.loads((ROOT/'curriculum/v5/exercises.json').read_text(encoding='utf8'))
    entry=next(r for r in catalog['exercises'] if r['id']==meta['id'])
    assert entry['brief_sha256']==sha(ROOT/brief) and all(entry[k]==v for k,v in meta.items())
    for p,h in entry['bound_files'].items():
        assert sha(ROOT/p)==h
        files.append(ROOT/p)
    pins=set(files)
    before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)};records=[];exported={};errors=[]
    def run(args,name,negative=None):
        r=subprocess.run([str(x) for x in args],cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+'\n'+r.stderr
        ok=(r.returncode!=0 and negative in out) if negative else r.returncode==0
        records.append(dict(id=name,command=[str(x) for x in args],exit_code=r.returncode,returncode=r.returncode,expected='semantic rejection' if negative else 0,required_marker=negative,stdout=out,passed=ok))
        assert ok,(name,out)
    with tempfile.TemporaryDirectory(prefix='corpus-ecc-export-') as temporary:
        tmp=pathlib.Path(temporary);workspace=tmp/'learner'
        run([sys.executable,'-m','cmastery','init','PR-T15-10',workspace,'--exercise','X-T15-008'],'init')
        assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
        for p in (ROOT/'labs/corpus_ecc').rglob('*'):
            if p.is_file() and '__pycache__' not in p.parts:
                rel=p.relative_to(ROOT/'labs/corpus_ecc');assert sha(workspace/rel)==sha(p);exported[rel.as_posix()]=sha(p)
        text=(workspace/'EXERCISE.md').read_text(encoding='utf8')
        assert all(row['criterion'] in text for row in meta['rubric'])
        for mode,provider in [('learner',None),('explicit-family',ROOT/'assessor/corpus_ecc'),('explicit-parent',ROOT/'assessor')]:
            build=tmp/mode;args=['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
            if provider:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
            run(args,mode+'.configure');run(['cmake','--build',build,'--parallel','4'],mode+'.build')
            run(['ctest','--test-dir',build,'--output-on-failure'],mode+'.ctest','ECC_CHECK' if not provider else None)
        run(['cmake','-S',workspace,'-B',tmp/'missing','-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],'missing-provider','Supply the explicit completed corpus_ecc implementation')
        bind_project_exports(ROOT,pins,records,errors)
        assert not errors,errors
    assert all(sha(ROOT/p)==h for p,h in before.items())
    d=dict(schema_version=1,passed=True,status='passed',source_stable=True,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},selected_contracts=[dict(id=meta['id'],canonical_brief=brief,brief_sha256=sha(ROOT/brief),observed_selected_catalog_entry=entry)],exported_artifacts=exported,executions=records,exercise='X-T15-008',private_sources_copied=False,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['The actual derived selected row is checked against its canonical brief and bound files. Unrelated catalog rows are not an input dependency.'])
    REPORT.parent.mkdir(parents=True,exist_ok=True);REPORT.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(records),exported=len(exported))))
if __name__=='__main__':main()
