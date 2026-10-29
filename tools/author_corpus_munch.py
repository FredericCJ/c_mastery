"""Author the additive maximal-munch refinement; preserve historical records."""
import json
from common import ROOT,dump,sha

def put(path,value):
    p=ROOT/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(value,encoding='utf-8',newline='\n')

def main():
    family='corpus_munch';ident='X-T15-020';folder='research/v5/expansion/'+family
    f=json.loads((ROOT/'labs/corpus_munch/family.json').read_text());f['exercises'][0]['project']='PR-T15-09';dump(ROOT/'labs/corpus_munch/family.json',f)
    source=(ROOT/'curriculum/v5/exercises/X-T15-018.md').read_text()
    meta=json.loads(source.split('```json\n',1)[1].split('\n```',1)[0])
    meta['id']=ident;meta['title']='Maximal-munch rollback and memoized failed configurations';meta['prerequisite_exercises']=['X-T15-010']
    meta['environment']['dependencies']='Original bounded scanner and failed-configuration table; actual prior regex DFA adapter; hosted independent regex enumeration is verification-only'
    passages=['Author-hosted manuscript pp1-3: longest-prefix/earliest-rule selection and quadratic abc/(abc)*d example','pp4-7 and Figure1: failed-configuration tabulation and fixed-automaton linear bound']
    meta['source_records']=[dict(id='R-CMUNCH-REPS',path=folder+'/resources.json',passages=passages)]
    meta['lab']=dict(family=family,scaffold='labs/'+family,tests=[family+'.'+x for x in ['semantics','bounds','memo']])
    meta['assessor_analysis']='assessor/corpus_munch/ANALYSIS.md'
    meta['status']['limitations']=['Bounded byte-class DFA, fixed input per call and original memoization representation.','Core cross-build evidence is not target execution; delayed learner evidence remains separate.']
    meta['status']['campaign_receipt']='reviews/v5/expansion/corpus_munch/current_validation.json'
    meta['fixtures']=['fixtures/corpus_munch/cases.json'];meta['shared_contracts']=['labs/corpus_munch/CONTRACTS.md'];meta['delayed_transfer']['variant_ids']=[ident+'-DELAYED']
    criteria=['Derive longest-prefix and earliest-rule semantics, a quadratic rollback witness and a sound failed-configuration invariant.','Implement bounded whole-call output rollback, exact metadata/capacity failures and actual prior regex-DFA composition with explicit ownership.','Falsify independent scanner/memo defects, account for state-dependent resource costs and runtime closure, and transfer under changed input or accepting labels.']
    for i,(r,cov) in enumerate(zip(meta['rubric'],meta['objective_coverage'])):
        r['id']=f'{ident}-R{i+1}';r['criterion']=criteria[i];cov['independent_assessment']=[r['id']];cov['assessed_claim']=criteria[i];cov['scope']='Required scanner rollback and memoization refinement; regex construction retains separate assessment.'
    body='''

# X-T15-020: Maximal-munch rollback and memoized failed configurations

## Prerequisites

Complete X-T15-010 and retain its actual NFA, subset-construction and struct rx_dfa artifacts. Explain finite-state recognition, ordered rules, array bounds, exclusive byte spans and whole-call publication. The refusing learner scaffold intentionally needs implementation.

## Environment

Use the C17/eight-bit-byte environment and original-runtime restrictions in [CONTRACTS](../../../labs/corpus_munch/CONTRACTS.md). Import the actual prior learner workspace explicitly; author validation selects a separately identified private provider. Hosted regex, Python, stdio and clock are verification tools. The scanner mechanism uses no library recognition or allocation.

## Problem

Implement both modes of mm_scan and the real mm_from_rx adapter. The complete bounded inputs, outputs, representations, status precedence and ownership rules are in the shared contract. First construct longest-prefix scanning and rule-priority selection. Then preserve both outputs on a late recognition/capacity failure. Finally tabulate failed state-position configurations to avoid repeated fruitless scans while keeping exactly the same output contract. The union DFA and abc/(abc)*d witnesses are supplied in dfa_cases.h; independent expected tokens must also be derived.

## Necessity

A prior whole-input DFA recognizer does not implement token selection and rollback. A deterministic automaton may still scan quadratically when the scanner repeatedly pursues a longer alternative. Stopping at the first acceptance or giving rule priority precedence over token length changes the language interface; retaining memo entries across changed input invalidates the optimization's proof.

## Reasoning

State the current-state, last-acceptance, token partition and failed-configuration invariants. Derive the 3k(k+1)/2 transition witness and distinguish fixed-automaton linear input dependence from state-dependent bounds and table initialization. Prove why marking cannot cross an acceptance. Establish progress without empty tokens, failure publication and scratch lifetime. Compare explicit C checks with language ownership and lexer-generator contracts, giving counterexamples to overstated equivalence.

## Deliverables

- D1: hand token traces, priority and rollback derivations, progress/termination proof and failed-pair argument.
- D2: original scanner/adapter, explicit prerequisite import, complete storage/ownership and failure contracts.
- D3: independent token oracle, boundary/late-failure/cache-invalidation regressions, compiling defect controls, generated dependency audits and equal-contract counter/clock measurements.
- D4: independently assessed changed-condition construction and the separately released delayed memoization variant, with hint disclosure.

## Verification

Configure CMake with the actual prerequisite root, build and run CTest. The three public anchors must reject the compiling learner scaffold for semantic reasons. tools/run_corpus_munch.py executes six compiler profiles, the independent candidate-end oracle, actual previous DFA import, counter/clock measurements and separate target-core audits. tools/check_corpus_munch.py --self-test must reject incomplete or overstated evidence. Pass requires exact spans/rules/statuses, unchanged outputs on late failure, nullable/malformed model rejection, sound table lifetime and fewer transitions on the proved amplification witness. Finite strings, sanitizer success and a successful link establish only their stated claims.

## Rubric

R1 assesses derivation and semantics; R2 construction, actual composition and failure/ownership invariants; R3 independent falsification, costs, dependencies and transfer. The machine contract maps each criterion to its own leaf objective and levels0–3. Supplied-example completion is level1; criterion-specific reasoning and discriminating evidence are required for level2; level3 requires independent changed-condition work. No finite collection of passes establishes absolute mastery.

## Hints

First separate an accepting candidate from the selected token. Next retain the last accepting position while exploring later bytes. Then distinguish failure of a state from failure of a state at a position for one fixed suffix. Finally identify the accepting boundary that a backwards failure proof cannot cross. Record used hints; full code and analysis remain private.

## Variants

Change rule order, admitting alphabets or accepting labels; expose which table entries need invalidation. X-T15-020-DELAYED, after at least seven days, asks for the memoized variant from the baseline interface, an unfamiliar lexicon, actual prior learner artifact import and equal-contract cost analysis. Its author reference is implemented today; an independent delayed learner attempt remains pending.

## Security

The only targets are original local scanners and synthetic inputs, bounded by the shared contract. Show an incorrect token or repeated-work counter increase as the harmless success condition of an original defect. Repair last acceptance, rule ordering or memo lifetime and retain regressions. Valid input objects remain a precondition; neither a crash nor a sanitizer observation demonstrates code execution or a deployed vulnerability.

## Validation

Authored required refinement with exact source-bound execution, oracle, mutation, export and core-link evidence recorded separately. Retain raw commands, bounds/seed and source identities. Non-author review, target execution/physical observations and independent delayed assessments remain distinct. Historical regex evidence and accepted hour records remain unchanged.
'''
    put('curriculum/v5/exercises/'+ident+'.md','```json\n'+json.dumps(meta,indent=2)+'\n```'+body)
    resource=dict(id='R-CMUNCH-REPS',title='Maximal-Munch Tokenization in Linear Time',authors=['Thomas Reps'],edition='Author-hosted manuscript of 1998 article; manuscript page numbering',year=1998,kind='paper',url='https://research.cs.wisc.edu/wpis/papers/toplas98b.pdf',access='Actual selected primary PDF text and Figure1 inspected',domains=['T15'],role='primary',reason='Precise longest-prefix selection, quadratic witness and failed-configuration tabulation',prerequisites=['finite automata','amortized analysis'],assignable=True,locators=passages,inspection=dict(date='2026-10-18',method='Selected PDF pages1-7 text read; page5 Figure1 visually inspected',scope='Listed pages only, not whole article',limits='Author-hosted manuscript is not claimed to be the publisher facsimile. Bounded API, transaction and byte-table representation are original.',download_path='build/corpus-lexmodel-sources/reps.pdf',download_sha256='f2ed52f524b129a5c7ab7c5dd218d4dbca5aa210a7781756c6d880b09f7e0478'))
    dump(ROOT/folder/'resources.json',dict(resources=[resource]))
    dump(ROOT/folder/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',scope_rationale='Finite corpus maximal-munch tokenization was not implemented by the prior regular-language recognizer or decimal lexer; selection, rescanning and memo lifetime require separate construction.',refinements=[dict(exercise=ident,mechanism='longest-prefix rollback and failed-configuration memoization',parent_objectives=meta['objectives'],outcomes=criteria,prerequisites=meta['prerequisite_exercises'],source_ids=[resource['id']])],preservation='Additive required refinement; historical accepted records unchanged.'))
    paths=[folder+'/'+n for n in ['resources.json','taxonomy_extensions.json']]
    dump(ROOT/folder/'research_gate.json',dict(status='passage-inspected',resources=paths[0],research_input_sha256={p:sha(ROOT/p) for p in paths},inspection=dict(date='2026-10-18',method='Actual selected Reps primary passages and Figure1',scope='Registered locators only',limits='No full-paper inspection, independent review, learner or board claim.')))
    dump(ROOT/'fixtures/corpus_munch/cases.json',dict(exercise=ident,rules=['abc','(abc)*d','abc'],cases=[dict(input='abcabcd',tokens=[[0,7,1]]),dict(input='abcabc',tokens=[[0,3,0],[3,6,0]],baseline_transitions=9,baseline_rollback=3),dict(input='dabc',tokens=[[0,1,1],[1,4,0]]),dict(input='abcx',status='PD_NO_MATCH',error_offset=3,outputs='unchanged'),dict(input='abcabc',capacity=1,status='PD_RANGE',error_offset=3,outputs='unchanged')],cost_witness=dict(input='abc repeated85',length=255,baseline_transitions=10965,memo_transition_bound_exclusive=765),scope='Original bounded byte scanner, independent expectations; not arbitrary lexer performance.'))
    print('Authored',ident)
if __name__=='__main__':main()
