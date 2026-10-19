"""Source-bound validation with independent controls for each statistical mechanism."""
import argparse,copy,json
from pathlib import Path
from common import ROOT,sha,dump
from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
DEFAULT='reviews/v5/expansion/corpus_sketch/current_validation.json'
def validate(d,files=True):
    meta=json.loads((ROOT/'labs/corpus_sketch/family.json').read_text());errors=generic_errors('corpus_sketch',meta,d)
    def need(value,message):
        if not value:errors.append(message)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'current passed campaign')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','external boundary')
    if files:errors+=pin_errors(d)+execution_errors(d)
    observations=d.get('independent_oracles',[])
    need(len(observations)==6,'six separately executed oracles')
    for r in observations:
        need(r.get('passed') is True and r.get('executed') is True,'actual oracle execution')
        need(r.get('cases',0)>2500 and r.get('affine_pair_bins')==96,'finite case scope')
        for key,bound in [('count_min',15000),('hyperloglog',50000),('minhash',130000)]:need(r.get('comparisons',{}).get(key,0)>=bound,'independent '+key+' comparisons')
        need(r.get('minhash_all_permutations')==120 and r.get('band_false_positive') and r.get('band_false_negative'),'permutation and candidate counterexamples')
        if files:
            path=ROOT/r['log'];need(path.is_file(),'oracle raw log')
            if path.is_file():need(all(json.loads(path.read_text()).get(k)==r.get(k) for k in ['comparisons','cases','band_false_positive','band_false_negative']),'raw oracle identity')
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=Path);a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        changes=[('passed',lambda x:x.update(passed=False)),('stability',lambda x:x.update(scope_source_stable=False)),('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),('mutant',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('mapping',lambda x:x['profiles'][0].update(controls_by_anchor={})),('stub',lambda x:x['learner_stub_rejections'].clear()),('oracle',lambda x:x['independent_oracles'].pop()),('finite-scope',lambda x:x['independent_oracles'][0].update(cases=0)),('permutation',lambda x:x['independent_oracles'][0].update(minhash_all_permutations=119)),('false-negative',lambda x:x['independent_oracles'][0].update(band_false_negative=None)),('execution',lambda x:x['independent_oracles'][0].update(executed=False)),('physical',lambda x:x.update(physical_validation='passed')),('learner',lambda x:x.update(learner_mastery='passed'))]
        for name in ['count_min','hyperloglog','minhash']:
            changes.append((name,lambda x,key=name:x['independent_oracles'][0]['comparisons'].update({key:0})))
        for name,change in changes:
            candidate=copy.deepcopy(d);change(candidate);rejected=bool(validate(candidate,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    out=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:dump(a.output,out)
    print(json.dumps(out));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
