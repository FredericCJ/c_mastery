"""Selected-project asset-binding controls; synthetic fixtures, not CLI executions."""
import argparse
import json
import tempfile
from pathlib import Path
from common import ROOT,dump,sha
from envelope_export_inputs import bind_project_exports


def main(output):
    controls=[]
    with tempfile.TemporaryDirectory(prefix='envelope-input-controls-') as tmp:
        root=Path(tmp);brief=root/'curriculum/projects/PR-LOCAL.md';brief.parent.mkdir(parents=True)
        source=root/'labs/local/public.txt';source.parent.mkdir(parents=True);source.write_text('public fixture')
        destination=root/'export';target=destination/'labs/local/public.txt';target.parent.mkdir(parents=True);target.write_bytes(source.read_bytes())
        meta=dict(id='PR-LOCAL',supplied_artifacts=['labs/local/public.txt'],shared_contracts=[])
        brief.write_text('```json\n'+json.dumps(meta)+'\n```\n')
        command=dict(command=['python3','-m','cmastery','init','PR-LOCAL',str(destination)],returncode=0)
        def check(rows):
            pins=set();errors=[];selected=bind_project_exports(root,pins,rows,errors)
            return pins,errors,selected
        pins,errors,selected=check([command])
        assert not errors and selected==['PR-LOCAL']
        assert {brief,source,root/'cmastery/project_assets.py'}<=pins
        assert not any('exercises.json' in str(p) for p in pins)
        controls.append(dict(name='selected-brief-assets-and-cli-pinned-without-catalog',passed=True))
        target.unlink();assert check([command])[1]
        controls.append(dict(name='missing-exported-declaration-rejected',passed=True))
        target.write_text('changed');assert check([command])[1]
        controls.append(dict(name='changed-exported-bytes-rejected',passed=True))
        target.write_bytes(source.read_bytes());assert check([])[1]
        controls.append(dict(name='no-init-record-rejected',passed=True))
        assert check([dict(command=command['command'],returncode=1)])[1]
        controls.append(dict(name='failed-init-not-accepted',passed=True))
        brief.write_text('```json\n'+json.dumps(dict(meta,id='PR-WRONG'))+'\n```\n')
        try:check([command]);rejected=False
        except ValueError:rejected=True
        assert rejected;controls.append(dict(name='wrong-project-contract-rejected',passed=True))
        brief.write_text('```json\n'+json.dumps(dict(meta,supplied_artifacts=['assessor/private.c']))+'\n```\n')
        try:check([command]);rejected=False
        except ValueError:rejected=True
        assert rejected;controls.append(dict(name='private-declaration-rejected',passed=True))
    result=dict(passed=True,controls=controls,source_sha256={p:sha(ROOT/p) for p in
        ['tools/check_envelope_export_inputs.py','tools/envelope_export_inputs.py','cmastery/project_assets.py']},
        scope='Seven synthetic selected-contract controls only; actual CLI executions belong to each package export report.')
    dump(ROOT/output,result);print(json.dumps(dict(passed=True,controls=len(controls))))


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--output',required=True);args=parser.parse_args();main(args.output)
