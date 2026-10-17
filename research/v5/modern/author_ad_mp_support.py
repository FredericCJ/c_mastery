"""Shared rendering mechanics for the original ADT/metaprogramming cards."""
import hashlib
import json
import re
from pathlib import Path
from render_ad_mp import author, dump, read, write
R=Path(__file__).resolve().parents[3]

def materialize(domain,family,prefix,title,data,prereqs,imports,exports,decision,phase=11):
    fixtures=[]
    for i,d in enumerate(data,1):
        fixtures.append(dict(kind=d['case'],example=d['example'],**d.get('fixture',dict(expected=d['example'])),oracle_scope='Case-specific independent oracle and public C boundary assertions; prose obligations remain separately assessed.'))
    dump(f'fixtures/{family}/cases.json',dict(schema_version=1,cases=fixtures))
    resource_path=f'research/v5/modern/{family}_resources.json'
    sources=[(resource_path,r) for r in read(resource_path)['resources']]
    if family=='metaprog':
        sources += [('research/v5/modern/adt_resources.json',r) for r in read('research/v5/modern/adt_resources.json')['resources'] if r['id']=='R-ADMP-C11']
    artifact_rows=[dict(id=aid,producer=f'PR-{domain}-{index:02}',contract=contract,critical_objectives=[f'{domain}.{index:02}.{c}' for c in 'abcd']) for aid,index,contract in exports]
    author(domain,family,prefix,title,data,prereqs,imports,artifact_rows,sources,phase=phase,campaign=f'research/v5/modern/{family}_validation.json')
    metadata=read(f'labs/{family}/family.json')
    metadata['campaign']=dict(profiles=['gcc-debug','gcc-release','gcc-asan','clang-debug','clang-release','clang-asan'],mutants_per_anchor=2,assertion_marker='ADT_CHECK' if family=='adt' else 'MP_CHECK',fixture_oracle=f'tools/verify_{family}_fixtures.py',oracle_driver='oracle-driver',report=f'reviews/v5/expansion/b07/{family}_campaign.json')
    dump(f'labs/{family}/family.json',metadata)
    fragment=read(f'curriculum/v5/domains/{domain}.json')
    fragment['domain']['imports']={f'PR-{domain}-{i:02}':d['imports'] for i,d in enumerate(data,1)}
    fragment['domain']['decisions']={f'PR-{domain}-01':decision}
    dump(f'curriculum/v5/domains/{domain}.json',fragment)
    entries=[]
    source_map={s['id']:s for _,s in sources}
    for i,d in enumerate(data,1):
        for j,(term,definition,other,difference) in enumerate(d['terms'],1):
            rid=d['refs'][0]
            entries.append(dict(id=f'V-{domain}-{i:02}-{j}',term=term,owner=domain,kind='programming-model',definition=definition,authority=[dict(resource_id=rid,locator=source_map[rid]['locators'][0])],edition_notes=[dict(edition='ISO C17 baseline',note='This is a stated abstraction contract, not an automatic C safety guarantee. Compare only the registered passages.')],distinguish_from=[dict(term=other,distinction=difference)],avoid=[],idiom=[f'For {d["case"]}, state {term} before claiming correctness.'],symbols=[d['case']],objectives=[f'{domain}.{i:02}.{c}' for c in 'abcd'],misconception=d['hints'][2],recall=dict(define='Define '+term+' and its invariant.',distinguish='Distinguish '+term+' from '+other+'.',counterexample=d['transfer'])))
    dump(f'curriculum/vocabulary/{domain}.json',dict(domain=domain,entries=entries,recall=dict(define='Define every representation and validity condition in this domain using its actual C declarations.',distinguish='Distinguish runtime admission, ISO C type constraints, optional diagnostics and caller lifetime obligations for each abstraction.',apply='Solve the named delayed variants and record a counterexample to a stronger claim for every exported artifact.')))
    contract='''# Original mechanism laboratory contract

Use ISO C17 with extensions disabled and -Wall -Wextra -Wpedantic -Werror.
Public src files are compiling unsolved learner stubs. Reference solutions and
semantic mutants reside only under assessor/. Explicit assessor selection must
never be used as learner evidence. Python, CMake, compilers and diagnostic
runtimes are host verification tools, not implementations of the taught mechanism.

Unless an operation explicitly states otherwise, all pointer/control/output
objects must be live, correctly aligned, initialized to their required
representation and disjoint from input/control storage. Nonzero extents require
real backing objects of that extent; numerical range checks cannot establish
that a fabricated pointer is valid. Null allowance is stated per operation.
Output failure atomicity refers to admitted inputs. There is no general promise
to diagnose invalid native pointers, data races or lifetime violations.

Views borrow stable backing storage. The caller retains ownership and must not
destroy, relocate or mutate it while the view's validated interpretation is in
use. Value copies of handles/views do not extend lifetime. Public representation
fields remain constructible in C; they are not unforgeable proof objects.
Operations are single-threaded unless explicitly synchronized by an external
contract; adding atomics to one counter does not provide reclamation safety.

No library may replace the specific constructor, validator, property runner,
token expander or graph mechanism being implemented. Declared prerequisite
components may be reused through their documented contracts. At final
composition, retain the actual learner source identities and an adapter test;
merely listing an import does not establish working integration.

All security investigations use only these original local teaching programs,
synthetic bytes and local state. A harmless violation is an invalid input
accepted, a wrong synthetic value returned, or a stated property falsified.
This does not establish arbitrary code execution, privilege escalation, or a
property of any deployed program. Never use third-party services or real data.

Every project requires D1-D4 from its exercise, all thirteen dimensions in
modern-cost-contract.md, a prediction before execution, exact tool/source
identity and independent expected results. Level2 requires the entire written
contract and original implementation/analysis; public tests alone earn at most
level1. Level3 requires a delayed unaided changed-condition attempt after at
least seven days. Record assistance and distinguish self-report from review.
Author campaigns are not learner acceptance, independent human assessment,
physical validation or proof of absolute mastery.
'''
    for i,d in enumerate(data,1):
        contract+=f'\n## PR-{domain}-{i:02}: {d["title"]}\n\n{d["problem"]}\n\n{d["construct"]}\n'
    write(f'labs/{family}/CONTRACTS.md',contract)
    write(f'labs/{family}/README.md',f'# {title}\n\nC17 original teaching mechanisms. Use the canonical projects and exercises together with CONTRACTS.md. Default builds select unsolved learner source; `-DCMASTERY_ASSESSOR_ROOT=/absolute/repository/assessor` explicitly selects assessor source for author validation. Do not copy it into learner work.\n\nCompile/execute records are in research/v5/modern/{family}_validation.json when present; independent learner attempts and delayed variants remain required.\n')
    cm=f'''cmake_minimum_required(VERSION 3.16)
project({family} C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
enable_testing()
set(CORE src/{family}.c)
if(CMASTERY_ASSESSOR_ROOT)
  if(EXISTS "${{CMASTERY_ASSESSOR_ROOT}}/{family}/{family}.c")
    set(CORE "${{CMASTERY_ASSESSOR_ROOT}}/{family}/{family}.c")
  else()
    set(CORE "${{CMASTERY_ASSESSOR_ROOT}}/{family}.c")
  endif()
endif()
function(configure_target target)
  target_include_directories(${{target}} PRIVATE include)
  if(NOT MSVC)
    target_compile_options(${{target}} PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
  if(CMASTERY_SANITIZE AND NOT MSVC)
    target_compile_options(${{target}} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie)
    target_link_options(${{target}} PRIVATE -fsanitize=address,undefined -no-pie)
  endif()
endfunction()
add_executable(mechanism-tests ${{CORE}} tests/tests.c)
configure_target(mechanism-tests)
add_executable(oracle-driver ${{CORE}} tests/oracle_driver.c)
configure_target(oracle-driver)
'''
    for i,d in enumerate(data,1):
        cm+=f'add_test(NAME {family}.{d["case"]} COMMAND mechanism-tests {d["case"]})\n'
        for n in [2*i-1,2*i]:
            cm+=f'''if(CMASTERY_ASSESSOR_ROOT)
  add_executable(mutant-{n} ${{CORE}} tests/tests.c)
  configure_target(mutant-{n})
  target_compile_definitions(mutant-{n} PRIVATE MOD_MUTATION={n})
  add_test(NAME {family}.mutation.{d['case']}.{n} COMMAND mutant-{n} {d['case']})
  set_tests_properties({family}.mutation.{d['case']}.{n} PROPERTIES WILL_FAIL TRUE)
endif()
'''
    write(f'labs/{family}/CMakeLists.txt',cm)
    header=(R/f'labs/{family}/include/{family}.h').read_text(encoding='utf-8')
    declarations=re.findall(r'^((?:int|void|uint32_t|uint64_t|size_t|ADPair|ADValue|ADPacketError|MPPair|MPSum|MPToken|MPArg)\s+\w+\([^;]*\));',header,re.M)
    stubs=[f'#include "{family}.h"','/* Intentionally incomplete; implement the canonical exercise contracts. */']
    for declaration in declarations:
        signature=declaration.strip();params=signature[signature.index('(')+1:-1].split(',')
        names=[]
        for param in params:
            if param.strip()=='void':continue
            name=re.search(r'(\w+)\s*(?:\[[^]]*\])?\s*$',param)
            if not name: raise ValueError(param)
            names.append('(void)'+name.group(1)+';')
        ret=signature.split()[0]
        value={'void':'','ADPair':'return (ADPair){0,0};','ADValue':'return (ADValue){AD_EMPTY,{.number=0}};','ADPacketError':'return AD_PACKET_HEADER;','MPPair':'return (MPPair){0,0};','MPSum':'return (MPSum){0,0};','MPToken':'return (MPToken){0,0,0};','MPArg':'return (MPArg){0,0,NULL};'}.get(ret,'return 0;')
        stubs.append(signature+' { '+' '.join(names)+' '+value+' }')
    write(f'labs/{family}/src/{family}.c','\n'.join(stubs)+'\n')
    costs='# Mechanism-specific cost and enforcement review\n\nComplete measured columns with compiler flags, architecture, workload and equivalent correctness contracts. The following are analytical expectations, not invented measurements.\n'
    dimensions=['representation','reachable storage','lifetime','mutation and aliasing','allocation','movement and copying','destruction','dispatch','concurrency','failure','compilation','binary and stack','boundary']
    for i,d in enumerate(data,1):
        costs+=f'\n## PR-{domain}-{i:02}: {d["title"]}\n\n{d["costs"]}\n\n'
        for dimension in dimensions:
            tasks={
             'representation':'List exact fields, sizeof/alignment/padding observations and the abstraction function; distinguish ISO constraints from ABI observations.',
             'reachable storage':'Draw pointers/indices and the retained backing extent; include storage kept alive by a borrow even when owned bytes are zero.',
             'lifetime':'State creation, invalidation and final permitted access for each representation; construct a counterexample to any automatic-lifetime claim.',
             'mutation and aliasing':'Identify all writable fields and permitted aliases; distinguish disjoint API preconditions from constraints actually diagnosed by C.',
             'allocation':'Count allocation requests including prerequisite adapters; bounded inline storage is zero dynamic allocation, not zero resource use.',
             'movement and copying':'Count copied payload bytes and metadata moves; show whether a copy changes ownership, identity or borrow validity.',
             'destruction':'Account for every resource on success and failure; a borrowed/trivial value has no owned destructor but still has an invalidation point.',
             'dispatch':'Identify direct branches, function pointers or generated specializations; inspect release assembly before claiming dispatch elimination.',
             'concurrency':'The supplied operations are single-threaded; prove absence of shared mutation or provide synchronization and reclamation for the required transfer. No TSan pass is inferred.',
             'failure':'List each admitted failure and exact post-state; assess a boundary counterexample and distinguish caller-contract violations from operational failure.',
             'compilation':'Record preprocessing/compilation time and diagnostic controls; generated source size and type-checking are distinct from runtime safety.',
             'binary and stack':'Record object text/data, compiler stack-use report and call depth; compare under identical semantics, not a weaker unchecked implementation.',
             'boundary':'Name the language-level counterpart, identify ISO/diagnostic/runtime/external-verification/caller enforcers and falsify one overstated equivalence.'}
            costs+=f'- **{dimension}:** {tasks[dimension]}\n'
        costs+='\nSpecific falsification and transfer: '+d['evidence']+' '+d['transfer']+'\n'
    write(f'labs/{family}/COST_REVIEW.md',costs)
    cov=read(f'research/v5/modern/coverage/{domain}.json')
    cov['swe']=[dict(element=key,disposition='project',target=f'PR-{domain}-{i:02}',exercise=f'X-{domain}-{i:03}',rubric=[f'X-{domain}-{i:03}-R{j}' for j in range(1,5)],reason=d['claim'],status='authored; independent assessment pending') for i,d in enumerate(data,1) for key in d['census'] if not re.fullmatch(r'[A-Z]\d\d',key)]
    cov['census']=[row for row in cov['census'] if re.fullmatch(r'[A-Z]\d\d',row['technique'])]
    dump(f'research/v5/modern/coverage/{domain}.json',cov)
    paths=[resource_path,f'research/v5/modern/{family}_taxonomy.json',f'research/v5/modern/coverage/{domain}.json']
    if family=='metaprog':paths.append('research/v5/modern/adt_resources.json')
    dump(f'research/v5/modern/{family}_research_gate.json',dict(status='passage-inspected',domains=[domain],reviewer='author inspection only; no independent review',research_input_sha256={p:hashlib.sha256((R/p).read_bytes()).hexdigest() for p in paths}))
    print(json.dumps(dict(domain=domain,projects=len(data),objectives=len(data)*4,units=len(data)*2,terms=len(entries),status='authored; no learner assessment inferred')))
