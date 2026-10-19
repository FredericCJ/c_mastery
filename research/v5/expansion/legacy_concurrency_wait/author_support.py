"""Bind actual CMX wait/IRQ composition to six original criteria and variants."""
import hashlib
import json
from copy import deepcopy
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
BASE = Path(__file__).resolve().parent
SUPPORT = 'research/v5/expansion/legacy_concurrency_wait/support.json'
CONTRACT = 'labs/concurrency_lifecycle_depth/wait/CONTRACTS.md'
IRQ = 'labs/concurrency_lifecycle_depth/wait/IRQ.md'
CAMPAIGN = 'reviews/v5/expansion/legacy_concurrency_wait/campaign.json'
TARGET = 'reviews/v5/expansion/legacy_concurrency_wait/target.json'
PROJECTS = ('PR-T51-01', 'PR-T51-13')
CLAIMS = {
    'T51.01.a': 'Actual CMX predicate/enrollment transition, independently checked split-check control and real Cortex-M4 saved-mask IRQ producer.',
    'T51.01.b': 'Caller-owned node with task/object/operation generations; duplicate/saturated enrollment and stale callback after actual CMX exit/reap.',
    'T51.01.c': '32768 independent finite histories, two consumers and actual two-CPU pthread publication around CMX logical tasks; no weak-memory proof.',
    'T51.13.a': 'All24 ready/timeout/cancel/close orders include all six three-event orders; strict immediate deadline variant and modular clock boundary are executed.',
    'T51.13.b': 'Actual CMX donation and mutex handoff are preserved against stale operation cleanup;200 old callbacks execute after new enrollment on pinned native workers.',
    'T51.13.c': 'Independent FIFO/credit oracle checks163840 transitions including close; duplicate-completion and stale-predicate controls fail. Reduction limits are explicitly derived.'}


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')


def attach(record):
    if record['project'] not in PROJECTS:
        return record
    ids = []
    for criterion in record['criteria']:
        if criterion['objective'] in CLAIMS:
            ids.append(criterion['id'])
            criterion['successor_support'] = {'path': SUPPORT, 'id': criterion['id']}
    additions = record.setdefault('additional_successor_support', [])
    additions[:] = [r for r in additions if r.get('path') != SUPPORT]
    additions.append({'path': SUPPORT, 'project': record['project'], 'criterion_ids': ids,
                      'scope': 'Actual CMX import, generation wrapper, finite oracle, pinned pthread and isolated M4 IRQ observations; no physical or independent learner claim.'})
    for contract in (CONTRACT, IRQ):
        if contract not in record['shared_contracts']:
            record['shared_contracts'].append(contract)
    public = [p.relative_to(ROOT).as_posix() for p in (ROOT / 'labs/concurrency_lifecycle_depth/wait').iterdir() if p.is_file()]
    public += ['labs/execution/include/cmx.h', 'labs/execution/src/cmx.c', 'labs/execution/src/support.c',
               'labs/execution/README.md', 'labs/concurrency_lifecycle_depth/targets/m4.ld']
    for name in public:
        if name not in record['supplied_artifacts']:
            record['supplied_artifacts'].append(name)
    return record


