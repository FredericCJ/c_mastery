"""Guard the exact selected Arm-source successor, not a new emulator campaign."""
import argparse
import copy
import json
import re

from common import ROOT, read_json, sha
from register_arm_walker_sources import BASE, DOC, IDENT, LIMITS, ORIGIN, PDF, PDF_SHA, PREVIOUS, RESOURCE, REPORTS


def decode(tcr):
    fields = dict(T0SZ=(0, 6), EPD0=(7, 1), IRGN0=(8, 2), ORGN0=(10, 2),
        SH0=(12, 2), TG0=(14, 2), T1SZ=(16, 6), A1=(22, 1), EPD1=(23, 1),
        TG1=(30, 2), IPS=(32, 3), AS=(36, 1), TBI0=(37, 1), TBI1=(38, 1),
        HA=(39, 1), HD=(40, 1), DS=(59, 1))
    return {k: (tcr >> lo) & ((1 << n) - 1) for k, (lo, n) in fields.items()}


EXPECTED = dict(T0SZ=25, EPD0=0, IRGN0=1, ORGN0=1, SH0=3, TG0=0,
    T1SZ=0, A1=0, EPD1=1, TG1=0, IPS=0, AS=0, TBI0=0, TBI1=0, HA=0, HD=0, DS=0)


def inspect(packet, inspection, resources):
    assert packet['source_sha256'] and all(sha(ROOT / p) == h for p, h in packet['source_sha256'].items()), 'byte identities'
    assert len(packet['updates']) == 1 and packet['updates'][0]['id'] == IDENT, 'exact parent'
    row = packet['updates'][0]
    original = next(r for r in read_json(ROOT / ORIGIN)['gaps'] if r['id'] == IDENT)
    prior = next(r for r in read_json(ROOT / PREVIOUS)['updates'] if r['id'] == IDENT)
    assert row['originating_record'] == dict(path=ORIGIN, record=original), 'original task preservation'
    assert row['previous_disposition'] == dict(path=PREVIOUS, record=prior), 'historical disposition preservation'
    assert set(row['dimensions']) == {'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}
    for key in ['external_review','independent_learner','physical_observation']:
        assert row['dimensions'][key] == prior['dimensions'][key], 'no external evidence promotion'
    assert row['evidence_limits'] == LIMITS and row['unresolved_conditions'] == inspection['unresolved_conditions'], 'scope retained'
    assert 'disabled-upper-region-encoding' in inspection['unresolved_conditions'], 'upper-region limit'
    assert 'single-pe-maintenance' in inspection['unresolved_conditions'], 'broadcast limit'
    assert inspection['full_document_inspected'] is False and inspection['full_document_pages'] == 11530, 'bounded inspection'
    assert inspection['capture']['path'] == PDF and inspection['capture']['sha256'] == PDF_SHA == sha(ROOT / PDF), 'exact PDF'
    assert inspection['edition'] == 'ARM DDI0487H.a (ID020222), 4 February 2022', 'edition'
    assert resources['resources'][0]['edition'] == inspection['edition'] and resources['resources'][0]['id'] == RESOURCE, 'resource identity'
    assert resources['resources'][0]['inspection']['local_sha256'] == PDF_SHA
    assert inspection['rendered_pages_inspected'] == [4823,4840,4844], 'actual visual scope'
    assert {g['id'] for g in inspection['passage_groups']} == {'identity','geometry','descriptors','permissions','memory','controls','maintenance'}
    assert all(sha(ROOT / p) == h for p, h in inspection['page_text_sha256'].items()), 'page capture identity'
    for rec, path in zip(inspection['existing_execution'], REPORTS, strict=True):
        d = read_json(ROOT / path)
        assert rec['path'] == path and rec['sha256'] == sha(ROOT / path), 'retained receipt'
        assert rec['original_recorded_at'] == d['recorded_at'] and rec['commands'] == len(d['checks'])
        assert rec['execution_repeated'] is False and rec['source_current'] is True, 'execution scope'
        assert d['passed'] and all(sha(ROOT / p) == h for p, h in d['input_sha256'].items()), 'current mechanism provider'
    asm = (ROOT / 'assessor/architecture_ports/guests/aa64.S').read_text()
    assert re.search(r'ldr\s+x0,\s*=0x803519\s*\n\s*msr\s+tcr_el1,\s*x0', asm, re.I), 'actual TCR constant'
    assert decode(0x803519) == EXPECTED
    assert [(0x80000000 >> n) & 511 for n in [30,21,12]] == [2,0,0]
    assert ((0x703 >> 6) & 3, (0x783 >> 6) & 3) == (0,2)
    assert all(((x >> 10) & 1, (x >> 8) & 3, (x >> 2) & 7, x & 3) == (1,3,0,3) for x in [0x703,0x783])
    assert (0x4ff & 255, (0x4ff >> 8) & 255) == (255,4)
    assert 'tlbi vmalle1' in asm.lower() and 'tlbi vmalle1is' not in asm.lower(), 'selected local invalidation'
    return dict(tcr_fields=EXPECTED, va_indices=[2,0,0],
                scope='Static derivation against actual constants and source passages; not guest execution.')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--report', required=True)
    ap.add_argument('--self-test', action='store_true')
    args = ap.parse_args()
    packet = read_json(ROOT / BASE / 'completion_dispositions.json')
    inspection = read_json(ROOT / BASE / 'inspection.json')
    resources = read_json(ROOT / BASE / 'resources.json')
    derivation = inspect(packet, inspection, resources)
    controls = []
    if args.self_test:
        for case in ['stale','missing','original','historical','learner','review','physical','unbounded','edition','page','rerun','upper','broadcast']:
            p, i, r = copy.deepcopy((packet, inspection, resources))
            if case == 'stale': p['source_sha256'][DOC] = '0' * 64
            elif case == 'missing': p['updates'] = []
            elif case == 'original': p['updates'][0]['originating_record']['record']['description'] = 'weaker replacement'
            elif case == 'historical': p['updates'][0]['previous_disposition']['record']['remaining_author_work'] = []
            elif case in ['learner','review','physical']:
                k = dict(learner='independent_learner',review='external_review',physical='physical_observation')[case]
                p['updates'][0]['dimensions'][k]['state'] = 'passed'
            elif case == 'unbounded': i['full_document_inspected'] = True
            elif case == 'edition': r['resources'][0]['edition'] = 'latest manual'
            elif case == 'page': i['page_text_sha256'][next(iter(i['page_text_sha256']))] = '0' * 64
            elif case == 'rerun': i['existing_execution'][0]['execution_repeated'] = True
            elif case == 'upper': i['unresolved_conditions'].remove('disabled-upper-region-encoding')
            elif case == 'broadcast': i['unresolved_conditions'].remove('single-pe-maintenance')
            try:
                inspect(p, i, r)
            except AssertionError:
                controls.append(dict(case=case, rejected=True))
            else:
                raise AssertionError('accepted negative control ' + case)
    out = ROOT / args.report
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(dict(passed=True, controls=controls, derivation=derivation,
        source_sha256={BASE + '/completion_dispositions.json':sha(ROOT / BASE / 'completion_dispositions.json')},
        evidence_limits=LIMITS), indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(passed=True, negative_controls=len(controls), report=args.report)))


if __name__ == '__main__':
    main()
