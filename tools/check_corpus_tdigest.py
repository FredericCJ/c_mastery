"""Guard mechanism evidence and the negative accuracy findings together."""
import argparse,copy,json
from pathlib import Path
from common import ROOT,sha,dump
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_tdigest/current_validation.json'
def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_tdigest/family.json').read_text());errors=generic_errors('corpus_tdigest',meta,d)
    def need(value,message):
        if not value:errors.append(message)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'current stable campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external evidence boundary')
    if files:errors+=pin_errors(d)+execution_errors(d)
    observations=d.get('independent_oracles',[]);need(len(observations)==6,'six actual independent oracles')
    for row in observations:
        need(row.get('passed') is True and row.get('executed') is True,'executed oracle')
        need(row.get('cases')==570 and row.get('comparisons',0)>=18000 and row.get('exact_fresh_prefix_probes')==30,'distinct bounded construction and history scope')
        need(row.get('high_precision_scale_checks') is True,'independent50-digit invariant checks')
        need(row.get('maximum_observed_rank_error',0)>=.625 and row.get('maximum_observed_value_error',0)>1000000,'retained negative accuracy result')
        need(row.get('reference_arithmetic_divergences',0)>0 and row.get('reference_arithmetic_example'),'cross-arithmetic disagreement retained')
        witness=row.get('merge_order_counterexample',{});need(witness.get('first_quantiles')!=witness.get('second_quantiles') and len(witness.get('raw_values',[]))==512,'actual same-data merge-order witness')
        if files:
            raw=json.loads((ROOT/row['log']).read_text());need(all(raw.get(k)==v for k,v in row.items() if k!='log'),'raw observation identity')
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args();d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        changes=[('passed',lambda x:x.update(passed=False)),('stability',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].clear()),('oracle',lambda x:x['independent_oracles'].pop()),('scope',lambda x:x['independent_oracles'][0].update(cases=1)),('fresh-construction',lambda x:x['independent_oracles'][0].update(exact_fresh_prefix_probes=0)),('scale',lambda x:x['independent_oracles'][0].update(high_precision_scale_checks=False)),('false-accuracy',lambda x:x['independent_oracles'][0].update(maximum_observed_rank_error=0)),('order',lambda x:x['independent_oracles'][0].update(merge_order_counterexample={})),('rounding',lambda x:x['independent_oracles'][0].update(reference_arithmetic_divergences=0)),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        for name,change in changes:
            candidate=copy.deepcopy(d);change(candidate);rejected=bool(validate(candidate,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:dump(a.output,result)
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
