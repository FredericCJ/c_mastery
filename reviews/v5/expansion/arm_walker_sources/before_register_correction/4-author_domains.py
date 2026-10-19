"""Materialize only the B06 canonical sources from hand-authored domain specifications.

This is an authoring convenience, not an acceptance producer. It never changes
edition manifests, old identities, aggregate catalogs or historical receipts.
"""
from pathlib import Path
import importlib.util,json,hashlib
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).parent
def write(path,text):
    path=ROOT/path;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text,encoding='utf-8',newline='\n')
def dump(path,value):write(path,json.dumps(value,indent=2,ensure_ascii=False)+'\n')
def doc(path,meta,body):write(path,'```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body.strip()+'\n')
def load(name):
    spec=importlib.util.spec_from_file_location(name,HERE/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m.DOMAIN
def project(title,slug,teach,contract,prototype,implementation,tests,fixture,reasoning,failure,variant,hints,terms,refs=None,external=None):
    return locals()
def attach_project_support(pid,meta,text):
    """Preserve the original B06 brief while attaching a concrete required successor."""
    if pid=='PR-T59-07':
        path=ROOT/'research/v5/expansion/cacheable_boot_depth/attachment.json'
        if path.is_file():
            attachment=json.loads(path.read_text(encoding='utf-8'))
            for key in ['shared_contracts','supplied_artifacts']:
                meta[key]=list(dict.fromkeys(meta.get(key,[])+attachment[key]))
            meta['cacheable_boot_author_support']=attachment['support']
            if attachment['body'].strip() not in text:text=text.rstrip()+attachment['body']
    return meta,text
SHARED='''# B06 common exercise and evidence contract

This contract applies to every PR-T57–PR-T64 project and its LAB unit. Each
project supplies the mechanism-specific inputs, API, invariant, boundary and
changed-condition assessment; these obligations supplement its public tests.

## Prerequisites and permitted dependencies

Use the accepted C17 implementation baseline. C11 draft clauses supply precisely
identified semantic reasoning; a draft is not a substitute claim that published
C17 was inspected. C23 comparison work must identify the actual changed clause
and compiler support; unchanged ABI or OS behavior is not an ISO C guarantee.
Before entry demonstrate checked extent arithmetic, object lifetime, byte
decoding, resource ownership and reproducible build capture with the declared
prerequisite artifacts. Reading a module does not accept an artifact.

The supplied portable cores use only C17 types and explicitly declared host
math operations. Implement the mechanism from these primitives: no allocator,
compiler, container, verification engine or protocol library may replace the
mechanism being taught. Host-side Python/CMake/CTest are verification tools.
Kernel headers, system calls and ISA assembly belong to separately named
adapters. A hosted state model does not demonstrate kernel scheduling, device
ordering, cache coherency or flash persistence. For each freestanding adapter
compile with -ffreestanding, inspect relocation and undefined-symbol tables,
linker maps and disassembly for compiler helpers; report every dependency and
provide or remove it. Merely disabling builtins does not prove runtime closure.

## Execution and containment

Run host tests in a disposable working directory using synthetic fixtures.
Kernel modules run only in a disposable VM with a snapshot, no host shared
directories, no bridged networking and no production data; do not insert them
into the workstation kernel. Firmware work uses the supplied model/emulator
first. Physical transfer is restricted to dedicated NUCLEO-F439ZI or
NUCLEO-H745ZI-Q development boards and requires a separate authorized session;
this authoring task does not flash devices or change security settings.
Security cases target only the original teaching arena/dispatcher. The attacker
controls the explicitly supplied bytes, handles or event schedule, not real
credentials. Harmless success is the named sentinel/property transition.
Crash, undefined behavior and sanitizer detection are different observations;
none alone demonstrates code execution or privilege escalation.

## Deliverables and verification

Retain (D1) a prediction before execution, pre/postconditions, representation,
invariants, ownership transitions, failure atomicity and cost derivation;
(D2) learner source and build identity; (D3) complete commands, raw outputs,
tool versions and independently derived expected results; (D4) a minimized
counterexample, repair and regression; (D5) a changed-condition implementation
and comparison; (D6) an evidence-limit statement. Use supplied fixture data,
then independently construct at least one input that distinguishes your repair
from the named plausible wrong implementation. Public finite checks cannot
establish absence of all defects. Sanitizers observe executions; bounded proof
depends on assumptions/unwinding; measurements depend on workload and machine.

## Assessment rubric and hints

Every objective has its own criterion in the project and authored unit. Score
0 for no assessable artifact or a contradiction of its contract; 1 for a
passing demonstration without a causal explanation; 2 for an original
implementation/analysis with a correct invariant, boundary argument and
reproducible evidence; 3 for an unaided delayed changed-condition solution
that identifies which previous proof or measurements no longer apply.
Require all objectives at level2 before technical completion and level3 on the
specified transfer task after at least seven days before independent-transfer
completion. Record whether the attempt was self-reported or externally
reviewed. No fabricated reviewer independence, mastery certificate or absolute
mastery claim is permitted. Hints are graduated in each brief; consulting the
assessor solution changes the attempt to assisted practice.

## Validation states

Canonical writing means authored; a compiling scaffold means executable; a
source-bound observed run means executed. These do not imply learner acceptance,
external assessment, VM execution or physical validation. Each family records
remaining target evidence and each campaign reports exact commands. A model
pass cannot satisfy a missing mandatory adapter investigation. New descriptive
hours are separate from accepted baseline hours and impose no learner-time cap.
'''
def author(d):
    did=d['id'];family=d['family'];prefix=d['prefix'];projects=d['projects'];sources={r['id']:r for r in json.loads((ROOT/'research/v5/b06_resources.json').read_text(encoding='utf-8'))['resources']}
    refs=[dict(resource_id='R-B06-'+key,locator=sources['R-B06-'+key]['locators'][0],purpose=sources['R-B06-'+key]['reason'],hours=1) for key in d['refs']]
    units=[];rows=[];topics=[];body=[];recall=[];glossary=[];header=['#ifndef B06_'+prefix+'_H','#define B06_'+prefix+'_H','#include <stddef.h>','#include <stdint.h>','#include <limits.h>','#include <math.h>',d.get('types','')];impl=['#include "core.h"','#ifndef B06_MUTATION','#define B06_MUTATION 0','#endif',d.get('helpers','')];stubs=['#include "core.h"'];test=['#include "core.h"','#include <stdio.h>','#include <string.h>','#define CHECK(x) do { if(!(x)){fprintf(stderr,"B06_CHECK %s:%d: %s\\n",__FILE__,__LINE__,#x);return 1;} } while(0)'];dispatch=[];cmtests=[];analyses=[]
    for n,p in enumerate(projects,1):
        tid=f'{did}.{n:02}';pid=f'PR-{did}-{n:02}';testname=family+'.'+p['slug'];fx=f'fixtures/b06_{did.lower()}/{n:02}_{p["slug"]}.json';analysis=f'assessor/{family}/{pid}.md'
        objectives=[{'id':tid+'.a','text':'Derive and defend: '+p['reasoning']},{'id':tid+'.b','text':'Implement and investigate: '+p['contract']},{'id':tid+'.c','text':'Falsify and transfer: '+p['failure']+' Then '+p['variant']}]
        topics.append(dict(id=tid,title=p['title'],elective=False,objectives=objectives));oids=[o['id'] for o in objectives]
        dump(fx,dict(id=f'FX-B06-{did}-{n:02}',description=p['title'],**p['fixture'],verification_method='Independent Python oracle in tools/verify_b06_fixtures.py; actual C/adapter execution separately recorded.'))
        rubric=[{'id':pid+'.'+chr(97+i),'objective':o['id'],'criterion':o['text'],'evidence':['D1','D2','D3'] if i<2 else ['D4','D5','D6'],'levels':{'0':'Missing artifact or incorrect contract.','1':'Visible example passes but invariant, failure or changed assumptions are unexplained.','2':o['text']+' Retain original source or derivation and counterexample.','3':p['variant']+' Complete unaided after seven days and revise the affected argument.'}} for i,o in enumerate(objectives)]
        base=dict(project=pid,title=p['title'],minutes=240,objectives=oids,tests=[testname],target_scope=d['scope'],scope=p['contract'],scaffolding='independent design with graduated hints',reasoning=p['reasoning'],deliverables=['D1 prediction/invariants','D2 source/build','D3 observations/oracle','D4 repair/regression','D5 transfer','D6 limitations'],rubric=rubric,variants=[dict(id=pid+'-transfer',minimum_days=7,task=p['variant'])],assessor_analysis=analysis,remaining_evidence=p.get('external') or d['remaining'],references=refs,artifact_inputs=[],artifact_outputs=[],validation_status='authored; campaign and learner evidence separate')
        u1=dict(base,id=f'LAB-{prefix}-{n:02}A',new_executable=True,kind='implementation',steps=[p['contract'],'Implement the declared core API in src/core.c; compare every supplied fixture and add a discriminating input.',p['reasoning']],checks=[p['failure'],'All declared CTest cases pass for the learner source, with current source hashes; a scaffold pass is not expected.'])
        u2=dict(base,id=f'LAB-{prefix}-{n:02}B',new_executable=False,kind='adaptation',steps=[p['failure'],p['variant'],'After seven days reconstruct the changed-condition argument without previous source.'],checks=[p['reasoning'],'Assess each objective using the project rubric, including all outstanding target observations.'])
        units += [u1,u2];rows.append(dict(id=pid,tests=[testname],capabilities=['c17','cmake'],scope=p['contract'],remaining_evidence=base['remaining_evidence']))
        meta=dict(id=pid,title=p['title'],module=did,topics=[tid],status='core',hours=8,profile=d['profile'],fixtures=[fx],requires_projects=([f'PR-{did}-{n-1:02}'] if n>1 else d.get('entry_projects',[])),edition=5,shared_contracts=['curriculum/v5/contracts/b06.md'])
        specific_refs=p.get('refs') or d['refs']
        text=f'''# {pid}: {p['title']}

## Contract

{p['contract']}

Shared requirements: [B06 exercise contract](../v5/contracts/b06.md). Stable
unit IDs are `{u1['id']}` and `{u2['id']}`. Families: {d.get('families','A, B, C, D, F, G, I, J')};
source records: {', '.join('R-B06-'+r for r in specific_refs)} in
`research/v5/b06_resources.json`, using only its inspected locators.

Prerequisite knowledge: {d['entry']}
Prerequisite learner artifacts: the source, contract argument and recorded
assessment of {', '.join(meta['requires_projects'])}. Keep these artifacts and
their evidence identities in D1; reading or copying their assessor solution
does not satisfy the prerequisite. Use the already accepted artifact interface
where integration is assigned; do not substitute a library for this mechanism.
Target-specific supplied materials are in `labs/{family}/targets/` and remain
subject to the named outstanding evidence requirements.

## Inputs and expected results

Use `{fx}` and the public declaration:

```c
{p['prototype']}
```

The fixture's independently recomputable expectation is
`{json.dumps(p['fixture']['expected'],ensure_ascii=False)}`. The harness contains
additional boundaries. Inputs/control/output objects must be live and disjoint
unless the API explicitly permits aliasing. Failure leaves caller output
unchanged unless partial progress is the declared result.

## Steps

- **{tid}:** {p['teach']}
- **{tid}:** {p['contract']}
- **{tid}:** {p['reasoning']}

## Failure cases

{p['failure']}

Retain the first divergent transition and distinguish the cause from a symptom.
The named mutation control must fail for its intended reason; compilation
failure is not a successful mutation kill.

## Acceptance and evidence

- **{tid}:** {p['reasoning']}
- **{tid}:** Pass `{testname}` using learner source, account for every output and error branch, and submit the six shared deliverables.
- **{tid}:** {p['variant']}

{base['remaining_evidence']}

### Objective rubric

'''+ '\n\n'.join('**'+o['id']+'** — '+o['text']+' Evidence: '+', '.join(r['evidence'])+'. Shared levels0–3 apply separately.' for o,r in zip(objectives,rubric))+f'''

## Hints and recovery

1. {p['hints'][0]}
2. {p['hints'][1]}
3. {p['hints'][2]}

The separate assessor analysis is `{analysis}`; it is excluded from ordinary
learner initialization. Reading it must be recorded as assisted practice.

## Transfer challenge

{p['variant']}

## Environment, security and validation

{d['environment']} {p.get('external') or d['remaining']}
The B06 shared containment contract applies. Content is authored and supplied
scaffolds are intentionally incomplete. Consult the source-bound campaign for
actual executions; no external reviewer, learner success or physical result is
inferred from a supplied reference implementation.
'''
        meta,text=attach_project_support(pid,meta,text)
        doc(f'curriculum/projects/{pid}.md',meta,text)
        body.append(f'### {tid} — {p["title"]}\n\n'+p['teach']+'\n\n'+p['reasoning']+'\n\nMisconception and counterexample: '+p['failure']+'\n\nInvestigation: '+p['contract']+'\n\nEvidence: '+p['variant']+'\n')
        header.append(p['prototype']);impl.append(p['implementation']);stubs.append(p.get('stub') or p['prototype'].rstrip(';')+' { '+p.get('stub_body','return 0;')+' }')
        test.append('static int test_'+p['slug']+'(void) {\n'+p['tests']+'\nreturn 0;\n}');dispatch.append('if(strcmp(argv[1],"'+p['slug']+'")==0) return test_'+p['slug']+'();');cmtests.append((n,p['slug']))
        write(analysis,f'# Assessor analysis: {pid}\n\n'+p['teach']+'\n\n'+p['reasoning']+'\n\nThe reference implementation is in `core.c`; inspect it together with the public test. The named fault is: '+p['failure']+'\n\nRequire the learner to derive '+str(p['fixture']['expected'])+' from the fixture inputs independently. '+p['hints'][2]+'\n\nTransfer answer obligations: '+p['variant']+'\n\n'+base['remaining_evidence']+'\n\n'+json.dumps(rubric,indent=2)+'\n')
        recall.append(dict(id=tid,title=p['title'],objectives=oids,objective_text={o['id']:o['text'] for o in objectives},projects=[pid],resource_ids=['R-B06-'+r for r in specific_refs],variants=[dict(id=tid+'-derive',kind='mechanism',objectives=[oids[0]],prompt=p['reasoning'],checklist=[p['hints'][2]]),dict(id=tid+'-repair',kind='trace-or-small-implementation',objectives=[oids[1]],prompt=p['failure'],checklist=[p['contract']]),dict(id=tid+'-transfer',kind='changed-condition',objectives=[oids[2]],prompt=p['variant'],checklist=[p['reasoning']])],grading='Per-objective evidence; consulted answers are assisted practice.'))
        for j,(term,definition,distinction) in enumerate(p['terms'],1):
            rid='R-B06-'+specific_refs[0]
            glossary.append(dict(id=f'V-{did}-{n:02}-{j}',term=term,owner=did,kind='course',definition=definition,authority=[dict(resource_id=rid,locator=sources[rid]['locators'][0])],edition_notes=[dict(edition='C17 baseline',note='This laboratory abstraction is a program contract; ISO C does not enforce its lifecycle or target-specific obligations.')],distinguish_from=[dict(term=distinction[0],distinction=distinction[1])],avoid=[],idiom=[term+' requires an explicit contract.'],symbols=[p['prototype'].split('(')[0].split()[-1]],objectives=oids,misconception=p['failure'],recall=dict(define='Define '+term+' with its invariant.',distinguish='Distinguish '+term+' from '+distinction[0]+'.',counterexample=p['variant'])))
    header+=['#endif'];write(f'labs/{family}/include/core.h','\n'.join(header)+'\n');write(f'labs/{family}/src/core.c','\n'.join(stubs)+'\n');write(f'assessor/{family}/core.c','\n'.join(impl)+'\n');test+=['int main(int argc,char **argv) { if(argc!=2) return 2;','\n'.join(dispatch),'return 2; }'];write(f'labs/{family}/tests/tests.c','\n'.join(test)+'\n')
    cm=f'''cmake_minimum_required(VERSION 3.16)
project({family} C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
enable_testing()
set(CORE src/core.c)
if(CMASTERY_ASSESSOR_ROOT)
  if(EXISTS "${{CMASTERY_ASSESSOR_ROOT}}/{family}/core.c")
    set(CORE "${{CMASTERY_ASSESSOR_ROOT}}/{family}/core.c")
  else()
    set(CORE "${{CMASTERY_ASSESSOR_ROOT}}/core.c")
  endif()
endif()
add_executable(b06-tests ${{CORE}} tests/tests.c)
target_include_directories(b06-tests PRIVATE include)
if(NOT MSVC)
  target_compile_options(b06-tests PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(b06-tests PRIVATE m)
endif()
'''
    for n,slug in cmtests:
        cm+=f'add_test(NAME {family}.{slug} COMMAND b06-tests {slug})\n'
        cm+=f'''if(CMASTERY_ASSESSOR_ROOT)
  add_executable(b06-bad-{n} ${{CORE}} tests/tests.c)
  target_include_directories(b06-bad-{n} PRIVATE include)
  target_compile_definitions(b06-bad-{n} PRIVATE B06_MUTATION={n})
  if(NOT MSVC)
    target_link_libraries(b06-bad-{n} PRIVATE m)
  endif()
  add_test(NAME {family}.mutation.{slug} COMMAND b06-bad-{n} {slug})
  set_tests_properties({family}.mutation.{slug} PROPERTIES WILL_FAIL TRUE)
endif()
'''
    write(f'labs/{family}/CMakeLists.txt',cm)
    write(f'labs/{family}/README.md',f'# {d["title"]} laboratory\n\n'+d['environment']+'\n\nPortable core controls are independent of required guest/board observations. Ordinary init copies compiling learner stubs, tests and contracts. Explicit `-DCMASTERY_ASSESSOR_ROOT=/absolute/repository/assessor` selects reference sources; never use that flag for learner evidence.\n\n'+d['remaining']+'\n\nSee the canonical project briefs and `CONTRACTS.md`.\n')
    write(f'labs/{family}/CONTRACTS.md',SHARED+'\n\n'+ '\n\n'.join('## '+str(i+1)+': '+p['title']+'\n\n'+p['contract']+'\n\n'+p['reasoning'] for i,p in enumerate(projects)))
    dump(f'labs/{family}/family.json',dict(version=5,family=family,title=d['title'],build_system='cmake',assessor_root='assessor/'+family,modules=[did],projects=rows,units=units,tests=[family+'.'+p['slug'] for p in projects],negative_controls=[family+'.mutation.'+p['slug'] for p in projects],remaining_evidence=d['remaining']))
    meta=dict(id=did,title=d['title'],track=d['track'],status='core',prerequisites=d['prereqs'],prelude_objectives=[],workload=dict(reading=len(refs),implementation=40,verification=24,assessment=16,consolidation=8),readings=refs,projects=[r['id'] for r in rows],profile=[d['profile']],equipment=d['equipment'],edition=5)
    doc(f'curriculum/modules/{did}.md',meta,f'# {did}: {d["title"]}\n\n## Purpose and outcomes\n\n'+d['purpose']+'\n\n## Entry check and remediation\n\n'+d['entry']+'\n\n## Study sequence and teaching\n\n'+'\n'.join(body)+'\n## Reading guidance\n\nRead only the exact registered passages; distinguish their authority from the original laboratory model. Record additional source gaps before changing a target assumption.\n\n## Implementation projects\n\n'+'\n'.join('- {ref}`'+r['id']+'`: '+projects[i]['title'] for i,r in enumerate(rows))+'\n\n## Mastery gate and feedback\n\nEvery objective needs its own D1–D6 evidence and rubric score. Model tests establish bounded behavior only. Repeat the named changed-condition problem after seven days; identify self-assessment versus an external reviewer.\n\n## Consolidation and transfer\n\n'+d['transfer']+'\n\n## Environment and limits\n\n'+d['environment']+' '+d['remaining']+' New hours are descriptive additions; accepted historical hours and receipts are immutable and do not limit study depth.\n')
    artifact=dict(id=d['artifact'][0],producer=f'PR-{did}-{d["artifact"][1]:02}',contract=d['artifact'][2],critical_objectives=[f'{did}.{d["artifact"][1]:02}.{s}' for s in 'abc'])
    domain=dict(family=family,prefix=prefix,phase=10,strand=d['strand'],imports=d.get('imports',{}),artifacts=[artifact],decisions={},research_gate='research/v5/expansion/b06/research_gate.json',hand_authored_units=True)
    dump(f'curriculum/v5/domains/{did}.json',dict(id=did,domain=domain,new_executable_counts={prefix:len(projects)},fixture_oracles=['verify_b06_fixtures.py']))
    dump(f'curriculum/v5/recall/{did}.json',dict(domain=did,sets=recall));dump(f'curriculum/vocabulary/{did}.json',dict(domain=did,entries=glossary,recall=dict(define='For '+d['title']+', define '+', '.join(e['term'] for e in glossary)+', giving each invariant.',distinguish='For '+d['title']+', distinguish these near concepts: '+'; '.join(e['term']+' versus '+e['distinguish_from'][0]['term'] for e in glossary)+'.',apply=d['transfer'])))
    return dict(id=did,title=d['title'],depth='derivation, implementation, falsification and delayed transfer',subtopics=topics)
def main():
    gate=json.loads((HERE/'research_gate.json').read_text(encoding='utf-8'))
    for rel,sha in gate['research_input_sha256'].items():
        if hashlib.sha256((ROOT/rel).read_bytes()).hexdigest()!=sha:raise ValueError('Research input changed: '+rel)
    write('curriculum/v5/contracts/b06.md',SHARED)
    domains=[author(load('spec_t'+str(n))) for n in range(57,65) if (HERE/('spec_t'+str(n)+'.py')).exists()]
    dump('research/v5/b06_taxonomy.json',dict(edition=5,domains=domains))
    print(json.dumps({'authored_domains':[d['id'] for d in domains],'projects':sum(len(d['subtopics']) for d in domains),'status':'authored; no execution or learner acceptance inferred'}))
if __name__=='__main__':main()
