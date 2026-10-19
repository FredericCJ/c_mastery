"""Discriminate token-ring evidence and false completeness claims."""
import argparse,copy,json
from pathlib import Path
from common import ROOT,sha,dump
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_ring/current_validation.json'
def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_ring/family.json').read_text());errors=generic_errors('corpus_ring',meta,d)
    def need(value,reason):
        if not value:errors.append(reason)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'stable passed campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','separate external evidence')
    targets=d.get('target_audits',[])
    need(len(targets)==4 and {(r.get('compiler'),r.get('optimization')) for r in targets}=={(c,o) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3']},'four target audits')
    for r in targets:need(r.get('final_imports')==[] and r.get('libraries')==[] and r.get('execution')=='not-run' and r.get('original_provider')=='assessor/execution/support.c','original runtime closure and observation scope')
    need(len(d.get('generated_artifact_sha256',{}))==24,'actual objects/images/maps/stack reports')
    oracles=d.get('independent_oracles',[])
    need(len(oracles)==2 and {r.get('compiler') for r in oracles}=={'gcc','clang'} and all(r.get('passed') and r.get('seed')==760010 and r.get('observations')==14414 for r in oracles),'two independent driver oracles')
    measured=d.get('measurements',[])
    need(len(measured)==2 and all(r.get('keys_per_layout')==65536 and 'load_max=65536' in r.get('raw','') and len(r.get('raw','').splitlines())==16 for r in measured),'actual comparable cost observations')
    need(len(d.get('hash_integrations',[]))==2 and {r.get('compiler') for r in d.get('hash_integrations',[])}=={'gcc','clang'} and all(r.get('passed') and r.get('observations')==513 and r.get('provider')=='assessor/replicated/hashing/hashing.c' for r in d.get('hash_integrations',[])),'actual existing SHA adapter')
    if files:
        errors+=pin_errors(d)+execution_errors(d)
        logs=[(r,(ROOT/r['log']).read_text()) for r in d.get('executions',[]) if r.get('log')]
        suites=[s for r,s in logs if '--output-on-failure' in r['command'] and '100% tests passed' in s]
        need(len(suites)==6 and all('corpus_ring.oracle' in s for s in suites),'six actual suites with independent oracle')
        need(sum('CR_HELPERS:1040 passed' in s for r,s in logs)==2,'actual original helper checks')
        for r,s in logs:
            if r['log'].endswith(('.helper-imports.log','.image-imports.log')):need(not s.strip(),'no external helper imports')
        for p,h in d.get('generated_artifact_sha256',{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'target artifact '+p)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        edits=[('hash-integration',lambda x:x['hash_integrations'].clear()),('pass',lambda x:x.update(passed=False)),('stable',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].pop()),('target',lambda x:x['target_audits'].pop()),('helper',lambda x:x['target_audits'][0].update(final_imports=['memcpy'])),('library',lambda x:x['target_audits'][0].update(libraries=['libc'])),('target-run',lambda x:x['target_audits'][0].update(execution='passed')),('artifact',lambda x:x['generated_artifact_sha256'].pop(next(iter(x['generated_artifact_sha256'])))),('oracle',lambda x:x['independent_oracles'].clear()),('cost',lambda x:x['measurements'].clear()),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        for name,edit in edits:
            c=copy.deepcopy(d);edit(c);rejected=bool(validate(c,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:dump(a.output,result)
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
