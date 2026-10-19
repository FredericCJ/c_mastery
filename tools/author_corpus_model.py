"""Author one additive semantic-model/REPL exercise, without global generation."""
import json
from common import ROOT,sha,dump
BASE='research/v5/expansion/corpus_model'
ID='X-T15-021'
SOURCES=[
 ('semantic','R-CMODEL-SEMANTIC','Semantic Model',['Martin Fowler'],2026,
  'Online DSL catalog entry, exact 18 October 2026 capture; book chapter not inspected',
  'https://martinfowler.com/dslCatalog/semanticModel.html',
  ['The short Semantic Model catalog definition and reference to chapter11'],
  'Brief catalog only, not the referenced book chapter. It motivates a separately represented domain model, not this original graph API.'),
 ('builder','R-CMODEL-BUILDER','Generating Code for DSLs',['Martin Fowler'],2005,
  'Article dated 12 June 2005, exact online capture 18 October 2026',
  'https://martinfowler.com/articles/codeGenDsl.html',
  ['Single Pass Builder: textual configuration constructing a runtime Reader',
   'Two Pass Builder: syntax AST, separate Reader construction, alternate XML syntax and independently testable stages'],
  'Selected actual article sections, not the entire DSL book. Runtime semantic objects are distinguished from a temporary syntax tree; no reuse of the article implementation is claimed.'),
 ('repl','R-CMODEL-REPL','An Introduction to Scheme and its Implementation — The Read-Eval-Print Loop',
  ['Authorship not identified in the inspected online section'],1997,
  'HTML edition generated 19 February 1997 with texi2html1.45 (index footer); exact section capture 18 October 2026',
  'https://docs.scheme.org/schintro/schintro_114.html',
  ['The Read-Eval-Print Loop: read representation, evaluate, print, repeat; evaluator callback'],
  'Actual selected section and index edition footer. No full interpreter or Scheme error/output transaction guarantee is assigned; the recoverable C session policy is original.')]
