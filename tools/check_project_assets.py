"""Exercise real project asset exports and adversarial containment contracts."""
import json
from pathlib import Path
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery import cli,project_assets
from common import ROOT,dump,sha

def main():
    checks=[]
    def require(ok,label):
        if not ok:raise AssertionError(label)
        checks.append(dict(id=label,passed=True))
    def rejected(action,label):
        try:action()
        except (ValueError,OSError):require(True,label)
        else:raise AssertionError('Accepted '+label)
    with tempfile.TemporaryDirectory(prefix='cmastery-project-assets-') as temporary:
        tmp=Path(temporary);root=tmp/'source';destination=tmp/'destination';destination.mkdir()
        for folder in ['curriculum/projects','labs/example','assessor/example','fixtures']:(root/folder).mkdir(parents=True)
        (root/'labs/example/contract.md').write_text('PUBLIC CONTRACT\n')
        (root/'assessor/example/answer.c').write_text('WITHHELD\n')
        (root/'fixtures/example.json').write_text('{}\n')
        brief=root/'curriculum/projects/PR-TEST.md'
        def write(names,shared=None):
            brief.write_text('```json\n'+json.dumps(dict(id='PR-TEST',shared_contracts=shared or [],supplied_artifacts=names))+'\n```\n')
        write(['labs/example/contract.md','fixtures/example.json'],['labs/example/contract.md'])
        rows=project_assets.prepare(root,'PR-TEST');require(len(rows)==2,'deduplicate-identical-public-input')
        project_assets.export(root,destination,rows)
        require((destination/'labs/example/contract.md').read_text()=='PUBLIC CONTRACT\n','exact-public-copy')
        require(not any('assessor' in p.parts for p in destination.rglob('*')),'private-exclusion')
        project_assets.export(root,destination,rows);require(True,'identical-copy-fixed-point')
        for bad in ['../assessor/example/answer.c','assessor/example/answer.c','labs/../assessor/example/answer.c','labs/Assessor/answer.c','/absolute','C:/absolute','labs\\example\\contract.md','labs/example/contract.md:stream','labs//example/contract.md','labs/example/missing']:
            write([bad]);rejected(lambda:project_assets.prepare(root,'PR-TEST'),'reject-'+bad)
        write(['labs/example/contract.md','fixtures/example.json'])
        rows=project_assets.prepare(root,'PR-TEST')
        empty=tmp/'atomic';empty.mkdir();(root/'fixtures/example.json').write_text('changed\n')
        rejected(lambda:project_assets.export(root,empty,rows),'changed-source-before-copy')
        require(not list(empty.iterdir()),'no-partial-asset-copy')
        write(['labs/example/contract.md']);rows=project_assets.prepare(root,'PR-TEST')
        (destination/'labs/example/contract.md').write_text('learner content\n')
        rejected(lambda:project_assets.export(root,destination,rows),'no-overwrite-different-learner-content')
        if sys.platform!='win32':
            (root/'labs/example/link.c').symlink_to(root/'assessor/example/answer.c');write(['labs/example/link.c'])
            rejected(lambda:project_assets.prepare(root,'PR-TEST'),'source-symlink-private')
            write(['labs/example/contract.md']);rows=project_assets.prepare(root,'PR-TEST');trap=tmp/'trap';trap.mkdir();outside=tmp/'outside';outside.mkdir();(trap/'labs').symlink_to(outside,target_is_directory=True)
            rejected(lambda:project_assets.export(root,trap,rows),'destination-symlink-escape')
        for project in ['PR-T59-07','PR-T58-01']:
            actual=tmp/project;cli.instantiate(project,actual)
            expected=project_assets.prepare(ROOT,project);require(bool(expected),'actual-project-contract-'+project)
            require(all(sha(actual/r['path'])==r['sha256'] for r in expected),'actual-cli-export-'+project)
            require(not any('assessor' in p.relative_to(actual).parts for p in actual.rglob('*')),'actual-no-private-'+project)
    report=dict(passed=True,checks=checks,input_sha256={p:sha(ROOT/p) for p in ['cmastery/cli.py','cmastery/project_assets.py','tools/check_project_assets.py']},scope='Actual project initialization plus preflight, no-private-copy and collision guards; no learner assessment or release acceptance.')
    dump(ROOT/'reviews/v5/expansion/project_assets/validation.json',report);print(json.dumps(dict(passed=True,checks=len(checks))))
if __name__=='__main__':main()
