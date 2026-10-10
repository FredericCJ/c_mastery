"""Preserve the entire accepted 42-domain edition while adding T43–T45."""
import hashlib
from common import ROOT,read_json,sha,dump,load_taxonomy

def main():
    baseline=ROOT/'reviews/baseline_2026_10_09_architecture_networking'
    prior=read_json(baseline/'final_acceptance.json')
    previous={r['path']:r['sha256'] for r in prior['canonical_source_snapshot']}
    errors=[]; unchanged=[]; changed=[]; missing=[]
    for path,digest in previous.items():
        candidate=ROOT/path
        if not candidate.is_file(): missing.append(path)
        elif sha(candidate)==digest: unchanged.append(path)
        else: changed.append(path)
    for path in changed+missing:
        if path.startswith(('curriculum/projects/','fixtures/')):
            errors.append('Accepted project/fixture changed or missing: '+path)
        if path.startswith('curriculum/modules/') and path in changed:
            # A handoff may be appended at EOF; all previously accepted bytes remain exact.
            content=(ROOT/path).read_bytes()
            marker=b'<!-- performance-debugging handoff -->'
            prefix=content.split(marker,1)[0]
            if marker not in content or hashlib.sha256(prefix).hexdigest()!=previous[path]:
                errors.append('Prior module teaching changed rather than an appended handoff: '+path)
    for path in missing:
        if path.startswith('curriculum/'): errors.append('Prior teaching file removed: '+path)
    manifest=read_json(baseline/'artifact_manifest.json')['artifacts']
    for name in ['curriculum/taxonomy.json','curriculum/enhancement_taxonomy.json']:
        if sha(ROOT/name)!=manifest[name]: errors.append('Accepted taxonomy changed: '+name)
    taxonomy=load_taxonomy()
    prior_domains=read_json(ROOT/'curriculum/taxonomy.json')['domains']+read_json(ROOT/'curriculum/enhancement_taxonomy.json')['domains']
    if [d for d in taxonomy if d['id'] in {p['id'] for p in prior_domains}]!=prior_domains:
        errors.append('Accepted 1383 objective definitions changed in combined taxonomy')
    report={'passed':not errors,'prior_domains_preserved':42,'prior_objectives_preserved':1383,
        'prior_projects_preserved':sum(p.startswith('curriculum/projects/') for p in unchanged),
        'prior_fixtures_preserved':sum(p.startswith('fixtures/') for p in unchanged),
        'changed_prior_paths':changed,'missing_prior_paths':missing,'errors':errors,
        'scope':'Exact accepted taxonomies,265 prior briefs,254 fixture files and all prior module bytes; appended handoffs and changed common guides require fresh independent review.'}
    dump(ROOT/'reviews/performance_debugging_preservation.json',report)
    print({k:v for k,v in report.items() if k not in ['changed_prior_paths','missing_prior_paths']})
    if errors: raise SystemExit(1)

if __name__=='__main__': main()
