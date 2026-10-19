"""Reject missing profiles, exact controls, source changes and target overclaims."""
import argparse
import copy
import json
import pathlib
from common import ROOT,sha
from check_expansion_campaigns import pin_errors,execution_errors,generic_errors
DEFAULT='reviews/v5/expansion/corpus_scene/current_validation.json'
def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_scene/family.json').read_text(encoding='utf8'))
    errors=generic_errors('corpus_scene',meta,d)
    def need(ok,why):
        if not ok:errors.append(why)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'successful stable campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external evidence limits')
    if files:errors+=pin_errors(d)+execution_errors(d)
    rows=d.get('executions',[])
    if files:
        outputs=[]
        for r in rows:
            if r.get('log'):
                outputs.append((r,(ROOT/r['log']).read_text(encoding='utf8')))
            else:outputs.append((r,r.get('stdout','')))
        oracle=[text for r,text in outputs if '--driver' in r.get('command',[])]
        need(len(oracle)==6 and all('"independent_visibility_paths": 1280' in t and '"uncached_matrices": 4096' in t for t in oracle),'six actual independent driver oracles')
        for r,text in outputs:
            if r.get('id','').endswith('.undefined') or r.get('id','').endswith('.link-undefined'):need(not text.strip(),'no implicit target helpers')
        for p,h in d.get('generated_artifact_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'target artifact '+p)
    targets=d.get('target_audits',[])
    need(len(targets)==4 and {(r.get('compiler'),r.get('optimization')) for r in targets}=={(c,o) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3']},'four actual target audits')
    for r in targets:need(r.get('language')=='c17' and r.get('execution')=='not-run' and r.get('undefined_symbols')==[] and r.get('libraries')==[],'target scope')
    need(len(d.get('generated_artifact_sha256',{}))==12,'complete target artifacts')
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=pathlib.Path);a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text(encoding='utf8'));errors=validate(d);controls=[]
    if a.self_test:
        edits=[('status',lambda x:x.update(passed=False)),('source',lambda x:x.update(scope_source_stable=False)),
               ('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),
               ('control',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),
               ('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),
               ('stub',lambda x:x['learner_stub_rejections'].pop()),('target',lambda x:x['target_audits'].pop()),
               ('target-execution',lambda x:x['target_audits'][0].update(execution='passed')),
               ('target-helper',lambda x:x['target_audits'][0].update(undefined_symbols=['__helper'])),
               ('artifact',lambda x:x['generated_artifact_sha256'].pop(next(iter(x['generated_artifact_sha256'])))),
               ('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        for name,edit in edits:
            candidate=copy.deepcopy(d);edit(candidate);rejected=bool(validate(candidate,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
