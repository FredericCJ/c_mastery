"""Canonical exact paired textual mechanism and actual dialect composition."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def write(name,value):
 p=ROOT/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8')
ident='X-T15-022'
locators=['Section2.2 Characters: Char production and character domain','Section2.4 Character Data and Markup: literal delimiters versus entity references','Section4.6 Predefined Entities: amp,lt,gt,apos,quot and character-data interpretation']
resource=dict(id='R-CESC-XML08',title='Extensible Markup Language (XML)1.0 Fifth Edition',authors=['W3C XML Core Working Group'],edition='W3C Recommendation26November2008',year=2008,kind='specification',url='https://www.w3.org/TR/2008/REC-xml-20081126/',access='public',domains=['T15'],role='reference',reason='Ground the character-data/markup distinction and named-reference interpretation; original narrower contract is explicit.',prerequisites=['Explicit byte lengths','Finite grammars'],locators=locators,assignable=True,inspection=dict(status='passage-inspected',date='2026-10-18',method='Direct read of official rendered primary publication',scope='; '.join(locators),limits='Only these sections; original ASCII subset rejects numeric references and is not an XML or HTML processor. No immutable web hash claimed.'))
write('research/v5/expansion/corpus_escape/resources.json',dict(resources=[resource]))
criteria=[('T15.14.c','Derive the paired finite grammar, consume-once invariant and inverse law; distinguish character data from syntax and show failures of repeated decoding or context-independent safety.'),('T15.13.c','Implement exact boundary/error ordering and unchanged-output publication for malformed prefixes, all admitted byte values, expansion limits and every relevant capacity cut.'),('T15.15.c','Compose the actual template and event-parser artifacts, diagnose numeric-versus-named apostrophe incompatibility, implement the explicit adapter and verify changed event chunking with independent models and causal mutants.')]
rubric=[];coverage=[]
for n,(objective,criterion) in enumerate(criteria,1):
 rid=ident+'-R'+str(n);rubric.append(dict(id=rid,objective=objective,criterion=criterion,levels={'0':'Missing or incorrect grammar, failure semantics or context model.','1':'Ordinary replacement examples work but bounds, inverse or composition reasoning is absent.','2':criterion+' Provide source-bound implementation and exact positive/negative evidence.','3':'Predict an unfamiliar dialect/boundary interaction, minimize a counterexample, transfer independently and explain what each check cannot establish.'}));coverage.append(dict(objective=objective,teaching=['#problem','#necessity','#reasoning'],implementation=['D1','D2'],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[rid],scope='This exact textual pair/context adapter, not the entire parent leaf.'))
meta=dict(schema_version=1,id=ident,title='Text escaping, single-pass inverse and an incompatible consumer dialect',project='PR-T15-09',domain='T15',objectives=[x[0] for x in criteria],families=['A','B','C','D','E','F','G','H','I','J'],required=True,prerequisite_exercises=['X-T15-016','X-T15-019'],prerequisite_artifacts=[],profile='C',environment=dict(language='C17; eight-bit octets and explicit lengths',target='Original local bounded text components, hosted verification',toolchain=['GCC','Clang','CMake','Python3'],dependencies='Original first-principles escape core; actual completed text_composition and corpus_events artifacts are composed explicitly; Python/table oracle and test I/O are host-only'),source_records=[dict(id=resource['id'],path='research/v5/expansion/corpus_escape/resources.json',passages=locators)],lab=dict(family='corpus_escape',scaffold='labs/corpus_escape',tests=['corpus_escape.escape']),assessor_analysis='assessor/corpus_escape/ANALYSIS.md',status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',limitations=['Finite ASCII subset, not general HTML/XML or Unicode conformance.','No browser execution, strict final-target or physical observation.','Author execution is not delayed learner mastery.'],campaign_receipt='reviews/v5/expansion/corpus_escape/corpus_escape_campaign.json',learner_assessment='not-assessed'),fixtures=['labs/corpus_escape/'+n for n in ['escape.h','tests.c','driver.c','composition.c','CMakeLists.txt','reject.py']]+['fixtures/corpus_escape/cases.json'],shared_contracts=['labs/corpus_escape/CONTRACTS.md'],delayed_transfer=dict(minimum_days=7,variant_ids=[ident+'-DELAYED']),objective_coverage=coverage,rubric=rubric)
body='''# X-T15-022: Text escaping, single-pass inverse and an incompatible consumer dialect

## Prerequisites

Know explicit-length byte arrays, bounded scanning, failure atomicity and context-sensitive interpretation. Bring completed X-T15-016 template and X-T15-019 events artifacts for the composition; the paired escape implementation must be original. Read their public contracts and compare actual apostrophe spellings before changing any code.

## Environment

The explicit [version1 shared contract](../../../labs/corpus_escape/CONTRACTS.md) supplies every representation, operation, status, dependency, ownership premise and cost bound. Implement src/escape.c. Core Profile C has no parser, string conversion, allocator or escaping library. Actual prerequisite components are separate completed learner artifacts; Python and test I/O are host-side verification. Record C compiler/optimization selections and do not infer strict final-image closure from host execution.

## Problem

Implement the encoder, once-only decoder and generated-HTML-text dialect adapter for64 plain/384 encoded octets. Encode `&<>"'`, derive the exact25-octet result, then decode it. Predict decoding `&amp;lt;x&amp;gt;` and every incomplete prefix of the five entities. Fail every insufficient capacity without changing any output byte. Distinguish an actual apostrophe from literal `&#39;` in a template model. Render the real `{a}` template, expose the original event parser's rejection, then adapt the dialect and feed a single wrapped root at every split point.

## Necessity

A one-way encoder does not establish its inverse, and a binary framing escape is a different grammar. Repeated decoding can turn previously literal entity-looking data into syntax. Two correct components can disagree about a permitted entity spelling. Context-specific byte replacement is not universal sanitization, authentication or authorization.

## Reasoning

State preconditions, postconditions, the consumed-prefix invariant, exact error precedence and publication point. Prove the round-trip law only on the admitted domain and provide counterexamples to idempotence, repeated-decoding safety and arbitrary-HTML equivalence. Account for6n expansion, two stages in recoding, copied callback data and the absence of retained borrows. Separate ISO C guarantees, compiler diagnostics, runtime bounds, external finite-model evidence and caller discipline. Explain why emitted '<' is character data and why an adapter validating bytes does not establish their provenance.

## Deliverables

- D1: Grammar/inverse derivation, incorrect-claim counterexamples and precise source comparison.
- D2: All three original C operations, publication/ownership ledger, explicit actual-component adapter and storage/work bounds.
- D3: Independent model, malformed/entity-prefix/capacity tests, two causal mutants, actual dialect-rejection observation and successful composition at all split points with event chunks16 and1.
- D4: Named delayed transfer predictions, changed-condition implementation and compiler/environment comparison with bounded conclusions.

## Verification

The five special bytes encode to25 bytes and decode to the original five. `&amp;lt;x&amp;gt;` decodes to nine literal bytes `&lt;x&gt;`. A failed capacity or syntax check preserves the complete EsText object. The actual template emits numeric apostrophe39; the strict inverse and events parser reject that spelling, while the adapter produces named apos. The final event ledger has exactly one START, one END, one DOCUMENT and exactly the original text. Compare12994 finite cases with the independent table-language model. Run six C profiles, two harmless compiled mutants and both actual composition builds. The compiling scaffold must fail corpus_escape.escape at ES_ASSERT. Tests do not replace the derivation or independent assessment.

## Rubric

Assess all three metadata criteria separately: paired grammar and inverse, failure/boundary implementation, and actual context composition. Level2 requires every specified mechanism with evidence; level3 requires independent changed-condition prediction and repair. No finite test suite proves absolute mastery, and author execution does not satisfy the seven-day learner gate.

## Hints

1. Mark source bytes consumed separately from bytes appended to the output.
2. Validate an entire entity against the remaining input length before appending its one decoded byte.
3. Compare the actual two producers' apostrophe spellings. Decode a literal ampersand-number sequence once before deciding whether it is syntax.

## Variants

X-T15-022-DELAYED: after at least seven days, move the named encoder into the actual template-to-event pipeline. Predict the failed apostrophe path before execution; implement the generated-HTML-text adapter without modifying either prerequisite. Preserve literal `&#39;` and escaped tag-like text. Change EV_TEXT16 to1, enumerate every two-part split and defend the same copied-event contract. Explain which parts fail if the destination becomes an HTML attribute, script or URL context; no new sanitizer is implied.

## Security

Use only the original local teaching components and synthetic bytes. The attacker controls text, malformed entities, capacities and chunk boundaries. A harmless demonstrated violation is extra structure, wrong copied character data or output mutation on rejection. Reproduce and repair both unescaped-ampersand and double-decoding defects, retain regressions and name downstream-context limitations. No external target or browser exploitation is involved; a parse failure is not code execution.

## Validation

Use the exact named source-bound campaign and isolated export receipts. Actual primary inspection is limited to the registered W3C sections; no full standard or XML conformance is claimed. Compiler profiles, finite independent oracle, actual retained-component composition, external review, learner delay and physical observations are separate dimensions.
'''
(ROOT/'curriculum/v5/exercises'/f'{ident}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
write('labs/corpus_escape/family.json',dict(version=1,family='corpus_escape',title='Text escape pair and actual dialect composition',build_system='cmake',assessor_root='assessor/corpus_escape',projects=[],units=[],modules=['T15'],dependencies=['text_composition','corpus_events'],tests=['corpus_escape.escape'],exercises=[dict(id=ident,project='PR-T15-09',tests=['corpus_escape.escape'],capabilities=['linux-c17'],scope='Paired finite grammar, unchanged failures and actual dialect adapter')],contract='labs/corpus_escape/CONTRACTS.md',fixture='fixtures/corpus_escape/cases.json',campaign=dict(producer='tools/run_corpus_escape.py',profiles=[c+'-'+p for c in ['gcc','clang'] for p in ['debug','release','asan']],assertion_marker='ES_ASSERT:',mutants_per_anchor=2,fixture_oracle='assessor/corpus_escape/oracle.py',oracle_driver='escape_driver')))
write('fixtures/corpus_escape/cases.json',dict(synthetic=True,ordinary=dict(plain="&<>\"'",encoded='&amp;&lt;&gt;&quot;&apos;',length=25),single_pass=dict(encoded='&amp;lt;x&amp;gt;',decoded='&lt;x&gt;'),dialect=dict(template_apostrophe='&#39;',named_apostrophe='&apos;',literal_number='&amp;#39;'),scope='Expected finite examples, not execution evidence.'))
write('research/v5/expansion/corpus_escape/taxonomy_refinement.json',dict(exercise=ident,objectives=[x[0] for x in criteria],subtopics=['Textual encode/decode inverse and consume-once invariant','Malformed references and unchanged-output capacity admission','Context/dialect mismatch in actual template/event composition'],rationale='Fixed84 escape-sequence description exposes missing paired textual mechanism; refine existing grammar/boundary/verification leaves without changing accepted wording, IDs or historical hours.'))
print('PASS escape canonical authoring')