def register():
    definitions = [
        ('CMX', 'Original CMX synchronization, port and declared compiler-helper implementation', 'labs/execution/README.md',
         ['cmx.c: wait_on/ready/priority, tick/cancel, semaphore/mutex, exit/reap; header serialization and CMX_BLOCKED contract',
          'cortex_port.c cmx_port_lock/unlock and handler/nonzero-mask blocking refusal',
          'support.c complete original byte routines and AEABI memory aliases'],
         'Original course-source implementation evidence, not an external standard or all scheduler behavior.'),
        ('COND', 'POSIX.1-2017 condition waits reproduced in the POSIX Programmer manual',
         'https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html',
         ['DESCRIPTION: locked mutex, atomic release/enrollment, reacquisition, predicate recheck and dynamic mutex binding'],
         'Selected manual passage; the native CMX adapter has its own mutex discipline, not a claim that every signal must hold that mutex.'),
        ('AFFINITY', 'Linux man-pages6.19 pthread affinity',
         'https://man7.org/linux/man-pages/man3/pthread_setaffinity_np.3.html',
         ['SYNOPSIS/DESCRIPTION and RETURN VALUE: set/get thread affinity', 'STANDARDS/NOTES: GNU extension, allowed-set intersection and inherited affinity'],
         'Observed Linux placement only; affinity does not establish memory ordering or physical simultaneous execution.'),
        ('MASK', 'CMSIS-Core configurable exception masking and NVIC pending state',
         'https://arm-software.github.io/CMSIS_6/main/Core/group__Core__Register__gr.html',
         ['__disable_irq, __get_PRIMASK and __set_PRIMASK: configurable exceptions, NMI/HardFault exceptions and privilege',
          'https://arm-software.github.io/CMSIS_6/main/Core/group__NVIC__gr.html NVIC_SetPendingIRQ: device interrupt pending bit'],
         'Actual selected current documentation read2026-10-17; no board-specific peripheral or timing claim.'),
        ('NVIC', 'CMSIS6 core_cm4.h register layout and pending operation',
         'https://raw.githubusercontent.com/ARM-software/CMSIS_6/main/CMSIS/Core/Include/core_cm4.h',
         ['NVIC_Type ISER offset0 and ISPR offset0x100; SCS_BASE0xE000E000 and NVIC_BASE+0x100',
          '__NVIC_SetPendingIRQ and __NVIC_ClearPendingIRQ register accesses'],
         'Selected upstream header passages read2026-10-17, not imported as a target dependency or a complete Arm manual inspection.')]
    resources = []
    for suffix, title, url, locators, limits in definitions:
        resources.append({'id': 'R-LWAIT-' + suffix, 'title': title, 'authors': ['C Mastery' if suffix == 'CMX' else 'Named primary specification/documentation authors'],
            'url': url, 'kind': 'course' if suffix == 'CMX' else 'primary', 'edition': 'As identified in title and inspected locator',
            'domains': ['T51'], 'locators': locators, 'access': 'Actual bounded source passages inspected', 'role': 'oracle',
            'assignable': True, 'reason': 'Support exact generational enrollment, publication and IRQ transfer arguments.',
            'prerequisites': ['C lifetime/ownership and the parent CMX contracts'],
            'inspection': {'date': '2026-10-17', 'method': 'Read actual bounded source/manual text before adapter or target authorship.',
                           'scope': '; '.join(locators), 'limits': limits}})
    write(BASE / 'resources.json', {'schema_version': 1, 'resources': resources})
    write(BASE / 'research_gate.json', {'status': 'passage-inspected', 'domains': ['T51'],
        'inspection_kind': 'Author inspection, not independent review', 'research_input_sha256': {
        'research/v5/expansion/legacy_concurrency_wait/resources.json': hashlib.sha256((BASE / 'resources.json').read_bytes()).hexdigest()}})


