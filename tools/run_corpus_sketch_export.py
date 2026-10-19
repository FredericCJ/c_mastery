"""Export canonical sketch exercises and exercise actual isolated learner workspaces."""
import json,pathlib,subprocess,sys,tempfile
from common import ROOT,sha,dump
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,catalog
REPORT='reviews/v5/expansion/corpus_sketch/export_validation.json'
def main():
    ids=[f'X-T15-{n:03}' for n in (5,6,7)];selected={k:catalog()['exercises'][k] for k in ids}
    files=[p for p in (ROOT/'labs/corpus_sketch').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [ROOT/n for n in ['tools/run_corpus_sketch_export.py','cmastery/cli.py','cmastery/project_assets.py','cmastery/exercises.py','cmastery/learning.py','labs/corpus_sketch/CONTRACTS.md','fixtures/corpus_sketch/cases.json','research/v5/expansion/corpus_sketch/resources.json']]
    files += [ROOT/f'curriculum/v5/exercises/{k}.md' for k in ids]
    files += [ROOT/'assessor/corpus_sketch'/name for name in ['count_min.c','hyperloglog.c','minhash.c']]
    before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))};records=[];exports=[]
    def run(args,name,negative=None):
        r=subprocess.run(list(map(str,args)),cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+'\n'+r.stderr
        ok=(r.returncode!=0 and negative in out) if negative else r.returncode==0
        records.append(dict(id=name,command=list(map(str,args)),exit_code=r.returncode,expected='semantic rejection' if negative else 0,required_marker=negative,stdout=out,passed=ok));assert ok,(name,out)
    with tempfile.TemporaryDirectory(prefix='corpus-sketch-export-') as tmp:
        tmp=pathlib.Path(tmp)
        for identifier in ids:
            meta,_=parse_brief(ROOT/f'curriculum/v5/exercises/{identifier}.md');workspace=tmp/identifier
            run([sys.executable,'-m','cmastery','init',meta['project'],workspace,'--exercise',identifier],identifier+'.init')
            assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
            copied={}
            for p in (ROOT/'labs/corpus_sketch').rglob('*'):
                if p.is_file() and '__pycache__' not in p.parts:
                    rel=p.relative_to(ROOT/'labs/corpus_sketch');assert sha(workspace/rel)==sha(p);copied[rel.as_posix()]=sha(p)
            body=(workspace/'EXERCISE.md').read_text();assert all(r['criterion'] in body for r in meta['rubric'])
            for mode,provider in [('learner',None),('explicit-family',ROOT/'assessor/corpus_sketch'),('explicit-parent',ROOT/'assessor')]:
                build=tmp/(identifier+'-'+mode);args=['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
                if provider:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
                run(args,identifier+'.'+mode+'.configure');run(['cmake','--build',build,'--parallel','4'],identifier+'.'+mode+'.build')
                run(['ctest','--test-dir',build,'-R','^'+meta['lab']['tests'][0]+'$','--output-on-failure'],identifier+'.'+mode+'.ctest','SK_CHECK:' if not provider else None)
            run(['cmake','-S',workspace,'-B',tmp/(identifier+'-missing'),'-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],identifier+'.missing-provider','Cannot find source file')
            exports.append(dict(exercise=identifier,exported_artifacts=copied,unsolved_sources_unchanged=True,private_sources_copied=False,exact_rubric_present=True))
    assert all(sha(ROOT/p)==h for p,h in before.items())
    assert {k:catalog()['exercises'][k] for k in ids}==selected
    dump(ROOT/REPORT,dict(schema_version=1,passed=True,status='passed',source_stable=True,input_sha256=before,catalog_selected=selected,exports=exports,executions=records,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits='Actual canonical CLI init and selected anchors. The three exact catalog rows are bound, avoiding unrelated whole-catalog metadata churn. No learner assessment or physical observation.'))
    print(json.dumps(dict(passed=True,commands=len(records),exercises=len(exports))))
if __name__=='__main__':main()
