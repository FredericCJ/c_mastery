"""Canonical required SECDED competency refinement and exercise, no baseline rewrite."""
import hashlib
import json
import pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_ecc'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(path,value):
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')
def main():
    validation='reviews/v5/expansion/corpus_ecc/current_validation.json'
    report_path=ROOT/validation
    observed=json.loads(report_path.read_text(encoding='utf8')) if report_path.is_file() else {}
    executed=observed.get('passed') is True and bool(observed.get('inspected_paths')) and all((ROOT/p).is_file() and sha(ROOT/p)==h for p,h in observed.get('inspected_paths',{}).items())
    paper=['Section3, printed150-153: checking number, power-of-two check positions and seven-position construction',
           'Section4, printed153-154: overall even parity and0/1/2-error cases',
           'Section5, printed154-155: distance and TableV detection/correction bounds']
    contract=['Representation and operations','Derivation, limits and adversarial behavior','Costs and deliverables','Verification and assessment']
    records=[dict(id='R-CORPUS-ECC-HAMMING1950',title='Error Detecting and Error Correcting Codes',authors=['Richard W. Hamming'],year=1950,edition='Bell System Technical Journal29(2),147-160,April1950',kind='paper',url='https://zoo.cs.yale.edu/classes/cs323/doc/Hamming.pdf',access='Public primary paper, local bytes inspected',domains=['T15'],role='primary',reason='Derive the two selected extended Hamming constructions and bounded distance guarantee.',prerequisites=['T04 unsigned bit arithmetic','T15 loop invariants'],assignable=True,locators=paper,inspection=dict(date='2026-10-18',method='Read actual downloaded PDF selected extracted pages and rendered printed150/154; not a title-only citation.',scope='; '.join(paper),limits='Bounded sections only; erasure/interleave adapters are original derived work, not claimed paper implementations or channel observations.',download_path='build/corpus_codec_sources/hamming.pdf',download_sha256=sha(ROOT/'build/corpus_codec_sources/hamming.pdf'))),
             dict(id='R-CORPUS-ECC-CONTRACT',title='Original bounded SECDED and burst/erasure contract',authors=['Curriculum authors'],year=2026,edition='Original revision1,2026-10-18',kind='manual',url='labs/corpus_ecc/CONTRACTS.md',access='Local original contract inspected',domains=['T15'],role='primary',reason='Exact representation, failure publication and original transfer/security limits.',prerequisites=['PR-T04-01','PR-T15-01'],assignable=True,locators=contract,inspection=dict(date='2026-10-18',method='Read original contract and implementation source.',scope='; '.join(contract),limits='Author specification and source inspection; actual execution only in separate receipts.',local_sha256=sha(ROOT/'labs/corpus_ecc/CONTRACTS.md')))]
    dump(ROOT/BASE/'resources.json',dict(resources=records))
    rubric=[]
    rows=[('T15.15.a','Representation, parity derivation and code distance','Derive check positions, injectivity, minimum distance4 and both payload widths.'),
          ('T15.15.a','Bounded decoder and failure publication','Correct every admitted single flip, detect double flips, preserve refusal output and reject unsupported bounds.'),
          ('T15.15.c','Erasure recovery and independent nearest-codeword oracle','Derive2t+s<4, construct the enumerative recovery, compare independent codebooks and explain finite coverage.'),
          ('T15.15.b','Burst interleaving, phase and costs','Implement four-lane and rotating-phase maps, derive burst bounds and compare rate/work/buffering under equal guarantees.'),
          ('T15.15.a','Higher-error and attacker counterexamples','Demonstrate miscorrection, valid-word forgery and wrong-phase identity; separate noise correction from authenticity and real channel behavior.')]
    for i,(objective,title,claim) in enumerate(rows,1):
        rubric.append(dict(id=f'X-T15-008-R{i}',objective=objective,criterion=title+': '+claim,levels={'0':'Violates the stated bound or supplies an unsupported guarantee.','1':'Obtains an ordinary result without the mechanism or counterexample argument.','2':claim+' Supplies exact source, refusal/control and verification evidence.','3':'Independently derives and verifies the changed erasure/phase assumption after the recorded delay.'}))
    data=dict(schema_version=1,id='X-T15-008',title='Derive and falsify bounded error-correcting codes',project='PR-T15-10',domain='T15',objectives=['T15.15.a','T15.15.b','T15.15.c'],families=list('ABCDEFHIJ'),required=True,prerequisite_exercises=['X-T04-001'],prerequisite_artifacts=[],profile='C',environment=dict(language='ISO C17 implementation; exact final C17/C23 source access remains distinct from mathematical coding proof',target='Hosted Linux x86-64; strict Cortex-M4 object/link audit only',toolchain=['GCC','Clang','CMake','Python','Arm GCC'],dependencies='Original arithmetic/bit encoder, decoder and interleave; hosted independent oracle separate from target runtime'),source_records=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in records],lab=dict(family='corpus_ecc',scaffold='labs/corpus_ecc',tests=['corpus_ecc.8','corpus_ecc.16','corpus_ecc.erasures','corpus_ecc.burst']),fixtures=['fixtures/corpus_ecc/cases.json'],shared_contracts=['labs/corpus_ecc/CONTRACTS.md'],assessor_analysis='assessor/corpus_ecc/ANALYSIS.md',status=dict(content='authored',execution='pending-source-bound-campaign',independent_review='not-run',learner_assessment='not-assessed',limitations=['WSL author campaign pending; no target execution or physical channel evidence.','Finite tests and mathematical model premises are not absolute mastery.','No cryptographic authenticity, insertion/deletion correction or unbounded burst guarantee.']),objective_coverage=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in rubric if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in rubric if r['objective']==o)) for o in ['T15.15.a','T15.15.b','T15.15.c']],rubric=rubric,delayed_transfer=dict(minimum_days=7,variant_ids=['V1','V2']))
    body='''# X-T15-008: Derive and falsify bounded error-correcting codes

## Prerequisites
Bring X-T04-001 checked-arithmetic reasoning, PR-T04-01 bit operations, PR-T15-01 loop invariants and your own code-distance argument. The component imports no prior algorithm library. The historical PR-T15-10 elective label does not make this required expansion optional.

## Environment
Use C17 and the explicit octet/uint16 profile in the shared contract. Host-side Python codebook enumeration is independent verification. Cortex-M4 object/link audits establish dependencies only; no physical memory or communications target is used.

## Problem
Implement extended Hamming(8,4) and(16,11), bounded syndrome decode, known erasures, four-lane interleaving and rotating-phase transfer. The public header and shared contract define all bit positions, widths, statuses, immutable inputs, unchanged refused outputs and exact workloads. Start with payload1 encoded as135 in the eight-bit convention; independently derive it before viewing expected fixtures.

## Necessity
The corpus exposes coding theory beyond a checksum mention. Correcting a single flipped bit requires a representation and distance argument; blindly applying that decoder to two or more errors can corrupt data. This required refinement assesses those mechanisms without rewriting the retained generic advanced-algorithm leaf or historical acceptance.

## Reasoning
Derive parity equations, linearity and minimum distance4. Prove the0/1/2-error result and2t+s<4 uniqueness bound. Account for all output publication, backing lifetime and fixed-width shift bounds. Explain why a valid codeword can be forged and why interleaving requires known frame phase. Compare rate, work and buffering under equivalent error assumptions.

## Deliverables
D1: exact representation/contract and derivations. D2: all seven API implementations and independent codebook dossier. D3: complete bounded flip matrix, erasures/burst/phase cases, refusal-output evidence, compiled controls, native/target audit and counterexamples. D4: independently reconstructed V1/V2 after at least7days. The shared contract gives concrete inputs, outputs and costs for each item.

## Verification
Run the four named public tests and independent assessor protocol. All592 small and280576 changed-profile0/1/2-error cases must pass; the small nearest-codeword oracle covers all256 received values. Explicitly distinguish finite erasure sampling from the uniqueness proof. Stubs must compile and fail; each anchor has two distinct compiled controls. Compiler/target assumptions and exact commands must be recorded.

## Rubric
Score R1 derivation, R2 decoder/publication, R3 erasure/oracle, R4 interleave/cost and R5 limits separately. Every row needs its own mechanism and counterexample. The metadata and shared contract give0-3 levels; a passing reference campaign is not an independent learner attempt.

## Hints
H1 number positions in binary. H2 keep total parity separate from the XOR checking number. H3 ask which two codewords could both fit the stated error/erasure budget. Preserve predictions before reading assessor analysis.

## Variants
V1 changes the code dimension to16/11 and combines one known erasure with one unknown flip; rebuild the guarantee and cost argument. V2 rotates lane phase, changes frame-alignment assumptions and constructs a wrong-owner result that still passes bit decoding. The actual phase adapter is supplied as unsolved learner API and assessed separately after at least7days.

## Security
Only original local arrays and synthetic payloads. The attacker can replace a codeword; encode payload9 and show valid decoding despite missing authority. Three/four-flip and wrong-phase cases are harmless property violations. Preserve the repaired limited guarantee and tests; range checks do not supply general authentication. No external target, real credentials or exploit is involved.

## Validation
Authored source, independent oracle, controls and target audit protocol exist. Execution receipts remain pending while WSL startup is unavailable; no passing observations are invented. Exact published C editions and physical channel behavior are not inferred from the inspected Hamming paper or compiler flags. External review and independent mastery remain separate.
'''
    if executed:
        data['status']['execution']='see-source-bound-campaign'
        data['status']['campaign_receipt']=validation
        data['status']['limitations'][0]='Six native C17 profiles and four strict target object/link audits passed; no target execution or physical channel evidence.'
        body=body.replace('Execution receipts remain pending while WSL startup is unavailable; no passing observations are invented.',
                          'The source-bound author campaign passed six C17 host profiles and four strict target object/link audits; its exact commands, source pins and limitations are in '+validation+'. WSL interruption is recorded separately from passing observations.')
    p=ROOT/'curriculum/v5/exercises/X-T15-008.md';p.write_text('```json\n'+json.dumps(data,indent=2)+'\n```\n\n'+body,encoding='utf8')
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',parent_objectives=['T15.15.a','T15.15.b','T15.15.c'],exercise='X-T15-008',scope_rationale='The generic specialized-algorithm leaf did not supply the corpus error-correction mechanism. This required five-competency refinement adds full representations, bounds and counterexamples, not an optional heading; accepted IDs/hours remain unchanged.',competencies=[dict(id=r['id'].replace('X-T15-008','ECC'),title=rows[i][1],outcome=rows[i][2],rubric=r['id'],source=['R-CORPUS-ECC-HAMMING1950','R-CORPUS-ECC-CONTRACT'],prerequisites=['X-T04-001','PR-T04-01','PR-T15-01']) for i,r in enumerate(rubric)]))
    paths=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json']
    dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',resources=BASE+'/resources.json',research_input_sha256={p:sha(ROOT/p) for p in paths},inspection=dict(date='2026-10-18',method='Actual selected paper pages plus authored mechanism review',scope='Only exact registered passages and original required competency refinement',limits='Author source inspection only. No execution or independent review is inferred from source inspection; broader implementation pins are in the completion packet.')))
    print('Authored X-T15-008, five assessed refinements; execution status from separate campaign')
if __name__=='__main__':main()
