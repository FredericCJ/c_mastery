"""Bind the exact cold-start successor without replacing historical B06 evidence."""
import copy
import json
import pathlib
import subprocess
from run_cacheable_boot_depth import ROOT, REPORT, sha
from check_cacheable_boot_depth import validate

HERE = ROOT / 'research/v5/expansion/cacheable_boot_depth'
REVIEW = ROOT / 'reviews/v5/expansion/cacheable_boot_depth'


def read(p):
    return json.loads(p.read_text(encoding='utf8'))


def current(d):
    assert d['passed'] and d.get('source_stable', True)
    for p, digest in d.get('input_sha256', {}).items():
        assert sha(ROOT / p) == digest, p


def main():
    campaign = read(REPORT)
    assert not validate(campaign)
    export = read(REVIEW / 'export_validation.json')
    current(export)
    guard = read(REVIEW / 'support_validation.json')
    assert guard['passed'] and guard['report_sha256'] == sha(REPORT)
    costs = []
    for target in campaign['target_audits']:
        elf = ROOT / target['elf']
        command = ['arm-none-eabi-size', '-A', str(elf)]
        result = subprocess.run(command, text=True, capture_output=True, check=True)
        frames = []
        for p in sorted(elf.parent.glob(elf.stem + '-*.su')):
            frames.append(dict(path=p.relative_to(ROOT).as_posix(), sha256=sha(p), raw=p.read_text(encoding='utf8')))
        assert len(frames) == 6, (elf, len(frames))
        costs.append(dict(elf=target['elf'], sha256=sha(elf), command=command,
                          stdout=result.stdout, compiler_stack_records=frames))
    cost_path = REVIEW / 'cost_observations.json'
    cost_path.write_text(json.dumps(dict(passed=True, targets=costs,
        limits='Raw section sizes and per-function compiler stack records only. These are not a whole-call-path bound, physical latency or WCET proof.'), indent=2) + '\n', encoding='utf8')
    reports = [p.relative_to(ROOT).as_posix() for p in [REPORT, REVIEW / 'support_validation.json', REVIEW / 'export_validation.json', cost_path]]
    row = copy.deepcopy(read(HERE / 'before_correction.json'))
    docs = ['labs/cacheable_boot_depth/CONTRACTS.md', 'labs/cacheable_boot_depth/BOARD_CAPTURE.md', 'assessor/cacheable_boot_depth/ANALYSIS.md']
    code = [p for p in campaign['input_sha256'] if p.startswith(('labs/cacheable_boot_depth/', 'assessor/cacheable_boot_depth/'))]
    row['dimensions']['teaching']['evidence'] += docs
    row['dimensions']['implementation'] = dict(state='complete-for-selected-cacheable-cold-start', evidence=row['dimensions']['implementation']['evidence'] + code)
    row['dimensions']['author_validation'] = dict(state='passed-native-target-audit-and-original-model', evidence=row['dimensions']['author_validation']['evidence'] + reports)
    row['dimensions']['physical_observation'] = dict(state='not-run', evidence=['labs/cacheable_boot_depth/BOARD_CAPTURE.md'])
    row['closure_rationale'] = 'The previously missing cacheable cold-start integration is now implemented: exact shared linker reservation, separate M7 WBWA/M4 noncached MPU maps, full set/way invalidation, dirty-line initialization/clean and one-way peer release. Actual native/model executions and selected target links are separate from required physical observations.'
    row['remaining_author_work'] = []
    row['remaining_external_evidence'] = [
        'Run the dedicated-board capture under separately authorized provisioning; identify actual silicon/revision and applicable errata, then capture physical cache/HSEM and device-quiescence behavior.',
        'Independent delayed learner reconstruction and external review have not occurred.'
    ]
    row['artifacts'] += docs + code
    row['evidence_limits'] += campaign['limitations'] + [
        'The eight selected H745 images are compiled/linked only. Four MPS2 executions run an original register/cache-state model on Cortex-M CPUs, not STM32 cache/HSEM hardware.',
        'The selected cold-start premise forbids DMA or other masters owning the line; previous DMA reservation requirements remain caller obligations.',
        'A bounded author-team source review found a 9..15 MPU-region admission mismatch. Exact 8-or-16 admission, rejection regression and a causal mutant now pass; this is not independent human review.'
    ]
    local = read(HERE / 'resources.json')
    row['source_records'] += [dict(id=r['id'], path='research/v5/expansion/cacheable_boot_depth/resources.json', passages=r['locators']) for r in local['resources']]
    for criterion in row['criterion_bindings']:
        criterion['teaching'] += docs
        criterion['implementation'] += code
        criterion['author_validation'].append(dict(path=reports[0], scope='Selected cacheable cold-start only; no physical execution.', checks=[r['id'] for r in campaign['commands'] if '.execute.' in r['id'] or r['id'].endswith(('.sections', '.undefined', '.execute'))]))
        criterion['author_validation'].append(dict(path=reports[2], scope='Actual canonical PR-T59-07 public export and explicit prerequisite composition.', checks=[r['id'] for r in export['commands']]))
    paths = set(campaign['input_sha256']) | set(export['input_sha256']) | set(reports)
    paths |= {p.relative_to(ROOT).as_posix() for p in HERE.iterdir() if p.is_file() and p.name != 'completion_dispositions.json'}
    paths |= {'tools/close_cacheable_boot_depth.py', 'tools/check_cacheable_boot_depth.py',
              'tools/register_cacheable_boot_depth.py', 'curriculum/projects/PR-T59-07.md',
              'research/v5/expansion/b06/author_domains.py', 'research/v5/expansion/b06/coverage.json',
              'research/v5/expansion/b06/depth_reaudit.json'}
    out = HERE / 'completion_dispositions.json'
    out.write_text(json.dumps(dict(schema_version=1, source_sha256={p:sha(ROOT / p) for p in sorted(paths)},
        current_reports=reports, updates=[row], scope='One exact original transfer ID; inherited historical claims retained at their original scope.'), indent=2) + '\n', encoding='utf8')
    print(json.dumps(dict(passed=True, updates=1, path=out.relative_to(ROOT).as_posix(), sha256=sha(out))))


if __name__ == '__main__':
    main()
