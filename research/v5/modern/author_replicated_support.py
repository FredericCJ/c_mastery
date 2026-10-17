"""T76 publication glue, including explicit real producer dependency selection."""
import hashlib
import re
from author_ad_mp_support import materialize as base_materialize
from render_ad_mp import R, read, write, dump

def materialize(data):
    imports=['storage.pages','codec.records','io.frame','ipc.local','testing.faults','persist.hamt']
    numerical={
      'log':dict(input={'events':[[3,7],[3,9]],'prefix':1,'key':3},expected={'value':7,'nodes':6}),
      'projection':dict(input={'deliveries':[[1,0,7],[1,0,7]]},expected={'count':1,'total':7}),
      'clocks':dict(input={'local':[10,2],'remote':[10,7],'wall':9},expected=[10,8]),
      'counters':dict(input={'positive':[5,0,0],'negative':[0,3,0]},expected=2),
      'sets':dict(input={'observed':[1],'concurrent':[256],'removed':[1]},expected={'live':[256]}),
      'sequence':dict(input={'nodes':[[1,0,65],[2,0,66],[3,1,67]],'removed':[1]},expected=[66,67]),
      'merkle':dict(input={'left':[1,2,3],'right':[1,2,9]},expected={'difference_mask':4}),
      'wire':dict(input={'bytes':[24,7,16,1,8,150,1]},expected=[8,150,1,16,1,24,7]),
      'replay':dict(input={'moves':[1,1,0,-1,1,0,1,-1]},expected={'tick':8,'position':2,'score':12,'random':2388811721})}
    for d in data:
        d['fixture']=numerical[d['case']]
        d['security']='Target only the original replicated laboratory and its separately selected semantic mutants, inside a local process with synthetic state/bytes and no external transport. The adversary may reorder, duplicate, omit or corrupt the admitted teaching messages and trigger the named allocation failures; native pointer fabrication, host policy changes and deployed targets are excluded. The defect and violated invariant are those in Verification and the separate analysis. A harmless success condition is the named wrong value, unauthorized state admission, duplicate application or failed rollback within this target. Exploitability requires the exact missing admission/identity/commit check and stated delivery/lifetime assumptions; a crash or sanitizer report is not arbitrary code execution. Repair the invariant, kill both semantic mutants, retain boundary regressions and state residual identity, resource, collision and concurrency limits. '+d['evidence']
    exports=[('log.event_store',1,'Original bounded16-event positioned memory log using persist.hamt snapshot roots. Append is failure-atomic in reachable memory; keys0..31, int32 values, arena lifetime explicit. No disk durability or full replay-effect atomicity implied.'),('crdt.state',5,'Original fixed2-writer/4-value observed-remove tagged set with16 lifetime add identities, retained tombstones and idempotent state/delta merge. PN/register contracts are supplied by PR-T76-04. No dynamic membership, Byzantine admission or safe garbage collection implied.')]
    base_materialize('T76','replicated','RP','Replicated state, event history and deterministic replay',data,['T20','T32','T47','T70'],imports,exports,dict(question='Choose the authority and commit boundary of your event log, snapshot and externally visible effects before seeing the failure campaign.',alternatives=['Immutable event authority with rebuildable projections and explicitly committed effects','Snapshot authority with a versioned recovery log and explicitly stated lost-update/replay policy'],discriminator='Seal the selected invariant and crash-point predictions before implementation; compare recovery after each cut.',reversal_trigger='A new non-idempotent receiver or cross-file durability boundary invalidates the assumed atomic commit.'),phase=12)
    path='labs/replicated/family.json';metadata=read(path)
    metadata['dependencies']=['dod']
    metadata['source_imports']=[dict(family='dod',artifact='persist.hamt',producer='PR-T70-07',learner_source='labs/dod/src/dod.c',include='labs/dod/include',assessor_source='assessor/dod/dod.c',contract='labs/dod/CONTRACTS.md')]
    metadata['campaign'].update(producer='tools/run_replicated.py',receipt='replicated_campaign.json',assertion_marker='RP_CHECK',profiles=['gcc-debug','gcc-release','gcc-asan','clang-debug','clang-release','clang-asan','gcc-tsan','clang-tsan'])
    dump(path,metadata)
    cm=(R/'labs/replicated/CMakeLists.txt').read_text(encoding='utf-8')
    cm=cm.replace('enable_testing()','''enable_testing()
find_package(Threads REQUIRED)
set(CMASTERY_DOD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../dod" CACHE PATH "Explicit completed persist.hamt learner workspace")
set(CMASTERY_DOD_ASSESSOR_ROOT "" CACHE PATH "Explicit assessor override for producer validation only")
set(STORAGE "${CMASTERY_DOD_ROOT}/src/dod.c")''')
    cm=cm.replace('set(CORE "${CMASTERY_ASSESSOR_ROOT}/replicated/replicated.c")','''set(CORE "${CMASTERY_ASSESSOR_ROOT}/replicated/replicated.c")
    if(NOT CMASTERY_DOD_ASSESSOR_ROOT)
      set(CMASTERY_DOD_ASSESSOR_ROOT "${CMASTERY_ASSESSOR_ROOT}/dod")
    endif()''')
    cm=cm.replace('function(configure_target target)','''if(CMASTERY_DOD_ASSESSOR_ROOT)
  if(EXISTS "${CMASTERY_DOD_ASSESSOR_ROOT}/dod/dod.c")
    set(STORAGE "${CMASTERY_DOD_ASSESSOR_ROOT}/dod/dod.c")
  else()
    set(STORAGE "${CMASTERY_DOD_ASSESSOR_ROOT}/dod.c")
  endif()
endif()
function(configure_target target)''')
    cm=cm.replace('PRIVATE include)','PRIVATE include "${CMASTERY_DOD_ROOT}/include")\n  target_link_libraries(${target} PRIVATE Threads::Threads)')
    cm=cm.replace('  if(CMASTERY_SANITIZE','  if(CMASTERY_TSAN AND NOT MSVC)\n    target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)\n    target_link_options(${target} PRIVATE -fsanitize=thread)\n  endif()\n  if(CMASTERY_SANITIZE')
    cm=cm.replace('${CORE} tests/','${CORE} ${STORAGE} tests/')
    write('labs/replicated/CMakeLists.txt',cm)
    header=(R/'labs/replicated/include/replicated.h').read_text(encoding='utf-8')
    stubs=['#include "replicated.h"','/* Unsolved scaffold: every admitted operation still needs implementation. */']
    for declaration in re.findall(r'^((?:int|void|uint32_t|uint64_t|int32_t|int64_t|size_t)\s+\w+\([^;]*\));',header,re.M):
        names=[]
        for parameter in declaration[declaration.index('(')+1:-1].split(','):
            match=re.search(r'(\w+)\s*(?:\[[^]]*\])?\s*$',parameter)
            if not match:raise ValueError(parameter)
            names.append(match.group(1))
        if declaration.startswith('void rp_log_init'):
            body='*log = (RPLog){0};'
        else:
            body=' '.join('(void)'+name+';' for name in names)
            body+=' '+('' if declaration.startswith('void ') else 'return RP_FAILED;' if declaration.startswith('int ') else 'return 0;')
        stubs.append(declaration+' { '+body+' }')
    write('labs/replicated/src/replicated.c','\n'.join(stubs)+'\n')
    contract=(R/'labs/replicated/CONTRACTS.md').read_text(encoding='utf-8')
    contract+='''
## Exact shared assumptions and composition

The target has 8-bit bytes, uint32_t/uint64_t and 32-bit int. Arithmetic in
uint32_t is modulo2^32; signed-overflow behavior is never an oracle. The active
edition is ISO C17. C11 already supplies the required unsigned/representation
mechanisms; no C23 feature or stronger guarantee is assumed. Published C17/C23
wording remains an explicit access gap, not an inspected source.

Zero-initialized projection, PN counter, set and sequence are admitted empty
states. Log initialization calls the imported HAMT initializer. Public structs
must remain constructor/operation-admitted: routines do not authenticate forged
control state. PN/set merge allows exact self-alias; other input/output/control
objects are disjoint unless a value parameter makes overlap irrelevant. NULL
data is allowed only at zero byte/element length; output/control pointers are
always live. All functions are serialized core operations. The projection test
has a real pthread mutex around the entire shared transition and joins threads
before destroying storage. This demonstrates that adapter only; unsynchronized
core calls are outside the contract. No callbacks occur in core mechanisms.

Single-writer replica identities cannot be cloned into independent writers.
All log positions refer to one externally identified immutable history. Every
HAMT root belongs to its live stable-address arena. Quiescent checkpoint staging
may roll back arena.used on failure only because no intermediate root escapes;
unreachable cells and diagnostic allocation counters need not be byte-identical.
The persist.hamt producer is linked from an explicitly selected completed
workspace, not reimplemented or silently replaced by an assessor library.

Default source is the unsolved learner scaffold. In an initialized standalone
workspace provide --dependency dod=COMPLETED_DOD_WORKSPACE, which selects
CMASTERY_DOD_ROOT. Author validation explicitly selects assessor implementations
for both families. Common assessor root and family-specific assessor root are
supported; the latter needs CMASTERY_DOD_ASSESSOR_ROOT explicitly. Never count an
assessor-selected run as learner evidence.

The supplied memory anchors execute real HAMT composition. Durable storage.pages,
codec.records, io.frame, ipc.local and the full prior T20 learner reducer adapter
are required named investigations with explicit acceptance obligations, but are
not claimed executed by this campaign. Retain their source identities, interface
adapters, short-I/O/crash/rollback tests and residual limits in learner evidence.
Security success conditions are synthetic invariant violations only; all state,
message and local transport experiments remain confined to original teaching
programs. No remote services, real data or host security changes are authorized.
'''
    write('labs/replicated/CONTRACTS.md',contract)
    paths=['research/v5/modern/replicated_resources.json','research/v5/modern/replicated_taxonomy.json','research/v5/modern/coverage/T76.json']
    dump('research/v5/modern/replicated_research_gate.json',dict(status='passage-inspected',domains=['T76'],reviewer='author inspection only; no independent review',research_input_sha256={p:hashlib.sha256((R/p).read_bytes()).hexdigest() for p in paths}))
