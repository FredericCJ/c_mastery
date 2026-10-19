"""Bind original copied-message and teardown criteria to scoped executed support."""
import argparse
from copy import deepcopy
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
BASE = Path(__file__).resolve().parent
SUPPORT = 'research/v5/expansion/legacy_concurrency_lifecycle/support.json'
CONTRACT = 'labs/concurrency_lifecycle_depth/CONTRACTS.md'
CAMPAIGN = 'reviews/v5/expansion/legacy_concurrency_lifecycle/campaign.json'
TARGET = 'reviews/v5/expansion/legacy_concurrency_lifecycle/target.json'
CASES = {
    'T51.06.a': ('queue', 'pool', 'Copied snapshot versus live borrowed pointer; owned-buffer transfer/cancellation/release variant.'),
    'T51.06.b': ('queue', 'waiting', 'Unchanged-output bounds, full/blocked/timeout/close-drain behavior; variable contiguous backing distinguishes fragmentation from capacity.'),
    'T51.06.c': ('threaded', 'ack', 'Actual three-producer/two-consumer delivery history; duplicate/pointer defects and bounded volatile application acknowledgement variant.'),
    'T51.14.a': ('life', 'waiting', 'Generation leases, admission closure and destruction after complete references; actual joined timer callback variant.'),
    'T51.14.b': ('record', 'robust', 'Poison/rollback/abandon plus actual Linux robust owner and repairer death; explicit two-attempt course policy.'),
    'T51.14.c': ('life', 'record', 'Blocked operations, pending reacquisition and timer obligations; dependency-ordered three-object restart and generation exhaustion.'),
}


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2)+'\n', encoding='utf-8', newline='\n')


def attach(record):
    if record['project'] not in ('PR-T51-06', 'PR-T51-14'):
        return record
    ids = []
    for criterion in record['criteria']:
        if criterion['objective'] in CASES:
            criterion['successor_support'] = {'path': SUPPORT, 'id': criterion['id']}
            ids.append(criterion['id'])
    addition = {'path': SUPPORT, 'project': record['project'], 'criterion_ids': ids,
                'scope': 'Serialized core plus actual pthread and Cortex-M4 emulator support; original CMX scheduler integration and historical observations remain distinct.'}
    additions = record.setdefault('additional_successor_support', [])
    for index, row in enumerate(additions):
        if row.get('path') == SUPPORT:
            additions[index] = addition
            break
    else:
        additions.append(addition)
    if CONTRACT not in record['shared_contracts']:
        record['shared_contracts'].append(CONTRACT)
    for path in sorted((ROOT/'labs/concurrency_lifecycle_depth').rglob('*')):
        if path.is_file() and '__pycache__' not in path.parts:
            name = path.relative_to(ROOT).as_posix()
            if name not in record['supplied_artifacts']:
                record['supplied_artifacts'].append(name)
    return record


