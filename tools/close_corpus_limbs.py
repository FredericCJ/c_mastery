"""Bind exact required runtime-length arithmetic criteria and changed conditions."""
import json
import sys
from common import ROOT,sha
from check_corpus_limbs import validate
sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
BASE='research/v5/expansion/corpus_limbs'
REPORTS=['reviews/v5/expansion/corpus_limbs/'+n+'.json' for n in
         ['current_validation','support_validation','export_validation']]
def main():
    campaign,guard,export=[json.loads((ROOT/p).read_text()) for p in REPORTS]
    assert not validate(campaign)
    assert guard['passed'] and len(guard['negative_controls'])==14 and guard['report_sha256']==sha(ROOT/REPORTS[0])
    assert export['passed'] and export['source_stable'] and not export['private_sources_copied']
    assert len(export['exports'])==1 and len(export['executions'])==11 and all(r['passed'] for r in export['executions'])
    assert all(sha(ROOT/p)==h for p,h in export['input_sha256'].items())
    brief='curriculum/v5/exercises/X-T11-002.md';meta,_=parse_brief(ROOT/brief)
    code=['assessor/corpus_limbs/limbs.c','labs/corpus_limbs/limbs.h','labs/corpus_limbs/tests.c','assessor/corpus_limbs/oracle.py']
    teaching=[brief,'labs/corpus_limbs/CONTRACTS.md','assessor/corpus_limbs/ANALYSIS.md']
    claims={meta['id']:'All four exact criteria and both required implemented changed conditions, limited to the declared caller storage and allocator premises.'}
    claims.update({r['id']:r['criterion'] for r in meta['rubric']})
    claims[meta['id']+'-V1']='Actual16/8-bit radix implementations compare2544 exact arbitrary-length operations per profile, with separately compiled controls and eight target object/link audits.'
    claims[meta['id']+'-V2']='Signed truncation under all dividend/divisor signs, canonical zero, failed growth preserving a live old borrow, successful self-copy replacement and exact release. No revoked pointer is read after replacement.'
    rows=[]
    for ident,claim in claims.items():
        rows.append(dict(id=ident,parent_exercise=meta['id'],parent_project=meta['project'],dimensions=dict(
            teaching=dict(state='complete-for-declared-contract',evidence=teaching),implementation=dict(state='complete-variable-count-and-changed-conditions',evidence=code),author_validation=dict(state='passed-source-bound-host-oracle-export-target-audit',evidence=REPORTS),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run;target-link-only',evidence=[])),closure_rationale=claim,artifacts=teaching+code,remaining_author_work=[],evidence_limits=[
            'No fixed implementation limb maximum; the finite driver does not prove every possible extent or input.',
            'Binary restoring division is derived separately from the inspected normalized digit-estimation algorithm; no performance-equivalence claim.',
            'Exact published C17/C23 and Knuth volume2 remain uninspected; registered HAC14 passages are the actual primary evidence.',
            'Target section sizes and emitted function frames are actual; physical execution, whole-caller stack bounds and constant-time cryptography are not claimed.',
            'Reference execution is not independent delayed learner assessment or human-independent review.']))
    paths=set(campaign['inspected_paths'])|set(export['input_sha256'])|set(REPORTS)|set(teaching+code)
    paths|={BASE+'/'+p for p in ['resources.json','taxonomy_extensions.json','research_gate.json','audit.json']}
    paths|={'tools/author_corpus_limbs.py','tools/close_corpus_limbs.py'}
    out=ROOT/BASE/'completion_dispositions.json'
    result=dict(schema_version=1,scope='Exact variable-count arithmetic and ownership author support only.',source_sha256={p:sha(ROOT/p) for p in sorted(paths)},current_reports=REPORTS,updates=rows)
    out.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(passed=True,updates=len(rows),path=out.relative_to(ROOT).as_posix(),sha256=sha(out))))
if __name__=='__main__':main()
