"""Canonical source for the additive fixed-grid interval exercise."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def write(path,value):
 p=ROOT/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8')
source='R-CINTERVAL-SAND2007'
locators=['Section3.1 Interval arithmetic, printed p19','Section3.2 Outward-directed rounding and section3.3 Repeated parameter problem, printed pp20-21']
write('research/v5/expansion/corpus_interval/resources.json',dict(resources=[dict(id=source,title='Experimental Uncertainty Estimation and Statistics for Data Having Interval Uncertainty',authors=['Scott Ferson','Vladik Kreinovich','Janos Hajagos','William Oberkampf','Lev Ginzburg'],edition='SAND2007-0939, Unlimited Release, May2007',year=2007,kind='report',url='https://digital.library.unt.edu/ark:/67531/metadc887497/m2/1/high_res_d/910198.pdf',access='public',domains=['T17'],role='reference',reason='Separate real-set enclosure, outward numerical rounding and repeated-parameter dependency.',prerequisites=['Ordered real intervals','Integer quotient and remainder'],locators=locators,assignable=True,inspection=dict(status='passage-inspected',date='2026-10-18',method='Primary report text read via browser; private PDF captured and printed pp19-20 rendered and visually inspected for endpoint formulas.',scope='; '.join(locators),limits='Selected interval-analysis passages only, not all162pages, statistical inference chapters or a current IEEE1788 conformance assessment. Author-derived fixed-grid C adaptation uses no floating arithmetic.',sha256='26e6dcce06e3f9d8102d84b5b471f9ad4b3f122f092ace33b340899326cedb3f'))]))
id='X-T17-003';objectives=['T17.01.b','T17.03.b','T17.01.c']
criteria=[
 'Distinguish asserted physical input bounds, interval dependency overestimation and grid rounding; demonstrate x-x, independent a-b and square(a) versus a*a with correct correlation claims.',
 'Implement outward arithmetic and rescaling with widened signed bounds, all rejection statuses and output preservation; prove why clamping or truncating a negative lower endpoint loses containment.',
 'Construct an independent exact-rational oracle and minimal-grid witnesses, falsify both semantic mutants, and independently transfer bits16 enclosures to bits8 and back without claiming recovered information.'
]
rubric=[dict(id=id+'-R'+str(i),objective=o,criterion=c,levels={'0':'Missing or false containment/implementation claim.','1':'An example passes, but the named distinction or failure/rounding proof is absent.','2':c+' Provide the ordinary implementation, exact expected cases and explicit finite-evidence limits.','3':'Re-derive and implement the named delayed precision change without consulting the assessor; explain changed range, width, costs and unrecoverable uncertainty.'}) for i,(o,c) in enumerate(zip(objectives,criteria),1)]
public=['labs/numeric_interval/'+n for n in ['interval.h','tests.c','driver.c','CMakeLists.txt','reject_mutant.py']]
meta=dict(schema_version=1,id=id,title='Outward interval arithmetic without floating-environment assumptions',project='PR-T17-01',domain='T17',objectives=objectives,families=['A','B','C','D','F','H','I','J'],required=True,prerequisite_exercises=[],prerequisite_artifacts=[],profile='C',environment=dict(language='C17 baseline; exact published standard remains a separately tracked source gap',target='Hosted Linux verification of a portable typed-storage component',toolchain=['GCC','Clang','CMake','Python3'],dependencies='Original integer interval implementation; host-only exact Fraction oracle; compiler-generated integer/copy helpers explicitly audited if target claims are made'),source_records=[dict(id=source,path='research/v5/expansion/corpus_interval/resources.json',passages=locators)],lab=dict(family='numeric_interval',scaffold='labs/numeric_interval',tests=['numeric_interval.enclosure']),assessor_analysis='assessor/numeric_interval/analyses/'+id+'.md',status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',limitations=['No physical bounds are validated by an interval calculation.','Finite author experiments do not establish independent learner mastery.','No binary64 or IEEE1788 conformance claim.'],campaign_receipt='reviews/v5/expansion/corpus_interval/numeric_interval_campaign.json',learner_assessment='not-assessed'),fixtures=public+['fixtures/numeric_interval/cases.json'],shared_contracts=['labs/numeric_interval/CONTRACTS.md'],delayed_transfer=dict(minimum_days=7,variant_ids=[id+'-DELAYED']),objective_coverage=[dict(objective=o,teaching=['#problem','#necessity','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id']]) for o,r in zip(objectives,rubric)],rubric=rubric)
body='''# X-T17-003: Outward interval arithmetic without floating-environment assumptions

## Prerequisites

Study T04 signed arithmetic, T17.01 numerical models, T17.03 scaled values and the PR-T17-01 error ledger. Bring a written integer division/remainder table for positive and negative numerators, and your own exact rational calculations. No unlisted prebuilt library or accepted artifact import is required. Derive the int32 product bound before implementation.

## Environment

The explicit shared contract is [numeric interval contract1](../../../labs/numeric_interval/CONTRACTS.md). It supplies every API operation, storage, scale, ownership, admission precedence, dependency and isolation premise. Use the compiling unsolved `src/interval.c`. Host Python is an independent verifier; it is not linked into the C component. Record actual compiler identity and exact-width support. ISO C diagnostics do not prove real-set containment.

## Problem

Build a closed real-interval type represented by two int32 grid endpoints and0..16 fractional bits. Implement rational admission, addition, subtraction, multiplication, division, hull, intersection, correlated square and outward rescaling. Use synthetic voltage bounds [1/3,2/3] volts, and separately model two independent readings versus two uses of one reading. Compute at bits16 and bits8. Reproduce all concrete shared-contract outputs, including -1/3 at bits8, a square crossing zero, touching intersection and preserved output on zero-crossing division. No external interval package or arbitrary-precision arithmetic may implement the C mechanism.

## Necessity

Scalar tolerance tests cannot establish an enclosure. Ordinary truncation can raise a negative lower endpoint and exclude possible values. Saturation can also remove a valid result. Repeating an uncertain value is a correlation question: an interval type cannot automatically know two operands represent the same measurement. A larger output width can be sound while unnecessarily imprecise.

## Reasoning

Derive extrema of each operation over the full real input intervals, not only endpoint grid values. Prove the largest widened intermediate and that every division denominator is admitted before evaluation. Derive floor and ceiling from C quotient/remainder signs, including negative divisors. Explain why division containing zero is rejected and why refinement after coarsening cannot reconstruct discarded bounds. Prove transactionality by staging endpoints. State fixed storage/work and inspect the selected compiler's generated division support. Refute the explanations 'rounding to nearest is more accurate, therefore safe' and 'a*a is always the tight range of x squared'. Separate physical calibration assumptions from mathematical containment.

## Deliverables

- D1: Endpoint/range/rounding derivations, physical-versus-representation error ledger and exact primary-passage comparison.
- D2: Original C implementation and compiled resource/dependency ledger, with each rejected call's post-state contract.
- D3: Independent rational oracle, minimized mutant witnesses, input/output records, containment and minimal-grid checks including rational interior points.
- D4: Named delayed precision-transfer implementation, fresh argument and width/range comparison under the same real input contract.

## Verification

Run the supplied C boundary tests and compare all endpoint intervals drawn from -4..4 at scales0,8,16 against exact rational operations. Include int32 extrema, invalid/reversed operands, incompatible scales, denominator zero/endpoints, empty intersection, aliasing output with an input value and scale17 rejection. Interior thirds/halves verify that the modeled set is continuous. Compare exact minimum/maximum enclosures, not just a tolerance. The negative-rounding mutant and zero-crossing-square mutant must fail different assertions. Compile GCC/Clang debug, optimized and sanitizer profiles. Reproduce coarsening/refinement and range failure with preserved output. The source-bound campaign runs the C driver against a separate Fraction model; passing these cases does not replace D1 or D4.

Configure `cmake -S labs/numeric_interval -B <fresh-build>`, build and run `ctest --test-dir <fresh-build> --output-on-failure`. The unsolved scaffold must compile and fail with NI_ASSERT. Assessor implementations and independent oracle are withheld.

## Rubric

Apply the three objective-specific JSON criteria individually. Ordinary acceptance requires at least level2 for each; independent delayed transfer requires level3. A correct-looking width with a false containment or physical-confidence explanation fails. No finite passing suite proves absolute mastery or validates a sensor's calibration.

## Hints

1. Draw the real values represented by one grid interval; endpoint storage is finite while the represented set is continuous.
2. Work out -1/3 using integer quotient and remainder. Which endpoint needs one unit of correction?
3. Separate a function of one shared variable from an operation on two independently varying operands; test whether zero belongs to the domain before deriving a minimum square.

## Variants

X-T17-003-DELAYED: after at least seven days, derive and implement outward bits16-to-bits8 conversion and exact refinement back to bits16, with range rejection. Supply an interval whose refined result is strictly wider than the original and prove why equality cannot be promised. Re-evaluate the synthetic voltage expression at both precisions and separate grid widening from repeated-parameter widening. As a distinct analysis transfer, design a binary64 interface and identify rounding-mode, compiler contraction, nonfinite and signed-zero obligations; this fixed-grid implementation supplies no evidence for those behaviors.

## Security

The shared local synthetic scope applies. The two original arithmetic mutants demonstrate lost enclosure or a false lower bound only. No external service, equipment or real physical-control decision is involved. Treat a violated arithmetic precondition as a failed claim, not evidence of arbitrary code execution.

## Validation

Content is authored. The declared receipt records which reference executables, mutation controls and oracles actually ran. Learner acceptance, external review and physical measurements remain separate and unclaimed. Source inspection is bounded to the registered primary passages; the C adaptation and rational oracle are original constructions.
'''
(ROOT/'curriculum/v5/exercises'/f'{id}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf-8')
write('labs/numeric_interval/family.json',dict(version=1,family='numeric_interval',title='Outward real-interval enclosure on a fixed grid',build_system='cmake',assessor_root='assessor/numeric_interval',projects=[],units=[],modules=['T17'],tests=['numeric_interval.enclosure'],exercises=[dict(id=id,project='PR-T17-01',tests=['numeric_interval.enclosure'],capabilities=['linux-c17'],scope=meta['title'])],contract='labs/numeric_interval/CONTRACTS.md',fixture='fixtures/numeric_interval/cases.json',campaign=dict(producer='tools/run_numeric_interval.py',profiles=[c+'-'+p for c in ['gcc','clang'] for p in ['debug','release','asan']],assertion_marker='NI_ASSERT:',mutants_per_anchor=2,fixture_oracle='assessor/numeric_interval/oracle.py',oracle_driver='numeric-interval-driver')))
write('fixtures/numeric_interval/cases.json',dict(schema_version=1,synthetic=True,cases=[dict(operation='ratio',numerator=-1,denominator=3,bits=8,result=[-86,-85]),dict(operation='square',input=[-1,1],bits=0,result=[0,1]),dict(operation='independent-multiply',input=[-1,1],bits=0,result=[-1,1]),dict(operation='rescale',input=[-257,257],from_bits=16,to_bits=8,result=[-2,2])]))
write('research/v5/expansion/corpus_interval/taxonomy_refinement.json',dict(schema_version=1,omitted_subtopics=['Real-set interval enclosure','Outward fixed-grid rounding','Repeated-parameter dependency','Precision-changing enclosure'],rationale='The existing numerical formulation and boundary-rounding leaves genuinely cover these obligations, but scalar accumulation and Q7 projects did not implement interval enclosures. Add a required concrete exercise without changing accepted leaf text, IDs or historical hours.',objectives=objectives,exercise=id,not_claimed=['A complete interval standard','All floating-point environments','Physical uncertainty validation']))
print('PASS authored X-T17-003 and numeric_interval family')