def register():
    definitions = [
        ('ROBUST', 'Linux man-pages 6.19 robust mutex attributes',
         'https://man7.org/linux/man-pages/man3/pthread_mutexattr_setrobust.3.html',
         ['DESCRIPTION: PTHREAD_MUTEX_STALLED versus PTHREAD_MUTEX_ROBUST; EOWNERDEAD, consistency, ENOTRECOVERABLE and repairer death',
          'VERSIONS: process-shared exec behavior is a Linux extension'],
         'Selected actual primary manual text read; Linux comparison, not ISO C or all POSIX implementations.'),
        ('COND', 'POSIX.1-2017 condition waits, reproduced in the POSIX Programmer manual',
         'https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html',
         ['DESCRIPTION: atomic unlock/enrollment, spurious wakeups, dynamic mutex binding and cancellation reacquisition',
          'DESCRIPTION and RATIONALE Condition Wait Semantics: timeout may coincide with a true predicate'],
         'Selected actual 2017-edition text inspected; not relabeled as inspection of POSIX.1-2024.'),
        ('LOCK', 'POSIX.1-2017 mutex locking and robust acquisition',
         'https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html',
         ['DESCRIPTION: zero/EOWNERDEAD both acquire ownership; protected state repair and abandonment',
          'ERRORS: ENOTRECOVERABLE, EOWNERDEAD, EBUSY and non-EINTR returns'],
         'Selected actual primary interface text inspected; no independent mutex implementation proof.'),
        ('JUMP', 'WG14 N1570 longjmp automatic-object semantics',
         'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',
         ['7.13.2.1p2-4, printed page263 / PDF page281: changed nonvolatile automatic objects after longjmp'],
         'Actual WG14 primary PDF passage inspected after the optimized unwind failure; draft C11 text, not a fresh C17/C23 whole-standard inspection.'),
        ('COURSE', 'Original CMX and bounded T51 criterion contracts',
         'labs/execution/include/cmx.h',
         ['cmx.h opening serialization contract and cmx_queue representation',
          'assessor/execution/cmx.c: cmx_queue_init/send/recv/close/abort and cmx_mutex_close',
          'labs/execution/tests/contracts.c: queue and teardown',
          'PR-T51-06 and PR-T51-14 canonical briefs and all six original AX tasks/variants'],
         'Actual bounded course passages inspected; retained integer queue and planned-exit refusal are not byte ownership or unexpected owner-death recovery.'),
    ]
    resources = []
    for suffix,title,url,locators,limits in definitions:
        resources.append({'id':'R-LCL-'+suffix, 'title':title, 'authors':['Named primary document owner or C Mastery authors'],
                          'url':url, 'locators':locators, 'domains':['T51'],
                          'kind':'course' if suffix=='COURSE' else 'standard' if suffix=='JUMP' else 'manual',
                          'edition': 'As named; inspected 2026-10-17', 'access':'Actual bounded passages read',
                          'role':'oracle', 'assignable':True, 'reason':'Exact synchronization, ownership and evidence boundaries for the original criteria.',
                          'prerequisites':['C17 lifetimes and completed T51 condition/teardown prerequisite artifacts'],
                          'inspection':{'date':'2026-10-17', 'method':'Read actual named primary passages and bounded local course sources before binding.',
                                        'scope':'; '.join(locators), 'limits':limits}})
    write(BASE/'resources.json', {'schema_version':1, 'resources':resources,
          'local_platform_inspection': {'path':'/usr/include/pthread.h',
              'sha256':'5479c95276c6693fdd1baa448f5d457dee57fc02b31277bfa04df63cc0b26a94',
              'observed_library':'Ubuntu GLIBC 2.43-2ubuntu2.4',
              'passage':'C pthread_cleanup_push/pop macro branch using __sigsetjmp_cancel and __pthread_unwind_next, lines674-708',
              'limit':'Observed installed header, not a portable ISO implementation rule or separately reproduced distribution source build.'}})
    write(BASE/'research_gate.json', {'status':'passage-inspected', 'domains':['T51'],
          'inspection_kind':'Author primary-passage and local source inspection; no independent human review',
          'research_input_sha256':{(BASE/'resources.json').relative_to(ROOT).as_posix():hashlib.sha256((BASE/'resources.json').read_bytes()).hexdigest()}})


