"""Author canonical Markdown/JSON from individually written mechanism cards.

This local authoring helper is not a replacement for the repository's catalog
generation. Only canonical outputs are consumed by the established build.
"""
import json
from pathlib import Path
from polish_teaching import polish_document
R=Path(__file__).resolve().parents[3]
def read(p):return json.loads((R/p).read_text(encoding='utf-8'))
def write(p,s):
 s=polish_document(p,s)
 path=R/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(s,encoding='utf-8',newline='\n')
def dump(p,v):write(p,json.dumps(v,ensure_ascii=False,indent=2)+'\n')
def md(p,m,b):write(p,'```json\n'+json.dumps(m,ensure_ascii=False,indent=2)+'\n```\n\n'+b)

MODULE_ENVIRONMENTS = {
  "T66": "Use profile A: hosted ISO C17 on Linux x86-64, with exact-width integers and C17 atomics. The [ownership laboratory contract](../../labs/ownership/CONTRACTS.md) defines the live-object, callback and aliasing preconditions. GCC and Clang debug, release, ASan/UBSan and separate TSan profiles are declared in labs/ownership/family.json; pthreads provide the concurrent test backend. CMake, Python and diagnostic runtimes are host verification tools. Implement ownership and allocation policy from the declared primitives, preserving the imported V4Allocator ABI; a library ownership implementation cannot replace this work.\n\nThe instrumented allocator has eight actual int32_t[16] backing arrays; an owner acquires at most three blocks and borrowing has four guard slots. Exhaustion, partial construction, serial retirement and release-once accounting are part of the contract. Dynamic borrowing and the non-atomic reference-count path require external serialization; the atomic count path still requires an already owned strong or weak reference, valid publication and separate payload synchronization. Cancellation requests do not permit storage reuse before quiescence. Ordinary C copying can duplicate an owning representation, raw aliases can bypass guards, and strong cycles remain retained.\n\nThis hosted evidence does not certify a strict freestanding image, lock-free lowering, all interleavings or physical targets. Audit emitted helpers and link dependencies before transferring to another runtime. Security experiments remain the original local two-tenant array with synthetic values; a cross-tenant write demonstrates only its stated integrity failure. Author campaigns do not establish learner mastery or delayed transfer, and exact published C17 wording remains an access gap.",
  "T67": "Use profile A: hosted Linux x86-64 and ISO C17 with extensions disabled and -Wall -Wextra -Wpedantic -Werror. The [ADT laboratory contract](../../labs/adt/CONTRACTS.md) defines the permitted representations and disjoint live-object inputs/outputs. The family declares GCC and Clang debug, release and ASan/UBSan profiles. Python independent oracles, CMake and compiler diagnostics are host tools; no Unicode, parser, graph or algebraic-data-type library may implement the mechanism being assessed.\n\nThe supplied operations are serialized. Validated text borrows at most 65,535 stable bytes and promises Unicode scalar validation, not normalization or grapheme segmentation. The branded pool has four slots with nonwrapping generation retirement; its live address cannot be reused while handles survive. Recursive values contain at most 32 nodes and use fixed iterative scratch storage. Read-only strided views may alias logical elements, but their backing extent and lifetime remain caller obligations. Public tags, handles and views are forgeable C representations; successful numeric checks do not validate arbitrary pointers or provide static lifetime enforcement.\n\nThe TLV and packet grammars are original bounded formats, not general Protocol Buffers compatibility. Keep source-bound hosted anchors distinct from the required learner io.frame/cursor integration and changed-condition work. Security demonstrations use only synthetic local inputs and property violations in these teaching programs. Strict freestanding dependency closure, concurrent adapters and physical execution need separate evidence; finite tests and sanitizers do not prove all representations safe. Exact published C17 wording remains an access gap.",
  "T68": "Use profile A: hosted Linux x86-64 and ISO C17 with extensions disabled and -Wall -Wextra -Wpedantic -Werror. Follow the [metaprogramming laboratory contract](../../labs/metaprog/CONTRACTS.md), including live, disjoint input/output objects and serialized operations. GCC and Clang debug, release and ASan/UBSan are the declared profiles. Preprocessor output and positive/negative translation units assess generation and constraints; Python, CMake and temporary generated-file compilation are host verification facilities, not substitutes for the C mechanisms.\n\nThe template rings each hold four elements, the C17 nonempty variadic demonstration accepts one to four arguments, the token expander admits at most 32 tokens, and the build graph has at most eight nodes. The property runner enumerates bounded pairs through 255 and finds a local minimum under its stated shrinking moves. Every bound has an explicit rejection or exhaustion rule. Generated signatures and _Generic distinguish declared categories, while runtime extents, callback purity, ownership and algebraic laws require separate checks or discipline. The formatter treats percent sequences as literal data and may not delegate conversion to printf-family routines.\n\nThe explicit token-context model is not a complete C or Rust macro parser, and its hygiene argument is confined to that representation. FNV-1a equality/cache keys are not cryptographic integrity or authenticated-cache guarantees. No final C23 or C++26 feature guarantee follows from an inspected proposal or census entry; the implementation baseline remains C17 and exact final-edition text gaps stay recorded. Experiments use original local programs and synthetic inputs. Hosted diagnostics and finite law tests do not establish strict freestanding closure, arbitrary compile-time evaluation, absolute mastery or the required delayed transfer.",
  "T73": "Use profile A: hosted ISO C17 on Linux x86-64, with POSIX pthreads, mutexes and condition variables as explicit backend dependencies. The [messaging laboratory contract](../../labs/messaging/CONTRACTS.md) and messaging.h define actual storage, callback and concurrency preconditions. GCC and Clang debug, release, ASan/UBSan and separate TSan profiles are declared. CMake and Python are host verification tools. Construct the channel, actor, scheduler and transaction mechanisms locally; a ready-made implementation cannot replace them.\n\nThe nursery has eight owned child slots, the hub has two channels of capacity zero through four, and the work-stealing deque has eight atomic-value slots. Rendezvous success requires receiver acknowledgement; close/drain and terminal cancellation have different semantics. The finite STM uses four atomic cells and sequentially consistent accesses; version snapshots have eight stored versions and four reader pins. Actors, sender/receiver operation states and demand streams retain their separately stated serialized protocols. Stable addresses, callback lifetime and quiescent destruction are mandatory. Cooperative cancellation neither bounds join latency nor proves that a callback has stopped accessing storage.\n\nThe SC arguments do not validate weaker atomic orders, arbitrary pointer reclamation, durable transactions or fairness. Check actual atomic lowering and lock-free observations instead of inferring progress from a type name. Real pthread stress and TSan cover executed schedules only; required coroutine, asynchronous-I/O and existing learner-artifact adapters need their own evidence. Experiments stay within original local processes and synthetic messages. These hosted campaigns do not validate a kernel, bare-metal target, physical timing, independent learner performance or every interleaving; published C17 text remains an explicit source-access gap.",
  "T76": "Use profile A: hosted Linux x86-64 and ISO C17 with 8-bit bytes, 32-bit int and uint32_t/uint64_t. The [replicated laboratory contract](../../labs/replicated/CONTRACTS.md) states exact aliasing, publication and stable-arena requirements. GCC and Clang debug, release, ASan/UBSan and separate TSan profiles are declared. Core state transitions are serialized; the supplied projection adapter uses one real pthread mutex across state and cursor publication. Python, CMake and diagnostic runtimes are host tools, and no target hash, CRDT or replication library may replace the implemented mechanisms.\n\nThe memory log retains at most 16 events and 17 roots in the actual imported persist.hamt arena, whose 64-node capacity and three-node update rule remain binding. Select the completed DOD learner workspace explicitly; assessor-selected dependencies are author validation only. Clock/PN models use three replica identities, the observed-remove set has two writers with eight nonreused add identities each, sequences have 16 nodes, and the original message grammar has at most eight fields. SHA-256 is assessed over the specified 0..65-byte inputs and Merkle trees over 0..8 leaves; these bounds do not establish a general-purpose cryptographic implementation.\n\nSingle-writer identity ownership, retained tombstones and live immutable snapshot roots are premises, not Byzantine admission or persistence mechanisms. Timestamp order does not imply causality; hash equality does not authenticate a sender; pairwise OT checks do not prove a complete distributed editor. Required storage.pages, codec.records, io.frame, ipc.local and effect-adapter investigations need separate short-I/O, crash and recovery evidence beyond the in-memory campaign. Keep all experiments local with synthetic messages. Finite hosted tests establish neither deployment safety nor learner mastery; exact published C17/C23 text and physical/remote-system validation remain outside the inspected evidence."
}

