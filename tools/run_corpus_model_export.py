"""Exercise canonical CLI exports with unsolved and explicitly supplied providers."""
import json
import pathlib
import subprocess
import sys
import tempfile
from common import ROOT, sha, dump
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief, catalog

REPORT = 'reviews/v5/expansion/corpus_model/export_validation.json'


def main():
    identifier = 'X-T15-021'
    selected = {identifier: catalog()['exercises'][identifier]}
    files = [p for p in (ROOT/'labs/corpus_model').rglob('*')
             if p.is_file() and '__pycache__' not in p.parts]
    files += [ROOT/n for n in [
        'tools/run_corpus_model_export.py', 'cmastery/cli.py',
        'cmastery/project_assets.py', 'cmastery/exercises.py', 'cmastery/learning.py',
        'fixtures/corpus_model/cases.json',
        'research/v5/expansion/corpus_model/resources.json',
        'curriculum/v5/exercises/X-T15-021.md', 'assessor/corpus_model/model.c',
        'assessor/parsing_depth/pratt.c', 'labs/parsing_depth/parsing.h']]
    before = {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))}
    records = []

    def run(args, name, negative=None):
        result = subprocess.run(list(map(str,args)),cwd=ROOT,text=True,
                                capture_output=True,timeout=180)
        out = result.stdout+'\n'+result.stderr
        ok = (result.returncode != 0 and negative in out) if negative else result.returncode == 0
        records.append(dict(id=name,command=list(map(str,args)),exit_code=result.returncode,
            expected='semantic rejection' if negative else 0, required_marker=negative,
            stdout=out,passed=ok))
        assert ok,(name,out)

    with tempfile.TemporaryDirectory(prefix='corpus-model-export-') as temporary:
        tmp = pathlib.Path(temporary)
        meta,_ = parse_brief(ROOT/f'curriculum/v5/exercises/{identifier}.md')
        workspace = tmp/identifier
        run([sys.executable,'-m','cmastery','init',meta['project'],workspace,
             '--exercise',identifier],identifier+'.init')
        assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
        copied = {}
        for path in (ROOT/'labs/corpus_model').rglob('*'):
            if path.is_file() and '__pycache__' not in path.parts:
                relative = path.relative_to(ROOT/'labs/corpus_model')
                assert sha(workspace/relative) == sha(path)
                copied[relative.as_posix()] = sha(path)
        body = ''.join((workspace/n).read_text() for n in ('EXERCISE.md','ACTIVE.md') if (workspace/n).is_file())
        assert all(row['criterion'] in body for row in meta['rubric'])
        for mode,provider in [('learner',None),('explicit-family',ROOT/'assessor/corpus_model'),
                              ('explicit-parent',ROOT/'assessor')]:
            build = tmp/(identifier+'-'+mode)
            args = ['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
            if provider:
                args += ['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
            run(args,identifier+'.'+mode+'.configure')
            run(['cmake','--build',build,'--parallel','4'],identifier+'.'+mode+'.build')
            run(['ctest','--test-dir',build,'-R','^corpus_model.contract$',
                 '--output-on-failure'],identifier+'.'+mode+'.ctest','CM_CHECK:' if not provider else None)
        run(['cmake','-S',workspace,'-B',tmp/'missing',
             '-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],identifier+'.missing-provider',
             'Supply explicit completed model and accepted Pratt providers')
        missing_parser=tmp/'missing-parser'
        run(['cmake','-S',workspace,'-B',missing_parser,'-G','Ninja',
             '-DCM_PROVIDER='+str(ROOT/'assessor/corpus_model/model.c')],identifier+'.parser-stub.configure')
        run(['cmake','--build',missing_parser,'--parallel','4'],identifier+'.parser-stub.build')
        run(['ctest','--test-dir',missing_parser,'-R','^corpus_model.contract$',
             '--output-on-failure'],identifier+'.parser-stub.ctest','CM_CHECK:')
    assert all(sha(ROOT/p)==h for p,h in before.items())
    assert {identifier:catalog()['exercises'][identifier]} == selected
    dump(ROOT/REPORT,dict(schema_version=1,passed=True,status='passed',source_stable=True,
        input_sha256=before,catalog_selected=selected,
        exports=[dict(exercise=identifier,exported_artifacts=copied,unsolved_sources_unchanged=True,
                      private_sources_copied=False,exact_rubric_present=True)],
        executions=records,physical_validation='not-run',independent_review='not-run',
        learner_mastery='not-assessed',
        limits='Actual canonical CLI export and selected anchor; only the exact catalog row is bound. No learner, review or physical observation claim.'))
    print(json.dumps(dict(passed=True,commands=len(records),exercises=1)))


if __name__ == '__main__':
    main()
