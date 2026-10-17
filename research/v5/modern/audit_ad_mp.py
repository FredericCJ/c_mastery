"""Maintain scoped authored-depth and remaining-evidence records, never acceptance."""
import hashlib
import json
from pathlib import Path
from author_adt import DATA as ADT
from author_metaprog import DATA as MP
from render_ad_mp import dump
ROOT=Path(__file__).resolve().parents[3]

PENDING={
 'T67':['Owning-sum extension and diagnostics after adding a new variant.','Signed rational-unit transfer and measured wrapper overhead.','Full raw-mask exhaustive C campaign and negotiated required/advisory policy.','Incremental decoder split/finalization campaign; normalization is outside this constructor.','Independent enumeration of short pool traces and relocated/concurrent lifetime transfer.','Mutable injectivity and SIMD composition under identical alias/overflow contracts.','Whole-family tree/arena differential corpus and cyclic dataflow transfer.','Nested/partial optic composition and nontrivial callback laws.','Two-version relay integration and duplicate/canonical extension policy.','Actual learner io.frame/bytes.cursor/codec.records adapter and retained receive-ring lifetime.'],
 'T68':['Two-translation-unit/template collision campaign and owning-element transfer.','Changed size_t model and runtime fallback to constant capacity.','Measured specialized/dynamic policy comparison with effectful projection.','Protected-comma/logging expansion variants and exact arity diagnostic interface.','Volatile receiver target observations and cross-implementation qualifier corner cases.','Schema-change regeneration across all consumers and stale schema fingerprint detection.','Actual testing.faults adapter, operation-trace generator and precondition-preserving ownership shrinker.','Parsed format DSL and partial streaming failure/retry transfer.','Nested/repeated expansion budgets and full environment-resolution investigation.','Crash-safe incremental cache publication, hidden-input campaign and forced-collision handling.']}

def main():
    rows=[];gaps=[]
    for domain,family,data in [('T67','adt',ADT),('T68','metaprog',MP)]:
        for index,d in enumerate(data,1):
            identifier=f'X-{domain}-{index:03}';path=ROOT/'curriculum/v5/exercises'/(identifier+'.md')
            metadata=json.loads(path.read_text(encoding='utf-8').split('```')[1][4:])
            for leaf in metadata['objective_coverage']:
                rows.append(dict(source_records=metadata['source_records'],objective=leaf['objective'],assessed_claim=leaf['assessed_claim'],exercise=identifier,project=metadata['project'],teaching=leaf['teaching'],learner_artifacts=leaf['implementation'],boundary=leaf['boundary'],assessment=leaf['independent_assessment'],validation='authored mapping; finite author execution tracked separately'))
            gaps.append(dict(id=f'{domain}-{index:02}-TRANSFER',kind='independent-delayed-assessment',required=True,status='not-run',exercise=identifier,task=d['transfer'],minimum_days=7,reason='No learner or independent reviewer attempt is inferred from the author implementation.'))
            gaps.append(dict(id=f'{domain}-{index:02}-INVESTIGATION',kind='unvalidated-required-investigation',required=True,status='authored-not-executed',exercise=identifier,task=PENDING[domain][index-1],reason='The bounded C anchors and independent oracle do not execute this complete written obligation.'))
        for kind in ['C17','C23']:
            gaps.append(dict(id=domain+'-'+kind+'-REFERENCE',kind='unavailable-reference',required=True,status='reference-gap',source=kind,reason='Published C17 and final C23 passages were not inspected. Registered C11/N3033 passages and C2x diagnostic observations are bounded substitutes, not claimed equivalent source inspection.'))
        gaps.append(dict(id=domain+'-MEASUREMENTS',kind='performance-research',required=True,status='partially-executed',task='Complete measured thirteen-dimension comparisons under equal contracts; compiler stack/object reports exist, workload latency/throughput and all changed conditions do not.'))
    dump('research/v5/modern/ad_mp_depth_audit.json',dict(domains=['T67','T68'],baseline='Previously planned domain mechanisms were not exercise coverage; this batch authors concrete contracts and objective-specific assessments.',expansion={'projects':20,'leaf_objectives':80,'units':40,'exercises':20,'glossary_terms':40,'recall_variants':60,'compiled_semantic_mutants':40},traceability=rows,completion_claim='Authored and bounded host-executable coverage; no absolute mastery, independent learner assessment or full required-experiment completion.'))
    dump('research/v5/modern/ad_mp_gap_ledger.json',dict(gaps=gaps,required_open=len(gaps),comprehensive_complete=False))
    dump('research/v5/modern/ad_mp_remaining_work.json',dict(domains=['T67','T68'],authoring_status='delivered with explicit required investigation gaps',learner_status='not-assessed',required_open_gaps=[g['id'] for g in gaps],remaining_domains=['T76'],preserve_prior_gap_ledgers=True,comprehensive_complete=False))
    print(json.dumps(dict(traceability_rows=len(rows),required_open_gaps=len(gaps))))

if __name__=='__main__':main()