def main():
    register()
    host = json.loads((ROOT / CAMPAIGN).read_text(encoding='utf-8'))
    target = json.loads((ROOT / TARGET).read_text(encoding='utf-8'))
    if (not host.get('passed') or len(host['profiles']) != 8 or sum(len(p['outcomes']) for p in host['profiles']) != 192
            or not target.get('passed') or len(target['outcomes']) != 21):
        raise ValueError('Incomplete host/target wait campaign')
    for report in (host, target):
        for path, digest in report['inspected_paths'].items():
            if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
                raise ValueError('Stale wait evidence: ' + path)
    updates = []
    canonical_paths = []
    for project in PROJECTS:
        path = ROOT / 'curriculum/v5/expansion/legacy_assessments' / (project + '.json')
        record = attach(json.loads(path.read_text(encoding='utf-8')))
        write(path, record)
        canonical_paths.append(path.relative_to(ROOT).as_posix())
        for criterion in record['criteria']:
            objective = criterion['objective']
            row = {'id': criterion['id'], 'original_source': path.relative_to(ROOT).as_posix(),
                'project': project, 'original_objective': objective, 'closure_rationale': CLAIMS[objective],
                'dimensions': {
                    'teaching': {'state': 'complete-for-declared-contract', 'evidence': [CONTRACT, IRQ]},
                    'implementation': {'state': 'complete-for-declared-reference-and-changed-condition',
                        'evidence': ['assessor/concurrency_lifecycle_depth/wait.c', 'assessor/concurrency_lifecycle_depth/wait_irq.c', 'assessor/execution/cmx.c']},
                    'author_validation': {'state': 'passed-with-stated-scope', 'evidence': [CAMPAIGN, TARGET]},
                    'external_review': {'state': 'not-run-for-this-successor', 'evidence': []},
                    'independent_learner': {'state': 'not-assessed', 'evidence': []},
                    'physical_observation': {'state': 'not-run-no-physical-claim', 'evidence': []}},
                'variant_ids': [v['id'] for v in criterion['variants']],
                'required_environment': 'Explicit completed CMX import; hosted Linux with two allowed CPUs; owned QEMU mps2-an386 and bare-metal toolchains for IRQ transfer.',
                'external_blockers': ['Independent review, delayed learner attempt and physical measurements if making those claims.'],
                'evidence_limits': ['Native exclusion surrounds logical CMX tasks; target proves configurable IRQ/mask ordering, not a new context-switch runtime.',
                                   'Finite histories and TSan are not universal weak-memory, scheduling, timing or mastery proofs.',
                                   'Only these six original criteria and variants; other T51 mechanisms remain required.']}
            updates.append(row)
            for variant in criterion['variants']:
                child = deepcopy(row)
                child.update(id=variant['id'], parent_id=criterion['id'], original_variant_id=variant['id'],
                    original_variant_task=variant['task'], minimum_days=variant['minimum_days'], variant_ids=[variant['id']],
                    variant_readiness='Actual bounded author implementation/observations; original unaided delayed learner assessment remains unperformed.')
                updates.append(child)
        gap = deepcopy(updates[-2])
        gap.pop('original_objective', None)
        gap.update(id='G-LCON-' + project, original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
                   closure_rationale='Exact three criterion/variant successors supplied; learner/reviewer/physical dimensions remain separate.',
                   criterion_ids=[c['id'] for c in record['criteria']],
                   variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
        updates.append(gap)
    pins = [CAMPAIGN, TARGET, CONTRACT, IRQ, *canonical_paths,
            'research/v5/expansion/legacy_concurrency_wait/author_support.py',
            'research/v5/expansion/legacy_concurrency_wait/resources.json']
    write(ROOT / SUPPORT, {'schema_version': 1, 'required': True, 'updates': updates,
        'source_sha256': {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in pins},
        'scope': 'Exact original T51.01/T51.13 criteria and delayed variants; retained baseline and gap snapshots unchanged.',
        'remaining_author_work': [], 'remaining_external_work': ['Independent delayed learner attempts, external review and physical observations where claimed.']})
    path = ROOT / 'labs/concurrency_lifecycle_depth/family.json'
    family = json.loads(path.read_text(encoding='utf-8'))
    campaigns = family.setdefault('supplemental_campaigns', [])
    campaigns[:] = [row for row in campaigns if row.get('id') not in ('generational-cmx-wait', 'wait-irq-target')]
    campaigns += [{'id': 'generational-cmx-wait', 'producer': 'tools/run_wait_adapter.py', 'report': CAMPAIGN, 'contract': CONTRACT,
                   'scope': 'Exact T51.01/T51.13; actual CMX import, finite oracle and two-CPU pthread adapter.'},
                  {'id': 'wait-irq-target', 'producer': 'tools/run_wait_target.py', 'report': TARGET, 'contract': IRQ,
                   'scope': 'Actual isolated Cortex-M4 IRQ0 and saved-mask nesting with declared course memory helpers.'}]
    write(path, family)
    print(json.dumps({'updates': len(updates), 'criteria': 6, 'variants': 6}))


if __name__ == '__main__':
    main()
