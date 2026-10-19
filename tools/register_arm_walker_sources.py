"""Add the actually inspected DDI0487H.a walker source successor; no guest run."""
import argparse
import copy
import importlib.util
import json
from pathlib import Path

from common import ROOT, read_json, sha

BASE = 'research/v5/expansion/arm_walker_sources'
PDF = 'build/source-gap-final/ddi0487.pdf'
PDF_SHA = '62af4d2f908347c3018bd63572f0ce1d5edf0d34abae2bee3c18de7c97d06e43'
DOC = 'labs/architecture_ports_source/WALKER_MANUAL.md'
ORIGIN = 'research/v5/expansion/b06/gap_ledger.json'
PREVIOUS = 'research/v5/expansion/b06/completion_dispositions.json'
IDENT = 'G-B06-T58-07-TRANSFER'
RESOURCE = 'R-ARM-WALKER-DDI0487HA'
REPORTS = [f'research/v5/expansion/b06/{x}_validation.json'
           for x in ['walker', 'abi_guest']]
GROUPS = [
    ('identity', [1, 2, 3], 'Title, identifier, release history and EAC scope',
     'DDI0487H.a ID020222, 4 February 2022; full capture but bounded inspection.'),
    ('geometry', [4814, 4815, 4816, 4817, 4823],
     'D5.2.7 tables D5-19/D5-20; D5.2.8 table D5-24',
     'Selected 4KiB T0SZ25 starts at L1, indices38:30/29:21/20:12; table alignment and block sizes.'),
    ('descriptors', [4834, 4837, 4838, 4839, 4840, 4841, 4844, 4845],
     'D5.3.1/D5.3.2 and stage1 attribute diagram',
     'Valid/type distinctions; DS0 next-table and page output fields; AP/AF/nG/SH/AttrIndx.'),
    ('permissions', [4849, 4850, 4851, 4853, 4854, 4860, 4861, 4862],
     'D5.4.1-3, D5.4.5 table D5-29, D5.4.7-12',
     'EL1 versus EL0 AP permissions, independent execute controls, PAN/UAO, software AF versus optional hardware AF/dirty updates.'),
    ('memory', [4871, 4872, 5791, 5792, 5793],
     'D5.5.1 table D5-36 and D13.2.97 MAIR_EL1',
     'AttrIndx selects byte; ff normal WB non-transient RA/WA,04 Device-nGnRE; SH11 Inner Shareable.'),
    ('controls', [5946, 5953, 5954, 5955, 5956, 5957, 5958, 6014, 6015],
     'D13.2.131 TCR_EL1 and D13.2.144 TTBR0_EL1',
     'Selected lower fields decoded; historical reserved TG1=00/T1SZ0 corrected to TG1=10/T1SZ25 with EPD1=1; HA/HD0, IPS32, TTBR alignment/ASID/CnP.'),
    ('maintenance', [4911, 4912, 4913, 4914, 4915, 4916, 11358, 11359, 11360],
     'D5.10.1-2; K10.5.3',
     'Intermediate caching, local versus shareable invalidation, table-store completion, local ISB, concurrent break-before-make boundaries.'),
]
LIMITS = [
    'Only listed passages inspected from full DDI0487H.a; no whole-manual or latest-edition claim.',
    'Historical source/receipts preserved; active TCR upper fields corrected and33-command walker plus59-command guest campaigns repeated.',
    'Selected single-PE lower-region guest does not execute the disabled TTBR1 region; its granule/size fields now use defined encodings.',
    'No stage2, other granules, hardware AF/dirty updates, SMP shootdown, all fault priorities or physical qualification.',
    'Original independent changed-condition/delayed assessment remains not-assessed; external review remains not-run.',
]


def dump(path, value):
    p = ROOT / path
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')


