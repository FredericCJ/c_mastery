"""Validate, stage and reproduce fifth-edition exports before replacing final bundles.

Beyond the v4 comparisons (published HTML and the standalone Markdown), every
tracked generated file must be a fixed point of the extracted-source rebuild.

For explicitly provisioned local author inputs, run accept_v5.py first, then
candidate_author_environment.py --manifest REVIEWS_PATH, then this producer
with --author-environment-manifest REVIEWS_PATH. Acceptance is checked again;
any changed selected file set requires a new manifest, never automatic admission.
"""
import accept_v5
import package_v4
from package_v4 import bounded_directory, promote  # noqa: F401  (re-exported for the release guards)
from common import ROOT
from v5_common import RECEIPTS, V5, manifest

ARCHIVES = tuple(manifest()['archive_prefix'] + '_' + kind + '.zip' for kind in ('sources', 'learner', 'assessor', 'html'))
FIXED_POINT = ['curriculum/coverage.json', 'curriculum/program.json', 'curriculum/project_graph.json',
               'curriculum/guides/workload.md', 'curriculum/guides/roadmap.md', 'curriculum/guides/project-route.md',
               'curriculum/guides/executable-evidence.md', 'curriculum/guides/communication-stacks.md',
               'curriculum/guides/performance-debugging.md', 'references/resources.json',
               'references/registry_v2.json'] + sorted(p.relative_to(ROOT).as_posix() for p in V5.glob('*.json')) + [
               'curriculum/v5/expansion/' + name for name in
               ('coverage_depth.json', 'modern_discovery.json', 'domain_progress.json', 'audit.md',
                'legacy_assessment_catalog.json', 'completion_ledger.json', 'gaps.json', 'remaining_work.json')]


def learner_file(relative):
    """The source bundle owns v5 authoring recipes and their embedded answers.

    Learners keep the inspected source registers used by exercise contracts.
    Audit narratives, author scripts, cards and validation records remain in the
    full source bundle; rendered learner tasks and rubrics live in curriculum/.
    """
    parts = relative.parts
    if parts[:2] == ('research', 'v5'):
        return relative.name.endswith('resources.json')
    return True


def main(author_environment_manifest=None):
    package_v4.main(receipts=RECEIPTS, schema_version=5, accept=accept_v5.main, archives=ARCHIVES,
                    candidate_prefix='v5_', build_script='tools/build_v5.py', fixed_point=FIXED_POINT,
                    learner_filter=learner_file,author_environment_manifest=author_environment_manifest)


if __name__ == '__main__':
    import argparse
    from pathlib import Path
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--author-environment-manifest',type=Path,help='Explicit exact active-pin local author inputs; never distributed in archives and never an acceptance override')
    main(parser.parse_args().author_environment_manifest)
