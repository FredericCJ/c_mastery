"""Validate, stage and reproduce fifth-edition exports before replacing final bundles.

Beyond the v4 comparisons (published HTML and the standalone Markdown), every
tracked generated file must be a fixed point of the extracted-source rebuild.
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
               'references/registry_v2.json'] + sorted(p.relative_to(ROOT).as_posix() for p in V5.glob('*.json'))


def main():
    package_v4.main(receipts=RECEIPTS, schema_version=5, accept=accept_v5.main, archives=ARCHIVES,
                    candidate_prefix='v5_', build_script='tools/build_v5.py', fixed_point=FIXED_POINT)


if __name__ == '__main__':
    main()
