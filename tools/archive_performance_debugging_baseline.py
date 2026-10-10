"""Freeze the accepted 42-domain edition before performance/debugging extension."""
from datetime import datetime, timezone
import shutil
from common import ROOT, dump, read_json, sha


def main():
    target=ROOT/'reviews/baseline_2026_10_09_architecture_networking'
    if target.exists():
        print('Accepted 42-domain baseline already archived:',target)
        return
    receipt=read_json(ROOT/'reviews/final_acceptance.json')
    package=read_json(ROOT/'reviews/package_report.json')
    if not receipt.get('accepted') or receipt['counts']['modules']!=42 or not package.get('passed'):
        raise ValueError('Expected accepted and rebuilt 42-domain baseline')
    target.mkdir(parents=True,exist_ok=False)
    names=['final_acceptance.json','final_acceptance.md','package_report.json',
           'enhancement_research_gate.json','browser_review.json','browser_review.md',
           'canonical_validation.json','baseline_preservation.json',
           'review_enhancement_algorithms_architecture.json','review_enhancement_communications.json',
           'review_enhancement_stacks.json']
    for name in names: shutil.copy2(ROOT/'reviews'/name,target/name)
    dump(target/'artifact_manifest.json',{'edition':'2026-10-09 architecture/networking',
         'date_utc':datetime.now(timezone.utc).isoformat(),
         'artifacts':{name:sha(ROOT/name) for name in ['curriculum/taxonomy.json',
            'curriculum/enhancement_taxonomy.json','c_programming_mastery_curriculum.md',
            'dist/c_mastery_sources.zip','dist/c_mastery_html.zip']},
         'scope':'Prior accepted 42-domain edition; its receipts do not accept T43–T45.'})
    print('Accepted 42-domain baseline archived:',target)


if __name__=='__main__': main()