def origin():
    return next(r for r in read_json(ROOT / ORIGIN)['gaps'] if r['id'] == IDENT)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--attach', action='store_true'); args = ap.parse_args()
    assert sha(ROOT / PDF) == PDF_SHA, 'exact downloaded edition bytes'
    pages = sorted({n for _, ns, _, _ in GROUPS for n in ns})
    page_pins = {f'build/arm_walker_sources/pdf-{n}.txt':
                 sha(ROOT / f'build/arm_walker_sources/pdf-{n}.txt') for n in pages}
    execution = []
    provider_pins = {}
    for path in REPORTS:
        d = read_json(ROOT / path)
        assert d['passed'] and d['input_sha256']
        assert all(sha(ROOT / p) == h for p, h in d['input_sha256'].items()), path
        provider_pins.update(d['input_sha256'])
        execution.append(dict(path=path, sha256=sha(ROOT / path),
                              recorded_at=d['recorded_at'],
                              commands=len(d['checks']), source_current=True,
                              execution_repeated=True))
    inspection = dict(schema_version=1, date='2026-10-18',
        source_id=RESOURCE, title='Arm Architecture Reference Manual for A-profile architecture',
        edition='ARM DDI0487H.a (ID020222), 4 February 2022',
        full_document_pages=11530, full_document_inspected=False,
        capture=dict(path=PDF, sha256=PDF_SHA,
                     download_record='build/source-gap-final/ddi0487-access.json'),
        method='Actual local PDF text read; selected descriptor/entry-address diagrams rendered and visually inspected.',
        passage_groups=[dict(id=i, pdf_pages=ns, printed_locator=loc, finding=f)
                        for i, ns, loc, f in GROUPS],
        rendered_pages_inspected=[4823, 4840, 4844],
        page_text_sha256=page_pins,
        existing_execution=execution,
        corrected_condition=dict(historical_tcr='0x803519', active_tcr='0x80993519',
            change='TG1 00->10; T1SZ0->25; lower fields and EPD1=1 unchanged',
            reason='Reserved TG1 selects an implementation-defined granule; disabled walk does not make the encoding defined.',
            history='reviews/v5/expansion/arm_walker_sources/before_register_correction/manifest.json'),
        unresolved_conditions=['disabled-upper-region-unexecuted', 'feature-and-boot-premises',
            'single-pe-maintenance', 'stage2-and-other-granules', 'physical-and-independent-evidence'],
        limits=LIMITS)
    dump(BASE + '/inspection.json', inspection)
    resource = dict(id=RESOURCE, title=inspection['title'], authors=['Arm Limited'],
        edition=inspection['edition'], year=2022, kind='architecture manual',
        url='https://documentation-service.arm.com/static/61fbe8f4fa8173727a1b734e',
        domains=['T58'], role='primary', assignable=True,
        access='Complete authoritative PDF locally available; only explicitly listed passages inspected.',
        reason='Resolve the named selected Arm translation-register/page-table source omission with bounded comparison to retained Sv39 teaching.',
        prerequisites=['Bit fields, privilege levels, immutable table ownership and translation-cache visibility'],
        locators=[loc + ': PDF ' + ','.join(map(str, ns)) for _, ns, loc, _ in GROUPS],
        inspection=dict(date='2026-10-18', method=inspection['method'],
            scope='Selected lower-region 4KiB EL1 stage1 walker/descriptor/permission/AF/attribute/maintenance passages only.',
            limits='; '.join(LIMITS), private_payload=PDF, local_sha256=PDF_SHA,
            receipt=BASE + '/inspection.json'))
    dump(BASE + '/resources.json', dict(schema_version=1, resources=[resource]))
    attachment = dict(schema_version=1, project='PR-T58-07',
        shared_contracts=[DOC], supplied_artifacts=['labs/architecture_ports/targets/WALKER.md'],
        support=dict(source_id=RESOURCE, register=BASE + '/resources.json',
                     original_gap=IDENT, contract=DOC),
        body='\n\n## Selected Arm manual follow-through\n\nRead the supplied `labs/architecture_ports_source/WALKER_MANUAL.md` with the original walker contract. Derive the actual corrected TCR, TTBR, MAIR and descriptor fields; explain the versioned disabled-TTBR1 correction and single-PE invalidation boundary. This source companion resolves the named selected-manual access omission while retaining the original implementation, rubric, delayed transfer and external evidence requirements. Only listed DDI0487H.a passages were inspected.\n')
    dump(BASE + '/attachment.json', attachment)
    if args.attach:
        spec = importlib.util.spec_from_file_location('b06_arm_attachment', ROOT / 'research/v5/expansion/b06/author_domains.py')
        module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
        brief = ROOT / 'curriculum/projects/PR-T58-07.md'
        old = brief.read_text(encoding='utf-8'); first, body = old.split('\n```\n', 1)
        meta = json.loads(first[len('```json\n'):]); meta, body = module.attach_project_support('PR-T58-07', meta, body)
        module.doc('curriculum/projects/PR-T58-07.md', meta, body)
    previous = next(r for r in read_json(ROOT / PREVIOUS)['updates'] if r['id'] == IDENT)
    row = copy.deepcopy(previous)
    row['originating_record'] = dict(path=ORIGIN, record=origin())
    row['previous_disposition'] = dict(path=PREVIOUS, record=previous)
    additions = [DOC, BASE + '/resources.json', BASE + '/inspection.json', BASE + '/attachment.json']
    for dim in ['teaching', 'implementation', 'author_validation']:
        row['dimensions'][dim]['evidence'] = list(dict.fromkeys(
            row['dimensions'][dim]['evidence'] + additions))
    row['dimensions']['teaching']['state'] = 'complete-for-selected-walker-comparison'
    row['dimensions']['author_validation']['state'] = 'source-current-execution-and-bounded-primary-inspection'
    row['artifacts'] = list(dict.fromkeys(row['artifacts'] + additions))
    row['remaining_author_work'] = []
    row['source_records'] = row.get('source_records', []) + [dict(id=RESOURCE,
        path=BASE + '/resources.json', passages=resource['locators'])]
    row['evidence_limits'] = LIMITS
    row['unresolved_conditions'] = inspection['unresolved_conditions']
    row['closure_rationale'] = ('The previously inaccessible selected Arm register/table/manual clauses are now actually inspected from the exact full-manual capture. The historical disabled reserved TG1/T1SZ configuration is corrected in the active guest and the bounded walker/guest campaigns rerun. Actual learner initialization includes the source companion through the canonical B06 producer. This closes that named author source omission, not every architecture extension, full-manual review, physical observation or original independent transfer.')
    pins = dict(provider_pins)
    for p in additions + REPORTS + [ORIGIN, PREVIOUS,
            'assessor/architecture_ports/targets/WALKER_ANALYSIS.md',
            'research/v5/expansion/b06/followup_resources.json',
            'tools/register_arm_walker_sources.py', 'tools/check_arm_walker_sources.py',
            'tools/check_arm_walker_export.py',
            'research/v5/expansion/b06/author_domains.py',
            'curriculum/projects/PR-T58-07.md',
            'reviews/v5/expansion/arm_walker_sources/before_register_correction/manifest.json',
            'build/source-gap-final/ddi0487-access.json']:
        pins[p] = sha(ROOT / p)
    pins[PDF] = PDF_SHA
    pins.update(page_pins)
    export = 'reviews/v5/expansion/arm_walker_sources/export_validation.json'
    if (ROOT / export).is_file():
        report = read_json(ROOT / export)
        assert report['passed'] and all(sha(ROOT / p) == h for p,h in report['source_sha256'].items())
        pins[export] = sha(ROOT / export); pins.update(report['source_sha256'])
        row['dimensions']['author_validation']['evidence'].append(export)
    dump(BASE + '/completion_dispositions.json', dict(schema_version=1,
        updates=[row], source_sha256=pins,
        scope='Additive selected-source successor; previous B06 evidence and independent obligations preserved.'))
    print(json.dumps(dict(passed=True, rows=1, pages=len(pages),
                         support=BASE + '/completion_dispositions.json')))


if __name__ == '__main__':
    main()
