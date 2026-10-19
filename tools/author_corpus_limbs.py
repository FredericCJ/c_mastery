"""Canonical required variable-count arithmetic refinement and inspected source gate."""
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_limbs'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
    locators=['14.2.1 pp.592-594 radix length, zero and signed-magnitude representation',
              '14.2.2 algorithms14.7/14.9 pp.594-595 carry and borrow',
              '14.2.3 algorithm14.12 and note14.15 pp.595-596 inner-product bound B^2-1',
              '14.2.5 algorithm14.20 and notes14.22-14.25 pp.598-599 quotient/remainder and normalized correction']
    source=dict(id='R-CL-HAC14',title='Handbook of Applied Cryptography, chapter14',authors=['Alfred J. Menezes','Paul C. van Oorschot','Scott A. Vanstone'],year=1996,edition='1996 book; electronic chapter copyright1997',kind='book',url='https://cacr.uwaterloo.ca/hac/about/chap14.pdf',access='Authorized author-hosted electronic chapter; private copy only, not redistributed',domains=['T11'],role='primary',reason='Derive variable-length representation, carry/borrow, multiplication bounds and division comparison',prerequisites=['Unsigned arithmetic','Array extents','Invariant induction'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Actual downloaded primary PDF text read at pp.592-599; rendered printed595 and598 formulas inspected',scope='; '.join(locators),limits='Only selected clauses; no Knuth access claim, cryptographic conformance or complete bignum algorithm coverage',private_payload='build/corpus_limbs_sources/hac14.pdf',sha256=sha(ROOT/'build/corpus_limbs_sources/hac14.pdf')))
    dump(ROOT/BASE/'resources.json',dict(resources=[source]))
    template=json.loads((ROOT/'curriculum/v5/exercises/X-T14-011.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0])
    eid='X-T11-002';objectives=['T11.04.c','T11.04.a','T11.05.c']
    claims=[('T11.04.c','Derive canonical runtime-length radix representation, carry/borrow and exact multiplication accumulator bounds; implement transactional magnitude arithmetic and quotient/remainder.'),
            ('T11.04.c','Implement signed truncating division and an owned capacity adapter; prove old borrows survive failed allocation and expire only after successful replacement.'),
            ('T11.04.a','Audit actual generated target objects and linked images at both radices/optimizations; distinguish implementation support, emitted helpers and hosted verification dependencies.'),
            ('T11.05.c','Falsify six distinct defects with independent arbitrary-size arithmetic and refusal snapshots; transfer16-to8-bit limbs and caller storage to signed owning growth with explicit evidence limits.')]
    meta=copy.deepcopy(template)
    meta.update(id=eid,title='Variable-length limbs, restoring division and owned capacity',project='PR-T11-02',domain='T11',objectives=objectives,prerequisite_exercises=['X-T04-001','X-T03-003'],prerequisite_artifacts=[],lab=dict(family='corpus_limbs',scaffold='labs/corpus_limbs',tests=['corpus_limbs.'+a+str(b) for b in [16,8] for a in ['arithmetic','ownership']]),fixtures=['fixtures/corpus_limbs/cases.json'],shared_contracts=['labs/corpus_limbs/CONTRACTS.md'],assessor_analysis='assessor/corpus_limbs/ANALYSIS.md')
    meta['environment']=dict(language='ISO C17 with exact uint16_t/uint32_t; published C17/C23 source access remains separate',target='Hosted Linux; portable runtime-count core; Cortex-M4 object/link audits only',toolchain=['GCC','Clang','CMake','Python3','Arm GCC'],dependencies='First-principles arithmetic. Allocator callbacks supply storage only; host Python integers are an independent verification dependency.')
    meta['source_records']=[dict(id=source['id'],path=BASE+'/resources.json',passages=locators)]
    meta['rubric']=[dict(id=eid+f'-R{i+1}',objective=o,criterion=c,levels={'0':'Breaks arithmetic, ownership or evidence contract.','1':'Runs one fixture without invariant or changed condition.','2':c+' Supply exact source-bound counterexamples and observations.','3':'Independently reconstructs both variants at least7days later.'}) for i,(o,c) in enumerate(claims)]
    meta['objective_coverage']=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in meta['rubric'] if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in meta['rubric'] if r['objective']==o)) for o in objectives]
    meta['status']['limitations']=['Runtime limb count has no fixed implementation maximum; the finite driver workload is not its API extent.','No constant-time cryptography, physical target observation or independent learner assessment.']
    body='''# X-T11-002: Variable-length limbs, restoring division and owned capacity

## Prerequisites
Complete X-T04-001 checked extents and X-T03-003 allocated typed lifetime. Supply the learner prefix arithmetic and allocation-failure arguments. Study PR-T11-02 helper census before the target audit. No old assessor code is silently imported.

## Environment
Use C17 Profile C under the shared contract. BL_BITS16 and8 use actual uint16_t objects and uint32_t intermediates. Host malloc/free supplies only storage; it is not a target dependency. Inputs, scratch, output and controls obey the stated disjointness and lifetime rules.

## Problem
Implement runtime-count little-endian limb addition, nonnegative subtraction, multiplication and quotient/remainder. Count and capacity are explicit size_t values, not a hidden128-bit integer. Then implement signed addition/truncating division and allocation-fallible owner replacement. Supply all functions in src/limbs.c; the public header and contract define exact inputs, outcomes, scratch, failure and publication semantics.

## Necessity
Fixed128-bit success does not assess changing extent, noncanonical length, accumulator growth, dual-output publication or old/new allocation authority. A correct quotient with a partially published remainder is still a failed transaction.

## Reasoning
Prove each consumed-prefix invariant and B^2-1 accumulator bound. Derive restoring division separately from inspected normalized quotient estimation, including the extra remainder limb. Explain size_t overflow checks, zero representation, sign rules, scratch and old/new owner peaks. ISO C supplies arithmetic/types; ownership, overlap and callback discipline require checks or explicit premises.

## Deliverables
D1 representation and invariant proofs plus incorrect explanations and counterexamples. D2 complete core, signed adapter and actual allocation-failure harness. D3 independent arithmetic, six mutants, refusal snapshots and target dependency audits. D4 both changed-condition implementations and cost/evidence ledger. Do not copy the separate assessor solution.

## Verification
Run all four public anchors and twelve compiled controls across six C17 profiles. Check1272 exact operations per radix/profile with Python integers; test all-maximum carry/borrow chains, zero/one, unequal length, exact and insufficient capacities, malformed leading zeros and failed self-borrowed growth. Require both q/r outputs unchanged on refusal. Strict target images must have no undefined helper or linked library; links do not prove target execution.

## Rubric
Assess R1-R4 separately. Level1 reproduces ordinary examples; level2 requires derivation, implementation, diagnosis and exact bounded evidence; level3 requires independent delayed transfer. The reference campaign awards no learner mastery.

## Hints
H1 write the value of a processed prefix. H2 append one division bit and bound the new remainder by2*y. H3 draw the sole output/owner commit point and retain prior authority through every refusal.

## Variants
V1: Change16-bit digits to8-bit digits for the same mathematical values; rederive intermediate, scratch, storage and helper costs rather than reinterpret bytes.

V2: Replace unsigned caller outputs with signed, owned results. Inject failed growth while the input borrows the old owner; compare truncating and floor division under all sign combinations, then prove successful replacement invalidates the borrow.

## Security
Use only original local synthetic arithmetic and allocator controls. Adversarial admitted counts/digits/budgets expose wrong values, partial publication or an owner leak. No malformed pointer or external target is allowed. Repair checked extents, staged publication and matching release, then retain regressions and timing-leak limitations. This is not a constant-time cryptographic library.

## Validation
Actual host execution, strict object/link audits, independent learner assessment and physical evidence are distinct. Current scoped receipt is reviews/v5/expansion/corpus_limbs/current_validation.json. Exact published C17/C23 and Knuth volume2 passages remain inaccessible; the authorized HAC14 passages support only their registered claims. Finite passing tests do not prove absolute mastery.
'''
    (ROOT/'curriculum/v5/exercises'/f'{eid}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',scope_rationale='The existing T11 arithmetic-helper selection leaf is refined with an actual variable-count first-principles implementation; fixed128-bit support is not equivalent.',refinements=[dict(exercise=eid,required=True,parent_objectives=objectives,competencies=[dict(id=r['id'],outcome=r['criterion'],source=[source['id']]) for r in meta['rubric']])]))
    pins=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json'];dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in pins},limitations='Author selected primary inspection only; private PDF is not redistributed. Exact published ISO and Knuth access gaps remain.'))
    dump(ROOT/BASE/'audit.json',dict(status='confirmed-variable-count-gap-implemented',inspected_inputs=['curriculum/projects/PR-T11-02.md','curriculum/modules/T11.md','research/v5/expansion/corpus_followthrough/screening_map.json'],distinction='Runtime size_t count/capacity plus staged scratch and fallible ownership, not fixed128-bit arithmetic.',reference_gaps=['Exact published C17/C23','Knuth volume2 section4.3'],limits='No subquadratic multiplication, normalized digit-estimation implementation, modular cryptographic protocol or constant-time claim.'))
    dump(ROOT/'fixtures/corpus_limbs/cases.json',dict(radices=[8,16],sum=dict(bits=16,a=[65535,65535],b=[1],expected=[0,0,1]),multiply=dict(a=9274,b=847,expected=7855078),divide=dict(a=721948327,b=84461,quotient=8547,remainder=60160)))
    dump(ROOT/'labs/corpus_limbs/family.json',dict(version=1,family='corpus_limbs',title='Runtime-length arithmetic and capacity ownership',build_system='cmake',assessor_root='assessor/corpus_limbs',dependencies=[],projects=[],units=[],exercises=[dict(id=eid,project='PR-T11-02',tests=meta['lab']['tests'])],tests=meta['lab']['tests'],campaign=dict(producer='tools/run_corpus_limbs.py',receipt='corpus_limbs_campaign.json',profiles=[c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']],mutants_per_anchor=2,assertion_marker='BL_CHECK:',fixture_oracle='tools/verify_corpus_limbs.py',oracle_driver='limbs16')))
    print('Authored required X-T11-002 variable-count refinement')
if __name__=='__main__':main()
