"""Reconcile exact original T65/T66 criteria without inferring from a family pass."""
from common import ROOT,dump,read_json,sha
def main():
    inventory='research/v5/modern/errors_ownership_remaining.json';data=read_json(ROOT/inventory);inputs={inventory:sha(ROOT/inventory),'tools/reconcile_errors_ownership_readiness.py':sha(ROOT/'tools/reconcile_errors_ownership_readiness.py')};rows=[];errors=[];ids=set()
    for original in data['rows']:
        evidence=original['evidence'][0];record=read_json(ROOT/evidence);inputs[evidence]=sha(ROOT/evidence);wanted=set(original['original_ids']);criteria={r['id'] for r in record['criteria']};variants={r['id'] for r in record['variants']}
        expected_criteria={x for x in wanted if '-R' in x};expected_variants={x for x in wanted if x.endswith('-DELAYED')}
        if criteria!=expected_criteria or variants!=expected_variants:errors.append({'exercise':original['exercise'],'error':'original criterion/variant mapping mismatch'})
        if any(r['author_readiness']!='ready-bounded' for r in record['criteria']):errors.append({'exercise':original['exercise'],'error':'unready criterion'})
        stale=[]
        for path,h in record['input_sha256'].items():
            p=ROOT/path
            if not p.is_file() or sha(p)!=h:stale.append(path)
        if stale:errors.append({'exercise':original['exercise'],'stale':stale})
        reports=record['current_reports']
        for path,h in reports.items():
            p=ROOT/path
            if not p.is_file() or sha(p)!=h:errors.append({'exercise':original['exercise'],'changed_report':path});continue
            d=read_json(p)
            if not d.get('passed') or d.get('status')!='passed':errors.append({'exercise':original['exercise'],'unpassed_report':path})
        ids.update(wanted);rows.append(dict(exercise=original['exercise'],original_ids=original['original_ids'],criteria=sorted(criteria),variants=sorted(variants),readiness=evidence,readiness_sha256=sha(ROOT/evidence),current_reports=reports,current_inputs_verified=not stale,independent_learner='not-assessed',external_review='not-run',physical_observation='per-criterion-record-retained'))
    result=dict(schema_version=1,scope='Exactly20original T65/T66 exercises,80rubrics and20delayed IDs; author evidence reconciliation only',passed=not errors,status='passed' if not errors else 'failed',exercises=len(rows),leaf_criteria=sum(len(r['criteria']) for r in rows),unique_original_ids=len(ids),rows=rows,errors=errors,input_sha256=inputs,limits=['Checks exact existing per-criterion support records and current source/report hashes; does not infer coverage from family success.','Does not execute campaigns again or replace their separate guards.','No independent-human, unaided delayed-learner or physical completion inference.','Published C17/C23 access and frozen dispatch text correction remain separate required records.'])
    dump(ROOT/'research/v5/modern/errors_ownership_readiness_reconciliation.json',result);print({'passed':result['passed'],'exercises':len(rows),'criteria':result['leaf_criteria'],'original_ids':len(ids),'errors':len(errors)});return result['passed']
if __name__=='__main__':raise SystemExit(0 if main() else 1)
