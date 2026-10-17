"""Reconcile every B06 leaf with actual authored units and bounded receipts."""
import hashlib
import json
from pathlib import Path

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[3]
def read(path):return json.loads((ROOT/path).read_text(encoding='utf-8'))
def main():
    coverage=read('research/v5/expansion/b06/coverage.json')
    core=read('research/v5/expansion/b06/core_validation.json')
    target=read('research/v5/expansion/b06/target_validation.json')
    rows=[];errors=[];paths={};families={}
    for domain in coverage['domains']:
        fragment=read('curriculum/v5/domains/'+domain['domain']+'.json')['domain'];family=fragment['family']
        families[domain['domain']]=read('labs/'+family+'/family.json')
        for path in ['labs/'+family+'/family.json','labs/'+family+'/include/core.h','labs/'+family+'/tests/tests.c','assessor/'+family+'/core.c']:
            paths[path]=hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
    for old in coverage['objective_map']:
        oid=old['objective'];family=families[old['domain']];units=[u for u in family['units'] if oid in u['objectives']]
        criteria=[r for u in units for r in u['rubric'] if r['objective']==oid]
        if len(units)!=2 or not criteria or any(set(r['levels'])!={'0','1','2','3'} for r in criteria):errors.append(oid+': missing authored unit or full leaf rubric')
        for path in [old['teaching']['path'],'curriculum/projects/'+old['implementation_analysis']['project']+'.md',old['assessor']]:
            text=(ROOT/path).read_text(encoding='utf-8');paths[path]=hashlib.sha256(text.encode()).hexdigest()
        suffix=oid.rsplit('.',1)[1]
        rows.append(dict(objective=oid,teaching=dict(status='authored',source=old['teaching'],claim='Mechanism explanation retained; source/title mention alone is not its assessment.'),construction_or_analysis=dict(status='author-core-executed; full written criterion remains separately assessed' if suffix=='b' else 'authored reasoning/counterexample obligation; no learner proof inferred',units=[u['id'] for u in units],criterion=criteria[0]['criterion']),boundary_adversarial=dict(status='named finite core/mutant evidence exists; broader target scope remains open',test=old['boundary_adversarial']['test'],mutation=old['boundary_adversarial']['mutation'],required_counterexample=old['boundary_adversarial']['required_counterexample']),independent_assessment=dict(status='not-attempted',criterion=criteria[0]['levels']['3'],variants=units[-1]['variants'],minimum_days=7),remaining_required=old['remaining_required'],assessor=old['assessor'],executed_claim_limit='Neither a passing model nor a compiled target establishes all the objective text, changed conditions, actual target behavior or independent learner understanding.'))
    assert len(rows)==192 and len({r['objective'] for r in rows})==192
    stale={}
    for label,receipt in [('core',core),('target',target)]:
        pins=receipt.get('input_sha256',{})
        stale[label]=[path for path,digest in pins.items() if not (ROOT/path).exists() or hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=digest]
    result=dict(batch='B06',kind='author structural and evidence-scope re-audit; not independent review',passed=not errors,errors=errors,leaf_objectives=192,objective_map=rows,actual_recorded_evidence=dict(core_passed=core['passed'],core_checks=len(core['checks']),target_passed=target['passed'],target_checks=len(target['checks']),stale_source_paths=stale),assessment_counts=dict(authored_leaf_rubrics=192,unattempted_independent_leaf_assessments=192,delayed_project_variants=64),consequential_open_gaps=['Linux model cleanup and lifetime transitions are not guest module load/unload, RCU, lockdep or KASAN evidence.','Cross-assembled RV64/AArch64 contexts are not executed hostile-register, reset, trap-return or translation ports.','Hosted firmware arrays and journal cuts do not establish real flash granularity, secure image verification, power loss or dual-core board behavior.','The compiled and emulated compiler is the explicitly unsigned expression notation; declarations, control flow, calls and pointers lack completed reference implementations.','CBMC harnesses have no solver result; finite state words and fuzz executions cannot be renamed proofs.','io_uring setup does not execute submission/cancellation; splice, crash persistence and changed buffer ownership remain open.','Local memory-security witnesses establish only their named original-target violations; no general exploitation or mitigation efficacy is inferred.','Vector instruction observations and numeric differential cases do not supply equivalent-contract timing, traffic or physical measurements.'],gap_ledger='research/v5/expansion/b06/gap_ledger.json',required_open_gaps=len(read('research/v5/expansion/b06/gap_ledger.json')['gaps']),comprehensive_complete=False,inspected_file_sha256=paths)
    (HERE/'depth_reaudit.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({k:result[k] for k in ['passed','leaf_objectives','actual_recorded_evidence','assessment_counts','required_open_gaps','comprehensive_complete']}))
if __name__=='__main__':main()
