"""Exact arithmetic, ownership, target closure and evidence-negative guards."""
import argparse
import copy
import json
from pathlib import Path
from common import ROOT,sha
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_limbs/current_validation.json'
def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_limbs/family.json').read_text());errors=generic_errors('corpus_limbs',meta,d)
    def need(ok,why):
        if not ok:errors.append(why)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'successful stable campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external evidence limit')
    controls={'corpus_limbs.mutation.'+('arithmetic' if i<5 else 'ownership')+str(bits)+'.'+str(i) for bits in [8,16] for i in range(1,7)}
    for profile in d.get('profiles',[]):
        need(set(profile.get('compiled_mutant_controls',[]))==controls and profile.get('ctest_count')==16,'all twelve causal controls')
    if files:
        errors+=pin_errors(d)+execution_errors(d)
        oracle=[]
        for row in d.get('executions',[]):
            text=(ROOT/row['log']).read_text() if row.get('log') else row.get('stdout','')
            if '--driver' in row.get('command',[]):oracle.append(text)
            if row.get('id','').endswith(('.undefined','.link-undefined')):need(not text.strip(),'no implicit helper')
        need(len(oracle)==6 and all('"total_operations": 2544' in x and '"bits": 8' in x and '"bits": 16' in x for x in oracle),'both independent radix oracles in all profiles')
        for p,h in d.get('generated_artifact_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'actual target artifact '+p)
    targets=d.get('target_audits',[])
    need(len(targets)==8 and {(r.get('compiler'),r.get('optimization'),r.get('bits')) for r in targets}=={(c,o,b) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3'] for b in [8,16]},'eight strict target audits')
    for row in targets:need(row.get('language')=='c17' and row.get('execution')=='not-run' and row.get('undefined_symbols')==[] and row.get('libraries')==[],'target evidence scope')
    need(len(d.get('generated_artifact_sha256',{}))==24,'target artifact count')
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    edits=[('failed',lambda x:x.update(passed=False)),('changed',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].pop()),('radix',lambda x:x['target_audits'][0].update(bits=32)),('target',lambda x:x['target_audits'].pop()),('helper',lambda x:x['target_audits'][0].update(undefined_symbols=['memset'])),('execution',lambda x:x['target_audits'][0].update(execution='passed')),('artifact',lambda x:x['generated_artifact_sha256'].pop(next(iter(x['generated_artifact_sha256'])))),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
    if a.self_test:
        for name,edit in edits:
            candidate=copy.deepcopy(d);edit(candidate);rejected=bool(validate(candidate,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