def main():
    register()
    import sys
    sys.path.insert(0,str(ROOT/'tools'))
    from check_concurrency_lifecycle import validate_campaign, validate_target
    campaign = json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8'))
    target = json.loads((ROOT/TARGET).read_text(encoding='utf-8'))
    validate_campaign(campaign)
    validate_target(target)
    updates = []
    paths = [CONTRACT, CAMPAIGN, TARGET, 'research/v5/expansion/legacy_concurrency_lifecycle/resources.json']
    for project in ('PR-T51-06','PR-T51-14'):
        path = ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
        record = attach(json.loads(path.read_text(encoding='utf-8')))
        write(path,record)
        paths.append(path.relative_to(ROOT).as_posix())
        for criterion in record['criteria']:
            objective = criterion['objective']
            if objective not in CASES:
                continue
            first,second,scope = CASES[objective]
            row = {'id':criterion['id'], 'project':project, 'original_source':path.relative_to(ROOT).as_posix(),
                   'original_objective':objective, 'closure_rationale':scope,
                   'dimensions':{
                       'teaching':{'state':'complete-for-declared-adapter-contract','evidence':[CONTRACT]},
                       'implementation':{'state':'complete-serialized-core-and-explicit-hosted-comparison','evidence':['assessor/concurrency_lifecycle_depth/core.c','assessor/concurrency_lifecycle_depth/posix.c','labs/concurrency_lifecycle_depth/tests/core.c','labs/concurrency_lifecycle_depth/tests/posix.c','assessor/concurrency_lifecycle_depth/ANALYSIS.md']},
                       'author_validation':{'state':'passed-hosted-and-emulated-core-with-stated-scope','evidence':[CAMPAIGN,TARGET]},
                       'external_review':{'state':'not-run','evidence':[]},
                       'independent_learner':{'state':'not-assessed','evidence':[]},
                       'physical_observation':{'state':'not-run','evidence':[]}},
                   'tests':['concurrency_lifecycle_depth.'+first,'concurrency_lifecycle_depth.'+second],
                   'required_environment':'C17 serialized core; Linux pthread comparison; isolated QEMU mps2-an386 for core target execution. Original profile E runtime-port obligations remain separate.',
                   'external_blockers':['Actual delayed independent learner assessment and external review.','Dedicated physical target for physical timing/IRQ claims.'],
                   'evidence_limits':['Hosted synchronization is not a first-principles mutex/scheduler replacement.','Emulated core execution does not demonstrate CMX blocking/IRQ integration.','Stable live registry, caller exclusion and adapter-completion assertions remain preconditions.','Finite traces/TSan do not prove fairness, arbitrary schedules, lock-free progress or mastery.']}
            updates.append(row)
            for variant in criterion['variants']:
                child=deepcopy(row)
                child.update(id=variant['id'], parent_id=criterion['id'], original_variant_id=variant['id'],
                             original_variant_task=variant['task'], minimum_days=variant['minimum_days'],
                             closure_rationale='Exact original changed-condition author reference: '+scope)
                updates.append(child)
        updates.append({'id':'G-LCON-'+project, 'project':project,
                        'original_source':'research/v5/expansion/legacy_concurrency/gap_ledger.json',
                        'dimensions':{'teaching':{'state':'complete-for-new-declared-contract','evidence':[CONTRACT]},
                                      'implementation':{'state':'partial-cmx-scheduler-adapter-remains','evidence':['assessor/concurrency_lifecycle_depth/core.c','assessor/concurrency_lifecycle_depth/posix.c']},
                                      'author_validation':{'state':'passed-scoped-hosted-and-emulated-core','evidence':[CAMPAIGN,TARGET]},
                                      'external_review':{'state':'not-run','evidence':[]},
                                      'independent_learner':{'state':'not-assessed','evidence':[]},
                                      'physical_observation':{'state':'not-run','evidence':[]}},
                        'closure_rationale':'The concrete missing byte-ownership/repair mechanisms now have reference code, original changed conditions and actual hosted/emulated core evidence. Full original runtime integration remains required.'})
    paths += [Path(__file__).relative_to(ROOT).as_posix(), 'tools/check_concurrency_lifecycle.py']
    write(BASE/'support.json', {'schema_version':1,'required':True,
          'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths},
          'updates':updates,
          'remaining_author_work':['Adapt the copied-message and lifecycle primitives to the retained CMX task/blocking/IRQ port and execute its cancellation/shutdown schedules; hosted calls and emulated serialized core are not this integration.','Audit and execute all remaining T51 project-specific obligations, then T52/T53; these six criteria do not close the whole domains.'],
          'remaining_external_work':['Independent human review, delayed unaided learner attempts and selected physical target observations.']})
    print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--register-only',action='store_true')
    if parser.parse_args().register_only:
        register()
    else:
        main()
