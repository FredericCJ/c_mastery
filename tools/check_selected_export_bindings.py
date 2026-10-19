"""Negative controls for exact selected-contract export dependencies."""
import copy
import json
import tempfile
from pathlib import Path
from unittest.mock import patch
import selected_export_bindings as subject
from common import ROOT,sha

def run():
    cases=['positive','stale-before','wrong-selected','wrong-binding','wrong-rubric','missing-export','wrong-project','missing-init','failed-init','asset-error']
    outcomes=[]
    for case in cases:
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);brief=root/'curriculum/v5/exercises/X-T14-011.md';brief.parent.mkdir(parents=True)
            meta=dict(id='X-T14-011',project='PR-T14-09',rubric=[dict(criterion='Exact noncommuting world transform')])
            brief.write_text('```json\n'+json.dumps(meta)+'\n```\n',encoding='utf8')
            asset=root/'labs/test.h';asset.parent.mkdir(parents=True);asset.write_text('actual public bytes')
            helper=root/'tools/selected_export_bindings.py';helper.parent.mkdir(parents=True);helper.write_bytes((ROOT/'tools/selected_export_bindings.py').read_bytes())
            learner=root/'learner';learner.mkdir();exported=learner/'EXERCISE.md';exported.write_text(meta['rubric'][0]['criterion'])
            entry=dict(meta,brief_sha256=sha(brief),bound_files={'labs/test.h':sha(asset)})
            if case=='wrong-selected':entry['project']='PR-T00-00'
            if case=='wrong-binding':entry['bound_files']['labs/test.h']='0'*64
            catalog=root/'curriculum/v5/exercises.json';catalog.write_text(json.dumps(dict(exercises=[entry,dict(id='unrelated',changed=True)])))
            before={'curriculum/v5/exercises.json':sha(catalog),'labs/test.h':sha(asset)}
            records=[dict(command=['python','-m','cmastery','init','PR-T14-09',str(learner),'--exercise','X-T14-011'],exit_code=0)]
            if case=='stale-before':before['curriculum/v5/exercises.json']='0'*64
            if case=='wrong-rubric':exported.write_text('Different claim')
            if case=='missing-export':exported.unlink()
            if case=='wrong-project':records[0]['command'][4]='PR-T00-00'
            if case=='missing-init':records=[]
            if case=='failed-init':records[0]['exit_code']=1
            def assets(_root,pins,commands,errors):
                if case=='asset-error':errors.append('actual selected asset missing')
                if not any(r['returncode']==0 for r in commands):errors.append('no successful canonical project export')
            rejected=False
            with patch.object(subject,'bind_project_exports',assets):
                try:
                    selected=subject.bind_completed_exports(root,before,records)
                    assert selected==['X-T14-011'] and 'curriculum/v5/exercises.json' not in before
                    assert {'curriculum/v5/exercises/X-T14-011.md','labs/test.h','tools/selected_export_bindings.py'}<=set(before)
                except (AssertionError,FileNotFoundError,KeyError):rejected=True
            assert rejected==(case!='positive'),case
            outcomes.append(dict(case=case,passed=True,rejected=rejected))
    return outcomes
if __name__=='__main__':print(json.dumps(dict(passed=True,controls=run())))
