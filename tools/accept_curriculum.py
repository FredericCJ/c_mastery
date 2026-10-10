"""Record curriculum acceptance only after content and publication evidence pass.

Archive extraction/rebuild is recorded separately by package.py to avoid a
self-referential archive hash. This receipt accepts teaching/publication scope,
never learner implementations or physical firmware results.
"""
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha, write, load_taxonomy, require_research_gates


def main():
    if (ROOT/'curriculum/edition.json').exists():
        from accept_v2 import main as v2_main
        return v2_main()
    require_research_gates()
    checks = ['canonical_validation', 'fixture_checks', 'contract_guard_checks',
              'sphinx_build', 'html_checks', 'markdown_checks', 'browser_review', 'baseline_preservation']
    receipts = {}
    for name in checks:
        report = read_json(ROOT / f'reviews/{name}.json')
        if not report.get('passed'):
            raise ValueError('Required check did not pass: ' + name)
        receipts[name] = sha(ROOT / f'reviews/{name}.json')
    review_names = ['review_language_and_assessment', 'review_systems_and_guides',
                    'review_embedded_and_integrations']
    if (ROOT/'curriculum/enhancement_taxonomy.json').exists():
        review_names += ['review_enhancement_algorithms_architecture',
                         'review_enhancement_communications', 'review_enhancement_stacks']
    if (ROOT/'curriculum/performance_debugging_taxonomy.json').exists():
        review_names += ['review_performance','review_debugging','review_memory']
        checks_extra=['performance_debugging_preservation']
        for name in checks_extra:
            report=read_json(ROOT/f'reviews/{name}.json')
            if not report.get('passed'): raise ValueError('Required check did not pass: '+name)
            receipts[name]=sha(ROOT/f'reviews/{name}.json')
    reviewed_domains=set(); fresh_reviewed_paths=set()
    for name in review_names:
        report = read_json(ROOT / f'reviews/{name}.json')
        if not report.get('accepted'):
            raise ValueError('Independent review not accepted: ' + name)
        scope=report['scope']
        reviewed_domains.update(scope.get('modules',scope.get('domains',[])))
        for finding in report.get('findings', []):
            blocking=finding.get('blocking',finding.get('severity') in ['medium','high','critical'])
            if blocking and finding.get('status') not in ['resolved', 'closed']:
                raise ValueError('Open blocking finding in ' + name)
        if name in ['review_performance','review_debugging','review_memory']:
            pins=report.get('reviewed_sha256',{})
            if not pins: raise ValueError('Fresh independent review must pin inspected files: '+name)
            for path,digest in pins.items():
                resolved=(ROOT/path).resolve()
                if not resolved.is_relative_to(ROOT) or not resolved.is_file() or sha(resolved)!=digest:
                    raise ValueError('Independent review became stale: '+name+' '+path)
                fresh_reviewed_paths.add(path)
        receipts[name] = sha(ROOT / f'reviews/{name}.json')
    if reviewed_domains!={d['id'] for d in load_taxonomy()}:
        raise ValueError('Independent review must cover every original and enhancement domain')
    if (ROOT/'curriculum/performance_debugging_taxonomy.json').exists():
        previous=read_json(ROOT/'reviews/baseline_2026_10_09_architecture_networking/final_acceptance.json')
        old={r['path']:r['sha256'] for r in previous['canonical_source_snapshot']}
        required=[]
        for folder in ['curriculum/modules','curriculum/projects','curriculum/guides','fixtures']:
            for path in (ROOT/folder).rglob('*'):
                if path.is_file():
                    relative=path.relative_to(ROOT).as_posix()
                    if old.get(relative)!=sha(path): required.append(relative)
        required+=['curriculum/performance_debugging_taxonomy.json','references/resources.json']
        missing=set(required)-fresh_reviewed_paths
        if missing: raise ValueError('Changed teaching/fixtures lack a fresh independent review: '+str(sorted(missing)))
    manifest = read_json(ROOT / 'reviews/input_manifest.json')
    if any(sha(ROOT / name) != digest for name, digest in manifest.items()):
        raise ValueError('Original input changed')
    files = []
    for name in ['curriculum', 'fixtures', 'references', 'research', 'site', 'tools']:
        files += [p for p in (ROOT/name).rglob('*') if p.is_file()
                  and not {'private', '__pycache__'} & set(p.relative_to(ROOT).parts)]
    source_hashes = [{'path': p.relative_to(ROOT).as_posix(), 'sha256': sha(p)}
                     for p in sorted(files)]
    report = {'accepted': True, 'status': 'accepted',
              'date_utc': datetime.now(timezone.utc).isoformat(), 'reviewer': '/root',
              'scope': 'Complete project-driven curriculum and local publication, before archive packaging',
              'counts': read_json(ROOT/'reviews/canonical_validation.json'),
              'evidence_receipt_sha256': receipts, 'original_inputs': manifest,
              'canonical_source_snapshot': source_hashes,
              'limitations': ['No learner C implementations or completed learner mastery gates are claimed.',
                              'Physical STM32 evidence and formal-tool outcomes remain learner activities.',
                              'Compiler probes cover the recorded GCC/environment features only.',
                              'Archive hashes and fresh extraction/rebuild are in package_report.json.']}
    dump(ROOT/'reviews/final_acceptance.json', report)
    counts=report['counts']
    write(ROOT/'reviews/final_acceptance.md',
          f"# Curriculum acceptance\n\nAccepted: {counts['modules']} domains, {counts['subtopics']} subtopics and {counts['objectives']:,} objectives; "
          f"{counts['projects']} bounded project milestones. Original objectives are preserved; independent content reviews closed their findings. "
          'Canonical contracts, all fixture groups, strict Sphinx, rendered links/prose/objectives, '
          'standalone Markdown and actual browser workflows passed.\n\n'
          'The JSON receipt records exact source and evidence hashes. Archive reproducibility is '
          'recorded separately in `package_report.json`. Acceptance covers the teaching deliverable; '
          'learner C, formal-tool and physical-board evidence remains to be produced.\n')
    print('Curriculum accepted; archive packaging/rebuild is a separate final check.')


if __name__ == '__main__':
    main()