def main():
    rows=[]
    for name,rid,title,authors,year,edition,url,locators,limits in SOURCES:
        p=f'build/corpus_model_sources/{name}.html'
        rows.append(dict(id=rid,title=title,authors=authors,year=year,edition=edition,kind='web',url=url,
            access='Actual primary online passage inspected and byte-bound capture retained',domains=['T15'],role='primary',assignable=True,
            reason='Exact semantic-model separation or persistent read/eval/print mechanism',
            prerequisites=['Accepted X-T15-012 parser','C17 lifetimes and checked arithmetic'],locators=locators,
            inspection=dict(date='2026-10-18',method='Read the named primary online sections themselves, then retained exact HTML capture.',
                scope='; '.join(locators),limits=limits,download_path=p,download_sha256=sha(ROOT/p))))
    dump(ROOT/BASE/'resources.json',dict(resources=rows))
    dump(ROOT/BASE/'taxonomy_extensions.json',dict(schema_version=1,
        kind='required-competency-refinement-of-existing-leaf',objectives=['T15.14.c'],exercises=[ID],source_records=[r['id'] for r in rows],
        scope_rationale='The finite84 screening tail distinguishes a persistent semantic model from a transient AST, and a real persistent read/eval/print loop from a one-shot evaluator. This is a concrete additive refinement of syntax/semantics/diagnostics separation, not a replacement for tokenizer or parser implementation.',
        preservation='Original objectives, criteria, providers and historical receipts unchanged.'))
    claims=[
      'Implement the persistent signal domain graph with stable IDs, checked transactional updates, dependency/cycle validation and cached-value propagation; explain why it is not an AST.',
      'Construct the same semantic model through two actual configuration syntaxes and a direct API, then compose the accepted lexer/Pratt/evaluator for transient queries with grouped symbol values and explicit limits.',
      'Implement persistent read/eval/print with command-error recovery, commit-before-reply and prefix-owned output retry; execute the required changed chunk/EOF/failed-print reference without duplicating effects.']
    rubric=[dict(id=ID+f'-R{i}',objective='T15.14.c',criterion=claim,levels={
        '0':'Missing mechanism or false publication/lifetime claim.',
        '1':'Selected examples work but a required condition or independent distinction is absent.',
        '2':claim+' D1-D3 retain exact source/code/finite-check evidence.',
        '3':'Unaided D4 changes the stream/output conditions and defends the resulting state and effect trace.'}) for i,claim in enumerate(claims,1)]
    meta=dict(schema_version=1,id=ID,title='Persistent semantic signal graph and resumable read/eval/print',
        project='PR-T15-09',domain='T15',objectives=['T15.14.c'],families=list('ABCDEFGHIJ'),required=True,
        prerequisite_exercises=['X-T15-012'],prerequisite_artifacts=[],profile='C',
        environment=dict(language='ISO C17 with exact-width signed32/unsigned32 and signed64 types',target='Hosted Linux reference experiments',
            toolchain=['GCC','Clang','CMake','Python3'],dependencies='Original fixed-array domain/session code; explicitly supplied accepted original pd_lex/pd_pratt/pd_evaluate provider. Stdio/Python are host support. No external graph/parser/runtime library.'),
        source_records=[dict(id=r['id'],path=BASE+'/resources.json',passages=r['locators']) for r in rows],
        lab=dict(family='corpus_model',scaffold='labs/corpus_model',tests=['corpus_model.contract']),
        assessor_analysis='assessor/corpus_model/ANALYSIS.md',
        status=dict(content='authored',execution='source-bound campaign required',independent_review='not-run',learner_assessment='not-assessed',
            limitations=['Bounded original model/languages, not Scheme or a general DSL framework.','Live-session output recovery is not durable exactly-once delivery.','Author execution is separate from delayed learner, external human and physical observations.'],
            campaign_receipt='reviews/v5/expansion/corpus_model/current_validation.json'),
        fixtures=['fixtures/corpus_model/cases.json'],shared_contracts=['labs/corpus_model/CONTRACTS.md'],
        delayed_transfer=dict(minimum_days=7,variant_ids=[ID+'-DELAYED']),rubric=rubric,
        objective_coverage=[dict(objective='T15.14.c',teaching=['#problem','#reasoning'],implementation=['D1','D2'],
            boundary=dict(applicable=True,evidence=['D3']),independent_assessment=[r['id'] for r in rubric],
            assessed_claim='Persistent semantic state and explicit diagnostics are separate from transient parsing.',
            scope='This required companion refines semantic separation; it does not independently discharge the prerequisite tokenizer/parser leaves.')])
    body='''# X-T15-021: Persistent semantic signal graph and resumable read/eval/print

## Prerequisites

Complete X-T15-012 and supply its accepted original parser through the explicit build interface. Read the exact registered Fowler and Scheme passages, then CONTRACTS.md. Neither an assessor solution nor a whole uninspected book is a learner prerequisite.

## Environment

Use the declared C17 host profile, fixed typed storage and explicit spans. Record compiler/ABI and the accepted parser's exact source identity. C23 does not add automatic transactional publication or borrowed-text lifetime. No external graph library, Scheme runtime or alternate expression parser substitutes for the studied mechanisms.

## Problem

Implement model.h in src/model.c. A stable-ID signal DAG owns input constants and weighted two-dependency definitions. Stage a whole batch, validate references, detect cycles, recompute cached values with checked intermediate arithmetic, then publish exactly once. Both line syntax and CSV build the same persistent domain model; a direct API constructs it without syntax. Support forward references and reject duplicate definitions or trailing text.

Build persistent commands for input/linear/set/get/eval/exit. Query references use current model values and the actual accepted lexer, right-power Pratt parser and checked evaluator. The expression AST is discarded; the model survives. Retain a bounded reply after each admitted command. A command error preserves the model, while a failed print preserves both the already committed model and the unaccepted reply suffix.

## Necessity

A parse tree describes a source expression, but it does not by itself define reusable signal identity, dependency updates, transactional graph validation or cached domain values. Two surface languages should not require two semantic engines. A one-shot calculator also omits persistent state and the boundary between committing an effect and printing its result. Recovering from a failed print by rerunning a command can duplicate its effect.

## Reasoning

Trace the example graph through both configuration parsers into transient edits and then the persistent model. Prove the repeated-resolution progress measure or derive an equivalent bounded topological method. Explain why forward-reference order differs from source order, why zero-weight cyclic edges remain dependencies, and why cancellation cannot repair an overflowing intermediate product. Bound staging, scans and parser expansion separately.

For input0=-3, compare `$0^2` with ungrouped `-3^2`. Explain the INT32_MIN expansion without silently widening the accepted lexer. Keep expanded diagnostic offsets distinct from original source locations. Derive reply authority after one accepted byte, a zero-progress callback, an attempted intervening mutation and a resumed drain. State what the live-session contract cannot promise after process or sink failure.

## Deliverables

- D1: Domain-versus-AST representation map, transaction/invariant/lifetime tables, exact inspected source locators and concrete counterexamples to two false explanations.
- D2: Original completed model, both frontends, actual accepted parser composition and persistent command/session implementation with dependency manifest.
- D3: Independent recursive graph and arithmetic checks, exact transcript/effect expectations, rollback snapshots, six compiling semantic controls and actual GCC/Clang receipts.
- D4: Delayed changed-condition stream and prefix-output route with retained state, EOF, malformed input and failure/resumption evidence.

## Verification

Compare line, CSV and direct definitions for the same720 bounded graph constructions, then execute changed-input, self-cycle and duplicate-update histories. Check signed endpoints, missing dependencies, revision exhaustion, overflowing intermediates, grouped references, parser token/expansion limits and unchanged outputs. Mutating the old source buffer must not change the model.

Use a recursive arbitrary-precision oracle independently of the C scan. Run actual persistent stdio processes, and drive a live session under every selected reply-prefix interruption, write windows1/2/7/96, varying input chunks, an oversized discarded line and a final line without newline. Verify exact reply bytes and revision count, not only final values. The writer must accept the reported prefix; a zero report is not terminal cancellation.

Configure labs/corpus_model with CMake. Default learner and prerequisite stubs compile but must fail CM_CHECK. Explicit completed model plus accepted parser sources must pass. Each compiled semantic control must fail the intended assertion rather than fail compilation or crash. Isolated CLI export must include public contracts and unsolved sources while excluding assessor files.

## Rubric

Assess the three JSON criteria independently. A stateless evaluator fails the persistent-session criterion; a second parser without a distinct runtime domain graph fails the model criterion. Level2 needs concrete finite evidence and explicit limits. Level3 requires unaided delayed work; retained author variants are references, not proof of elapsed learner mastery.

## Hints

1. Draw the objects that survive a command beside those that disappear after parsing.
2. Write the transaction publication point before writing either frontend.
3. Give accepted and pending output bytes different owners; never reexecute to repair printing.

## Variants

X-T15-021-DELAYED: at least seven days later, implement the chunked cm_feed/cm_finish and prefix-aware cm_drain route from the complete-line model. Change input chunk sizes across command boundaries, remove the final newline, stop at exit with trailing input, inject an oversized/malformed line, and interrupt output at every accepted-prefix position of the selected transcript. Keep a live session while retrying zero-progress writes, vary positive write windows and attempt an intervening mutation while output is pending. Preserve exact replies, consumed-prefix positions and the once-per-command revision count. Execute actual stdio read/eval/print separately; explain why process-exit failure does not preserve the in-memory recovery object. No new syntax, source rewrite or external event loop may hide the ownership boundary.

## Security

Use only bounded synthetic local text and the original local teaching providers. Adversarial cycles, limits, malformed bytes and output callbacks test their stated refusal contracts. This is not an external-service, cryptographic or production-security exercise. Minimized local mutant counterexamples establish selected invariants only.

## Validation

Source inspection, author execution, bounded teammate source review, outside human review, independent learner work and physical observation remain separate. Learner sources stay incomplete. No current result is inferred until the mechanism guard and actual isolated export have checked exact source identities.
'''
    (ROOT/f'curriculum/v5/exercises/{ID}.md').write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf-8',newline='\n')
    dump(ROOT/BASE/'screening_resolution.json',dict(schema_version=1,
        origin='research/v5/expansion/corpus_followthrough/screening_map.json',candidates=['semantic-model','repl'],exercise=ID,
        mechanism='Persistent domain DAG with two frontends/direct API; actual accepted parser queries and a persistent print-resuming session.',
        limits='Bounded original graph/languages; no Scheme, code-generator or full book equivalence.'))
    print('Authored one additive semantic-model/REPL card; no global writes')
if __name__=='__main__':main()
