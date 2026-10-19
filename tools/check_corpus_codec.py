"""Check bounded codec claims without treating author tests as learner evidence."""
import argparse
import copy
import json
from pathlib import Path
from common import ROOT,sha,dump
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_codec/current_validation.json'

def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_codec/family.json').read_text())
    errors=generic_errors('corpus_codec',meta,d)
    def need(value,message):
        if not value:errors.append(message)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'successful stable source campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external limits')
    if files:
        errors+=pin_errors(d)+execution_errors(d)
        logs=[(r,(ROOT/r['log']).read_text()) for r in d.get('executions',[]) if r.get('log')]
        ctests=[s for r,s in logs if '--output-on-failure' in r['command'] and '100% tests passed' in s]
        need(len(ctests)==6 and all('corpus_codec.oracle' in s for s in ctests),'six executed independent oracles')
        for r,s in logs:
            if r['log'].endswith(('.object-undefined.log','.image-undefined.log')):need(not s.strip(),'empty actual target imports')
        for p,h in d.get('generated_artifact_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'generated artifact '+p)
    targets=d.get('target_audits',[])
    need(len(targets)==4 and {(r.get('compiler'),r.get('optimization')) for r in targets}=={(c,o) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3']},'four distinct target audits')
    for r in targets:need(r.get('execution')=='not-run' and r.get('undefined_symbols')==[] and r.get('libraries')==[],'target dependency and observation boundary')
    need(len(d.get('generated_artifact_sha256',{}))==16,'object/image/map/stack artifacts')
    need(len(d.get('measurements',[]))==2 and {r.get('compiler') for r in d.get('measurements',[])}=={'gcc','clang'},'two actual cost observations')
    for r in d.get('measurements',[]):need(r.get('workloads')==4 and r.get('algorithms')==2 and r.get('trials_per_pair')==1000 and len(r.get('raw','').splitlines())==8,'bounded cost scope')
    oracles=d.get('independent_oracles',[])
    need(len(oracles)==2 and {r.get('compiler') for r in oracles}=={'gcc','clang'} and all(r.get('passed') is True and r.get('cases',0)>10000 and r.get('greedy_counterexample') for r in oracles),'actual independent oracle observations')
    return errors

def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        mutations=[('passed',lambda x:x.update(passed=False)),('stable',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('oracle-anchor',lambda x:x['profiles'][0]['anchors'].clear()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].clear()),('target',lambda x:x['target_audits'].pop()),('helper',lambda x:x['target_audits'][0].update(undefined_symbols=['memcpy'])),('target-observation',lambda x:x['target_audits'][0].update(execution='passed')),('artifact',lambda x:x['generated_artifact_sha256'].pop(next(iter(x['generated_artifact_sha256'])))),('measurement',lambda x:x['measurements'].clear()),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        mutations.append(('oracle',lambda x:x['independent_oracles'].clear()))
        for name,change in mutations:
            candidate=copy.deepcopy(d);change(candidate);rejected=bool(validate(candidate,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:dump(a.output,result)
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
