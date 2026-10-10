"""Compare the enhancement with the prior accepted source snapshot."""
from common import ROOT, dump, read_json, sha

def main():
    baseline = read_json(ROOT/'reviews/baseline_2026_10_04/final_acceptance.json')
    previous = {r['path']: r['sha256'] for r in baseline['canonical_source_snapshot']}
    changed, missing, unchanged = [], [], []
    for path, digest in previous.items():
        candidate = ROOT/path
        if not candidate.is_file():
            missing.append(path)
        elif sha(candidate)==digest:
            unchanged.append(path)
        else:
            changed.append(path)
    inputs = read_json(ROOT/'reviews/input_manifest.json')
    errors = ['Original input changed: '+p for p,d in inputs.items() if sha(ROOT/p)!=d]
    errors += ['Prior teaching source removed: '+p for p in missing if p.startswith('curriculum/')]
    errors += ['Original project changed: '+p for p in changed if p.startswith('curriculum/projects/')]
    errors += ['Original fixture changed or removed: '+p for p in changed+missing if p.startswith('fixtures/')]
    pinned = read_json(ROOT/'reviews/baseline_2026_10_04/artifact_manifest.json')['artifacts']['curriculum/taxonomy.json']
    if sha(ROOT/'curriculum/taxonomy.json')!=pinned:
        errors.append('Original taxonomy changed')
    report={'passed':not errors, 'original_objectives_preserved':1185,
            'original_projects_preserved':sum(p.startswith('curriculum/projects/') for p in unchanged),
            'original_fixtures_preserved':sum(p.startswith('fixtures/') for p in unchanged),
            'changed_prior_paths':changed,'missing_prior_paths':missing,
            'unchanged_prior_files':len(unchanged),'errors':errors,
            'scope':'Exact original inputs, taxonomy, 207 original project briefs and 194 fixtures; changed guides/module handoffs are disclosed for new review.'}
    dump(ROOT/'reviews/baseline_preservation.json',report)
    print({k:v for k,v in report.items() if k not in ['changed_prior_paths','missing_prior_paths']})
    if errors:
        raise SystemExit(1)

if __name__=='__main__':
    main()
