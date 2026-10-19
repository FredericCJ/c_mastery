"""Actual canonical exercise export; unsolved source, explicit providers and no fallback."""
import datetime
import json
import pathlib
import subprocess
import sys
import tempfile
from common import ROOT,sha
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
REPORT=ROOT/'reviews/v5/expansion/corpus_ecc/export_validation.json'
def main():
    brief='curriculum/v5/exercises/X-T15-008.md';meta,_=parse_brief(ROOT/brief)
    files=[p for p in (ROOT/'labs/corpus_ecc').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [ROOT/p for p in meta['fixtures']+meta['shared_contracts']+[brief,'curriculum/v5/exercises.json','cmastery/cli.py','cmastery/exercises.py','assessor/corpus_ecc/ecc.c','tools/run_corpus_ecc_export.py']]
    before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))};records=[];exported={}
    def run(args,name,negative=None):
        r=subprocess.run([str(x) for x in args],cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+'\n'+r.stderr
        ok=(r.returncode!=0 and negative in out) if negative else r.returncode==0
        records.append(dict(id=name,command=[str(x) for x in args],exit_code=r.returncode,expected='semantic rejection' if negative else 0,required_marker=negative,stdout=out,passed=ok))
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
    assert all(sha(ROOT/p)==h for p,h in before.items())
    d=dict(schema_version=1,passed=True,status='passed',source_stable=True,input_sha256=before,exported_artifacts=exported,executions=records,exercise='X-T15-008',private_sources_copied=False,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed')
    REPORT.parent.mkdir(parents=True,exist_ok=True);REPORT.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=True,commands=len(records),exported=len(exported))))
if __name__=='__main__':main()