def author(domain,family,prefix,title,data,prereqs,imports,exports,sources,phase=11,strand='toolbox',campaign=None):
 refs=[dict(id=s['id'],path=p,passages=s['locators']) for p,s in sources]
 env=dict(language='ISO C17; C11 draft comparisons explicitly bounded',target='Hosted Linux x86-64',toolchain=['GCC','Clang','CMake','Python','POSIX pthread harness'],dependencies='Implement mechanisms locally using declared typed storage/callbacks; hosted harness and diagnostic runtimes are separate. See laboratory contract for exact dependencies.')
 topics=[];units=[];projects=[];exercise_rows=[];recalls=[];crosswalk=[]
 for i,d in enumerate(data,1):
  topic=f'{domain}.{i:02}';pid=f'PR-{domain}-{i:02}';xid=f'X-{domain}-{i:03}'
  objs=[dict(id=topic+'.'+c,text=d[k]) for c,k in zip('abcd',('claim','construct','evidence','transfer'))];oids=[o['id'] for o in objs]
  rub=[dict(id=xid+f'-R{j}',objective=o['id'],criterion=o['text'],levels={'0':'Missing or contradicted by the stated contract.','1':'Nominal test only; fails to explain the listed mechanism or boundary.','2':'Meets this complete criterion with justified assumptions and independent evidence.','3':'Meets it under the unseen changed-condition variant and explains a counterexample to the former assumptions.'}) for j,o in enumerate(objs,1)]
  facets=[dict(objective=o['id'],teaching=['#problem','#necessity','#reasoning'],implementation=['D'+str(j)],boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[rub[j-1]['id']],assessed_claim=o['text'],scope='Authored assessment obligations, not an assertion that a learner has demonstrated them.') for j,o in enumerate(objs,1)]
  analysis=f'assessor/{family}/analyses/{xid}.md';fixture=f'fixtures/{family}/{pid}.json'
  selected=[r for r in refs if r['id'] in d.get('refs',[r['id'] for r in refs])]
  shared=['curriculum/v5/exercises/shared-contract.md','curriculum/v5/exercises/modern-cost-contract.md',f'labs/{family}/CONTRACTS.md']
  meta=dict(schema_version=1,id=xid,title=d['title'],project=pid,domain=domain,objectives=oids,families=list(d['families']),required=True,prerequisite_exercises=d.get('prerequisite_exercises',[f'X-{domain}-{i-1:03}'] if i>1 else []),prerequisite_artifacts=d.get('imports',imports),profile='A',environment=env,source_records=selected,lab=dict(family=family,tests=[family+'.'+d['case']],scaffold='labs/'+family),fixtures=[fixture],shared_contracts=shared,assessor_analysis=analysis,status=dict(content='authored',execution='see-source-bound-campaign',independent_review='not-run',learner_assessment='not-assessed',limitations=['Exact published C17 text remains an access gap.','Public anchors do not establish all written independent or delayed obligations.']),objective_coverage=facets,rubric=rub,delayed_transfer=dict(minimum_days=7,variant_ids=[xid+'-DELAYED']))
  body=f'''# {xid}: {d['title']}

## Prerequisites

{d.get('prerequisites','Complete the earlier domain projects and retain the named prerequisite artifacts with their contracts, ownership ledger and tests.')} Required learner artifacts: {', '.join(meta['prerequisite_artifacts'])}. Do not import an assessor implementation.

## Environment

{env['dependencies']} Use C17 with extensions disabled and warnings treated as errors. A diagnostic extension is identified explicitly and does not become an ISO guarantee. The full platform, pointer, callback and concurrency preconditions are in [{family}/CONTRACTS.md](../../../labs/{family}/CONTRACTS.md).

## Problem

{d['problem']}

Concrete inputs and expected results: {d['example']}

Supplied artifacts: labs/{family}/include/{family}.h, compiling unsolved src/{family}.c, CTest {family}.{d['case']}, {fixture}, and the shared contracts. State every additional backend assumption before running it.

## Necessity

{d['claim']} The misconception and changed-condition case below are separately assessed; ordinary successful execution alone does not cover them.

## Reasoning

{d['construct']} Give the abstraction function, admitted states, pre/postconditions, ownership graph, failure effects and complexity before implementation. Identify the enforcement boundary of every safety claim using the shared modern-cost contract. Distinguish an invalid caller object from an operational failure admitted by the API.

## Deliverables

- D1: {d['claim']}
- D2: {d['construct']}
- D3: {d['evidence']}
- D4: {d['transfer']} Complete all thirteen cost/enforcement dimensions with evidence or a mechanism-specific non-applicability.

## Verification

{d['evidence']} Preserve predictions before execution, exact commands, source/input/toolchain identity and observed outputs. Explain independent-oracle assumptions and minimize disagreements. A supplied anchor or compiled mutant checks only its named property. Failure-side effects, ownership accounting and the required changed-condition result are pass/fail gates; finite tests cannot establish absolute mastery.

## Rubric

The metadata rubric maps each leaf to a distinct D1–D4 criterion. Require every criterion at level2 and at least two at level3 on independently attempted cases; record assistance. A violated ownership/failure contract, unsupported guarantee or incorrect exploitability claim fails regardless of performance. Passing only the public suite earns at most level1. Apply the shared six-dimension rubric as well.

## Hints

1. {d['hints'][0]}
2. {d['hints'][1]}
3. {d['hints'][2]}

## Variants

{d['transfer']}

Delayed assessment {xid}-DELAYED: use the separately held assessor variation at least seven days after assisted work, without the previous implementation. Record the new invariant and a counterexample to the old proof before coding.

## Security

{d.get('security','Use only original local teaching code and synthetic values. No external target, credentials, privilege changes or deployed equipment. Treat pointer validity as a precondition; do not run fabricated native pointers as a portable oracle. A diagnostic failure or resource leak is not evidence of code execution.')}

## Validation

Authored; actual executions come from a matching source-bound family campaign. Independent review and learner assessment have not occurred. Host state models and tests do not establish physical or privileged target behavior. The full independent investigation and delayed variant remain required even after public tests pass.
'''
  md('curriculum/v5/exercises/'+xid+'.md',meta,body)
  write(analysis,f'''# {xid} assessor technical analysis

{d['analysis']}

## Assessment boundaries

The assessor implementation is {family}.c in this separate directory tree. Review D1–D4 individually against the metadata rubric. The deliberate MOD_MUTATION values {2*i-1} and {2*i} targets the {d['case']} anchor; require its named semantic assertion, not a compiler failure, timeout or unrelated crash. Additional independent tests remain mandatory.

## Delayed case {xid}-DELAYED

{d['transfer']}

Require the revised contract, minimal counterexample, implementation/trace and cost evidence before feedback. Assess the invariant under the new assumptions, not just output similarity. {d.get('delayed_analysis',d['analysis'])}

## Limits

This analysis records the technical solution and expected reasoning, not learner performance or a non-author review. Public-source comparisons are restricted to the registered inspected passages. Tests, sanitizers and bounded models do not prove arbitrary lifetimes, interleavings or environments correct.
''')
  matched=[c for c in read('fixtures/'+family+'/cases.json')['cases'] if c['kind']==d.get('fixture_kind',d['case'])]
  dump(fixture,dict(id=pid,description=d['title'],input=dict(case=d['case']),expected=dict(contract=d['example']),cases=matched,verification_method='Independent state/set oracle recomputes cases.json; prose and changed conditions are assessed separately.'))
  pm=dict(id=pid,title=d['title'],module=domain,topics=[topic],status='core',hours=8,profile='A',fixtures=[fixture,'fixtures/'+family+'/cases.json'],edition=5,requires_projects=d.get('requires_projects',[f'PR-{domain}-{i-1:02}'] if i>1 else []),evidence_template='component')
  md('curriculum/projects/'+pid+'.md',pm,f'''# {pid}: {d['title']}

## Contract

{d['problem']}

The full required exercise {{ref}}`{xid}`, its shared contracts and objective rubric are part of this project. Eight hours is descriptive author accounting, not a learner-time or scope ceiling.

## Inputs and expected results

{d['example']}

## Steps

- **{topic}:** {d['construct']} Submit the exercise D1–D4 and retain prerequisite interfaces.

## Failure cases

{d['evidence']}

## Acceptance and evidence

- **{topic}:** {d['claim']} Every leaf has a separate rubric criterion in {xid}. Public test passage alone does not assess independent understanding.

## Hints and recovery

{d['hints'][0]} Then {d['hints'][1]} Record the failed hypothesis before using further hints.

## Transfer challenge

{d['transfer']}
''')
  topics.append(dict(id=topic,title=d['title'],elective=False,objectives=objs))
  projects.append(dict(id=pid,tests=[family+'.'+d['case']],scope='Hosted original mechanism; full exercise reasoning and independent/delayed evidence remain required.'))
  exercise_rows.append(dict(id=xid,project=pid,tests=[family+'.'+d['case']],capabilities=['linux-c17']))
  for part in range(2):
   units.append(dict(id=f'LAB-{prefix}-{2*i-1+part:02}',project=pid,title=('Construct: ' if not part else 'Falsify and transfer: ')+d['title'],minutes=240,new_executable=not part,scaffolding='independent design' if i>3 else 'bounded choice',kind='implementation' if not part else 'adaptation',objectives=oids,tests=[family+'.'+d['case']] if not part else [],steps=[d['claim'],d['construct']] if not part else [d['evidence'],d['transfer']],checks=[d['example'],d['evidence']],reasoning=[d['claim']],deliverables=[d['construct'],d['evidence'],d['transfer']],rubric=rub,variants=[d['transfer']],assessor_analysis=analysis,target_scope='host',scope=d['claim'],remaining_evidence='Independent reasoning, changed-condition implementation and all resource/target claims outside the anchor remain separate.',references=[dict(resource_id=r['id'],locator=r['passages'][0],purpose='Bounded source comparison for this mechanism') for r in selected]))
  variants=[dict(id=topic+'-'+name,kind=kind,objectives=oids,prompt=prompt,checklist=[o['text'] for o in objs]) for name,kind,prompt in [('explain','mechanism',d['claim']),('trace','trace-or-small-implementation',d['example']+' Derive each transition before executing.'),('transfer','changed-condition',d['transfer'])]]
  recalls.append(dict(id=topic,title=d['title'],objectives=oids,objective_text={o['id']:o['text'] for o in objs},projects=[pid],variants=variants,resource_ids=[r['id'] for r in selected],elective=False,grading='Grade each objective from the actual attempted evidence; consultation is assisted practice.'))
  for key in d['census']:crosswalk.append(dict(technique=key,disposition='project',target=pid,subtopic=topic,exercise=xid,rubric=[r['id'] for r in rub],reason=d['claim'],status='authored; independent assessment not performed'))
 dump(f'research/v5/modern/{family}_taxonomy.json',dict(domains=[dict(id=domain,title=title,depth='Mechanism construction, falsification, composition and independent transfer',subtopics=topics)]))
 dump(f'curriculum/v5/recall/{domain}.json',dict(schema_version=5,sets=recalls))
 dump(f'labs/{family}/family.json',dict(version=1,family=family,title=title,build_system='cmake',assessor_root='assessor/'+family,projects=projects,units=units,exercises=exercise_rows,tests=[family+'.'+d['case'] for d in data],modules=[domain],contract=f'labs/{family}/CONTRACTS.md',campaign=campaign,dependencies=[],scope='Hosted mechanism evidence only; no independent learner or strict target acceptance.'))
 dump(f'curriculum/v5/domains/{domain}.json',dict(id=domain,domain=dict(family=family,prefix=prefix,phase=phase,strand=strand,research_gate=f'research/v5/modern/{family}_research_gate.json',imports={'*':imports},artifacts=exports,decisions={}),new_executable_counts={prefix:len(data)},fixture_oracles=[f'verify_{family}_fixtures.py']))
 mm=dict(id=domain,title=title,track='shared',status='core',prerequisites=prereqs,prelude_objectives=[],workload=dict(reading=8,implementation=len(data)*4,verification=len(data)*4,assessment=8,consolidation=8),readings=[dict(resource_id=s['id'],locator=s['locators'][0],purpose=s['reason'],hours=1) for _,s in sources[:8]],projects=[p['id'] for p in projects],profile=['A'],equipment=['Linux C17 hosted laboratory','GCC and Clang','Python independent oracle'],edition=5)
 mb=f'# {domain}: {title}\n\n## Purpose and outcomes\n\nBuild and explain the representations and state transitions below, preserving the imported artifact contracts. The complete exercises are core work; descriptive hours do not cap depth. Distinguish ISO semantics, implementation diagnostics, runtime checks and programmer discipline.\n\n## Entry check and remediation\n\nDerive ownership through a failed acquisition and a retained borrow using the prerequisite artifacts. State the backing lifetime, aliasing and failure contract before composing the prerequisite artifacts.\n\n## Study sequence and teaching\n\n'
 if domain in MODULE_ENVIRONMENTS:
  mb=mb.replace('## Study sequence and teaching\n', '## Environment and limits\n\n'+MODULE_ENVIRONMENTS[domain]+'\n\n## Study sequence and teaching\n', 1)
 for i,d in enumerate(data,1):mb+=f"### {domain}.{i:02} — {d['title']}\n\n{d['claim']}\n\n{d['example']}\n\nConstruction: {d['construct']} The full assessed work is {{ref}}`X-{domain}-{i:03}`.\n\nMisconception: {d['hints'][2]}\n\nEvidence: {d['evidence']}\n\n"
 mb+='''## Reading guidance

Only the registered bounded passages are claimed inspected. The census is a discovery aid. Language-specific guarantees do not transfer to an arbitrary C representation. Exact published C17 access remains an explicit reference gap.

## Implementation projects

Follow the listed producer dependencies and complete both authored units for each project. Each exercise includes concrete APIs, fixtures, independent evidence, graduated hints and a separate assessor analysis. Reuse the existing learner artifact contracts and disclose adapter differences.

## Mastery gate and feedback

Every objective requires a separate rubric judgment. Compare failure-side ownership, boundary tests and counterexamples before timing. Independent and delayed attempts cannot be inferred from author campaigns.

## Consolidation and transfer

Use each project's changed-condition variant and the shared thirteen-dimension modern-cost contract. Compose with exported artifacts only after their producer obligations have been met. Retain unresolved source, platform and research questions in the gap ledger.

## Validation status

Authored, with actual execution reported only in source-bound receipts. No learner assessment, physical validation or non-author review is implied.
'''
 md('curriculum/modules/'+domain+'.md',mm,mb)
 dump(f'research/v5/modern/coverage/{domain}.json',dict(domain=domain,census=crosswalk,swe=[],limitations='Authored obligation mapping, not learner mastery or complete corpus coverage.'))
