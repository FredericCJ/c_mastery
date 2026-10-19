"""Required event-driven parsing refinement, without changing historical budgets."""
import copy
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/corpus_events'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def dump(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
    locators=[['2.1 document root and proper nesting','3 and3.1 element type match; productions39/40/42/44','2.4 text/markup and2.11 line normalization comparison'],['ContentHandler event order and startElement/endElement','characters arbitrary segmentation and bounded array access','endDocument successful completion and fatal-error limitation']]
    resources=[]
    for i,(ident,title,url,authors,year,edition,file) in enumerate([
        ('R-CE-XML','Extensible Markup Language1.0 Fifth Edition','https://www.w3.org/TR/2008/REC-xml-20081126/',['Tim Bray','Jean Paoli','C. M. Sperberg-McQueen','Eve Maler','Francois Yergeau'],2008,'W3C Recommendation26November2008','xml.html'),
        ('R-CE-SAX','SAX ContentHandler API','https://docs.oracle.com/en/java/javase/25/docs/api/java.xml/org/xml/sax/ContentHandler.html',['Oracle','SAX project'],2025,'Java SE25 SAX2 API documentation','sax.html')]):
        resources.append(dict(id=ident,title=title,url=url,authors=authors,year=year,edition=edition,kind='specification' if i==0 else 'documentation',access='Public primary documentation',domains=['T15'],role='primary',reason='Nested grammar, ordered streaming events, segmentation and successful completion boundaries',prerequisites=['Explicit byte extents','Stack invariants','Callback lifetime'],assignable=True,locators=locators[i],inspection=dict(date='2026-10-18',method='Actual primary HTML passages read; private downloaded bytes retained',scope='; '.join(locators[i]),limits='Selected clauses only; original local grammar is neither XML conformance nor a SAX implementation',private_payload='build/corpus_events_sources/'+file,sha256=sha(ROOT/'build/corpus_events_sources'/file))))
    dump(ROOT/BASE/'resources.json',dict(resources=resources))
    meta=json.loads((ROOT/'curriculum/v5/exercises/X-T11-002.md').read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0])
    ident='X-T15-019';objectives=['T15.14.a','T15.14.b','T15.14.c']
    claims=[('T15.14.a','Derive a bounded incremental scanner and exact consumed-prefix/error locations across names, five fixed entities, partial tokens and overwritten consumed input.'),('T15.14.b','Implement nested start/text/end events, matching names, bounded depth and empty tags; retain exactly one unaccepted event through backpressure and reject reentry.'),('T15.14.c','Compose a copied two-event queue with an independent decimal-leaf consumer; prove segmentation invariance, checked accumulation and complete-document publication authority.'),('T15.14.c','Falsify four distinct defects, compare independent Expat events under changed chunks/schedules, and audit actual resource/helper costs without overclaiming XML conformance or mastery.')]
    meta.update(id=ident,title='Nested streaming events, backpressure and a separate semantic consumer',project='PR-T15-09',domain='T15',objectives=objectives,prerequisite_exercises=['X-T03-001','X-T04-001'],prerequisite_artifacts=[],lab=dict(family='corpus_events',scaffold='labs/corpus_events',tests=['corpus_events.'+a+str(b) for b in [16,1] for a in ['parser','consumer']]),fixtures=['fixtures/corpus_events/cases.json'],shared_contracts=['labs/corpus_events/CONTRACTS.md'],assessor_analysis='assessor/corpus_events/ANALYSIS.md')
    meta['environment']=dict(language='ISO C17; exact uint32_t/uint64_t; published exact ISO editions remain separate source gaps',target='Hosted Linux reference execution and strict Cortex-M4 object/link audit',toolchain=['GCC','Clang','CMake','Python3 with Expat','Arm GCC'],dependencies='No allocation, recursion, parser/string/container/memory library in target. Host Expat is an independent oracle only.')
    meta['source_records']=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in resources]
    meta['rubric']=[dict(id=ident+f'-R{i+1}',objective=o,criterion=c,levels={'0':'Breaks an event, extent, ownership or evidence contract.','1':'Reproduces ordinary fixture without invariants.','2':c+' Demonstrate the exact counterexamples and refusal boundaries.','3':'Independently reconstruct both changed conditions after at least7days and explain limitations.'}) for i,(o,c) in enumerate(claims)]
    meta['objective_coverage']=[dict(objective=o,teaching=['#problem','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in meta['rubric'] if r['objective']==o],assessed_claim='; '.join(r['criterion'] for r in meta['rubric'] if r['objective']==o)) for o in objectives]
    meta['status']['limitations']=['Original bounded ASCII language; no XML/SAX conformance, external entity resolution or whole-document rollback.','Finite author executions do not establish independent delayed learner assessment or physical timing.']
    body='''# X-T15-019: Nested streaming events and a separate semantic consumer

## Prerequisites
Complete X-T03-001 cursor/prefix ownership and X-T04-001 checked extents. Supply the learner consumed-prefix invariant, finite stack representation and checked decimal accumulation argument. Study PR-T15-09 scanner/parser separation. No hidden assessor dependency is imported.

## Environment
Use C17 Profile C under CONTRACTS.md. The parser owns its bounded partial state; it borrows caller input only for the synchronous call. Host Expat is verification only. No studied mechanism may be delegated to a parsing, allocation, string-conversion or container library.

## Problem
Implement every public function in src/events.c for the exact grammar and callback contract. Inputs are arbitrary byte chunks plus explicit EOF/cancel and callback outcomes. Outputs are consumed lengths, terminal statuses and ordered START/CHARACTERS/END/DOCUMENT events. Build the two-event copied queue and separate EvSum semantic consumer for nested groups of decimal n leaves. For `<sum><g><n>12</n></g><n>30</n></sum>`, publish42 only after successful DOCUMENT. All capacities, sequences, ownership rules and failure semantics are explicit in the shared contract.

## Necessity
A binary frame parser does not assess matched nested names or event lifetime. A tree parser does not assess streaming backpressure. One TEXT callback is not necessarily one semantic value; WAIT is not permission to drop an event or replay consumed bytes. Constant document size in a fixture does not prove constant memory for arbitrary nesting.

## Reasoning
Derive scanner state, bounded name stack and the pending-delivery invariant. Distinguish parsed structural progress from accepted callback progress, consumed bytes from delivered events, and partial semantic sums from complete-document authority. Prove exact no-wrap sequence rules and checked decimal arithmetic. Identify which guarantees follow from ISO C, runtime checks, source proofs and caller discipline. Compare SAX ordered events/segmentation and Java exceptions with this C callback/status/lifetime contract, and give counterexamples to XML equivalence.

## Deliverables
D1 grammar, state diagram, pre/postconditions, prefix proofs and plausible incorrect explanations. D2 parser, copied queue, independent consumer and a streaming driver that overwrites consumed input safely. D3 all split positions, paused/rejected/reentrant callbacks, bounds, malformed EOF and independent event comparisons plus causal controls. D4 both changed implementations, target closure and resource/evidence ledger.

## Verification
Run four public anchors and eight compiled controls in six C17 profiles. Compare1024 seeded nested documents per text capacity with independent Expat after coalescing consecutive text at equal depth. Check every split of the supplied mixed/empty-tag sample, pending input overwrite, depth/name bounds,10000 streamed text octets, counter exhaustion, all terminal retries and unchanged consumer/queue refusal. Strict images require no implicit helper/library. Translation or sanitizer failure is not a semantic mutant rejection.

## Rubric
Assess R1-R4 separately. Ordinary fixture reproduction is level1; level2 requires invariant derivation, causal diagnosis, changed conditions and exact evidence limits. Level3 requires independent delayed reconstruction, never inferred from the assessor run.

## Hints
H1 track the state before and after consuming '<'. H2 pending event ownership outlives a WAIT callback but its borrowed pointer does not. H3 make semantic publication depend on DOCUMENT, not queue emptiness or root END alone.

## Variants
V1 changes text capacity16 to1 and feed boundaries without changing the logical document. Reprove segmentation invariance and compare actual event/storage costs, not just constants.

V2 separates parsing from semantic work using two copied event slots; defer drains, retry WAIT, overwrite consumed input, cancel with a pending event, and inject malformed/overflowing numeric text. Prove exactly which partial effects remain and when the final sum may be published.

## Security
Use original local synthetic strings and callback controls only. The attacker supplies malformed nesting/entities, excessive lengths/depth, truncation or numeric overflow. Harmless success is a wrong synthetic sum, lost event or bounded rejection. No external entity resolver or network target exists. Repair the violated invariant, retain regression controls, and explain that bounded parsing is not full XML security or protection against arbitrary callback effects.

## Validation
Current source-bound reference campaign is reviews/v5/expansion/corpus_events/current_validation.json. Actual author execution, target translation/link checks, external assessment and physical evidence are distinct. SAX project direct-host retrieval timed out; the actual Oracle primary API mirror was read and registered. W3C selected clauses were inspected. Exact published ISO copies and arbitrary XML conformance remain outside the evidence. Finite tests do not prove absolute mastery.
'''
    (ROOT/'curriculum/v5/exercises'/f'{ident}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,kind='required-competency-refinement-of-existing-leaf',scope_rationale='Finite84 event-driven-parsing requires nested incremental events and separate consumer, beyond binary framing or ordinary expression trees.',refinements=[dict(exercise=ident,required=True,parent_objectives=objectives,competencies=[dict(id=r['id'],outcome=r['criterion'],source=[x['id'] for x in resources]) for r in meta['rubric']])]))
    pins=[BASE+'/resources.json',BASE+'/taxonomy_extensions.json'];dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in pins},limitations='Selected author primary inspection only; no XML/SAX conformance or independent assessment claim.'))
    dump(ROOT/BASE/'audit.json',dict(status='confirmed-finite84-event-driven-parsing-gap-implemented',original_corpus_id='event-driven-parsing',inspected_inputs=['curriculum/projects/PR-T15-09.md','research/v5/expansion/corpus_followthrough/screening_map.json'],distinction='Incremental nested event stream, retained pending event, copied queue and separate consumer; binaryframes are not substituted.',reference_gaps=['Direct saxproject API host timed out; actual Oracle primary API mirror inspected','Exact published ISO C17/C23'],limits='Declared bounded ASCII grammar, not XML conformance or constant memory independent of configured depth.'))
    dump(ROOT/'fixtures/corpus_events/cases.json',dict(document='<a>x&amp;y<b/>z</a>',coalesced_events=[[1,1,'61'],[2,1,'782679'],[1,2,'62'],[3,2,'62'],[2,1,'7a'],[3,1,'61'],[4,0,'']],semantic_document='<sum><g><n>12</n></g><n>30</n></sum>',semantic_sum=42))
    dump(ROOT/'labs/corpus_events/family.json',dict(version=1,family='corpus_events',title='Nested streaming events and consumer composition',build_system='cmake',assessor_root='assessor/corpus_events',dependencies=[],projects=[],units=[],exercises=[dict(id=ident,project='PR-T15-09',tests=meta['lab']['tests'])],tests=meta['lab']['tests'],campaign=dict(producer='tools/run_corpus_events.py',receipt='corpus_events_campaign.json',profiles=[c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan']],mutants_per_anchor=2,assertion_marker='EV_CHECK:',fixture_oracle='tools/verify_corpus_events.py',oracle_driver='events16')))
    print('Authored required X-T15-019 event-driven parsing refinement')
if __name__=='__main__':main()
