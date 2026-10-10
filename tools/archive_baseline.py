"""Preserve the accepted October 4 receipts before the enhancement revision."""
from common import ROOT, dump, sha
import shutil

target = ROOT / 'reviews/baseline_2026_10_04'
if not target.exists():
    target.mkdir(parents=True)
    names = ['final_acceptance.json', 'final_acceptance.md', 'package_report.json',
             'research_gate.json', 'browser_review.json', 'canonical_validation.json',
             'review_language_and_assessment.json', 'review_systems_and_guides.json',
             'review_embedded_and_integrations.json']
    for name in names:
        shutil.copy2(ROOT / 'reviews' / name, target / name)
    dump(target / 'artifact_manifest.json', {'edition': '2026-10-04',
         'artifacts': {name: sha(ROOT / name) for name in
         ['curriculum/taxonomy.json', 'c_programming_mastery_curriculum.md',
          'dist/c_mastery_sources.zip', 'dist/c_mastery_html.zip']},
         'scope': 'Prior accepted edition; these receipts do not certify the enhancement.'})
print('Prior acceptance receipts preserved:', target)
