"""Public, source-bound validation with no private captures or historical binaries.

This profile executes new checks of shipped source. It does not re-certify any
historical author evidence or satisfy full release acceptance.
"""
from __future__ import annotations

import argparse
import ast
from datetime import datetime, timezone
import hashlib
import html
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

from common import ROOT, dump, load, parse, read_json
from native_tools import identify, resolve
from v2_common import source_identity

LIMITS = ['Public source and author implementation checks only; no release acceptance.',
          'Restricted source inspection, historical experiment reproduction, physical observations '
          'and independent learner assessment require their separate evidence profiles.',
          'A passing fixture oracle is not proof that a learner implementation works.']


def schema_checks():
    """Use real exercise validation and cross-reference canonical catalog contracts."""
    taxonomy, modules, projects, resources = load()
    topics = {t['id'] for d in taxonomy for t in d['subtopics']}
    objectives = [o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']]
    if len(objectives) != len(set(objectives)):
        raise ValueError('Duplicate objective identity')
    for key, project in projects.items():
        meta = project['meta']
        if meta['module'] not in set(modules) | {'INTEGRATION'} or set(meta['topics']) - topics:
            raise ValueError('Dangling project catalog reference: ' + key)
        if project['path'].stem != key:
            raise ValueError('Project filename/identity mismatch: ' + key)
        for name in meta['fixtures']:
            path = ROOT / name
            if not path.is_file() or path.resolve() != ROOT.resolve() / name or '..' in Path(name).parts:
                raise ValueError('Missing/redirected project fixture: ' + name)
    from cmastery import exercises
    catalog = exercises.build_catalog()
    from check_v5 import baseline, check_arithmetic, check_preserved
    from public_teaching_preservation import qualifies as public_teaching_proof
    from v2_common import check_migration
    preservation = check_preserved(baseline(), read_json(ROOT/'curriculum/v5/units.json'),
        read_json(ROOT/'curriculum/v5/artifacts.json'), read_json(ROOT/'curriculum/v5/recall.json'),
        projects, resources, taxonomy, publication_proof=public_teaching_proof)
    preservation += check_arithmetic(read_json(ROOT/'curriculum/edition.json'))
    preservation += check_migration(taxonomy)
    if preservation:
        raise ValueError('Historical catalog preservation: '+str(preservation))
    parsed = 0
    for folder in ('curriculum',):
        for path in (ROOT / folder).rglob('*.json'):
            read_json(path)
            parsed += 1
    for folder in ('tools', 'cmastery'):
        for path in (ROOT / folder).rglob('*.py'):
            if '__pycache__' not in path.parts:
                ast.parse(path.read_text(encoding='utf-8-sig'), filename=str(path))
    return dict(projects=len(projects), objectives=len(objectives),
                exercises=len(catalog['exercises']), json_documents=parsed,
                baseline_preservation='passed: original identities and scoped teaching-successor proofs',
                inspected_claim='Contract shapes, objective references, fixture presence and Python syntax; '
                                'source inspection statements are metadata, not re-inspected private evidence.')


def run_command(command, output, label, timeout=600, cwd=ROOT):
    started = datetime.now(timezone.utc).isoformat()
    try:
        result = subprocess.run(list(map(str, command)), cwd=cwd, capture_output=True,
                                text=True, timeout=timeout)
        code, stdout, stderr = result.returncode, result.stdout, result.stderr
        classification = 'success' if code == 0 else ('unexpected-crash' if code < 0 else 'failure')
    except subprocess.TimeoutExpired as error:
        code, stdout, stderr, classification = None, str(error.stdout or ''), str(error.stderr or ''), 'timeout'
    except OSError as error:
        code, stdout, stderr, classification = None, '', str(error), 'infrastructure-failure'
    log = output / (label + '.log')
    log.write_text(stdout + '\n' + stderr, encoding='utf8')
    return dict(label=label, command=list(map(str, command)), cwd=str(cwd), started=started,
                exit_code=code, passed=code == 0, classification=classification,
                log=str(log), log_sha256=hashlib.sha256(log.read_bytes()).hexdigest())


def foundation_checks(output):
    """Compile and execute shipped assessor code; reject the real learner stub."""
    rows = []
    names = ['prefix', 'size', 'cursor', 'owner', 'parse', 'sort', 'properties',
             'io', 'async', 'cleanup', 'packet']
    with tempfile.TemporaryDirectory(prefix='cmastery-public-labs-') as temporary:
        work = Path(temporary)
        for compiler in ('gcc', 'clang'):
            build = work / compiler
            commands = [
                [resolve('cmake'), '-S', ROOT/'labs/foundations', '-B', build, '-G', 'Ninja',
                 '-DCMAKE_C_COMPILER='+resolve(compiler), '-DCMAKE_BUILD_TYPE=Debug',
                 '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/foundations')],
                [resolve('cmake'), '--build', build, '--target', 'foundation-tests'],
                ['ctest', '--test-dir', build, '--output-on-failure', '--no-tests=error',
                 '--output-junit', build/'results.xml', '-R', '^foundations.('+'|'.join(names)+')$']]
            for stage, command in zip(('configure', 'build', 'test'), commands):
                row = run_command(command, output, compiler+'-'+stage)
                rows.append(row)
                if not row['passed']:
                    break
            else:
                cases = ET.parse(build/'results.xml').getroot().findall('.//testcase')
                expected = {'foundations.'+name for name in names}
                if {c.get('name') for c in cases} != expected or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):
                    rows[-1].update(passed=False, classification='missing-or-failed-test-evidence')
        learner = work/'learner'
        commands = [[resolve('gcc'), '-std=c17', '-Wall', '-Wextra', '-I'+str(ROOT/'labs/foundations/include'),
                     ROOT/'labs/foundations/src/foundation.c', ROOT/'labs/foundations/tests/tests.c', '-o', learner],
                    [learner, 'prefix']]
        build = run_command(commands[0], output, 'learner-build')
        rows.append(build)
        if build['passed']:
            rejection = run_command(commands[1], output, 'learner-rejection')
            text = Path(rejection['log']).read_text()
            # The foundation CHECK macro exits 1 and writes FAIL plus its assertion.
            rejection.update(passed=rejection['exit_code'] == 1 and 'FAIL ' in text,
                             classification='semantic-rejection' if rejection['exit_code'] == 1 and 'FAIL ' in text else rejection['classification'])
            rows.append(rejection)
    return rows


def algorithm_checks(output):
    """Execute touched algorithm implementations and semantic mutation checks."""
    rows = []
    with tempfile.TemporaryDirectory(prefix='cmastery-public-algorithms-') as temporary:
        for compiler in ('gcc', 'clang'):
            build = Path(temporary)/compiler
            commands = [
                [resolve('cmake'), '-S', ROOT/'labs/algorithms', '-B', build, '-G', 'Ninja',
                 '-DCMAKE_C_COMPILER='+resolve(compiler), '-DCMASTERY_PROFILE=semantic',
                 '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/algorithms')],
                [resolve('cmake'), '--build', build, '-j', '2'],
                ['ctest', '--test-dir', build, '--output-on-failure', '--no-tests=error',
                 '--output-junit', build/'results.xml', '-E', 'thumb']]
            for stage, command in zip(('configure', 'build', 'test'), commands):
                row = run_command(command, output, compiler+'-algorithms-'+stage)
                rows.append(row)
                if not row['passed']:
                    break
            else:
                cases = ET.parse(build/'results.xml').getroot().findall('.//testcase')
                names = {case.get('name') for case in cases}
                required = {'algorithms.v4_array', 'algorithms.v4_list', 'algorithms.v4_freelist',
                            'algorithms.v4_radix', 'algorithms.v4_dp', 'algorithms.v4_filter',
                            'algorithms.mutation_generation', 'algorithms.mutation_stability',
                            'algorithms.mutation_tombstone'}
                if not required <= names or any(c.find('failure') is not None or c.find('skipped') is not None for c in cases):
                    rows[-1].update(passed=False, classification='missing-or-failed-test-evidence')
                rows[-1]['executed_tests'] = len(cases)
    return rows


def build_handbook(output):
    """Render the active resolver, not historical overlay instructions, publicly."""
    from markdown_it import MarkdownIt
    from cmastery import route
    pages = {'index.md': route.render_guide()}
    data = route.load()
    for identifier in data['order']:
        entry = route.node(identifier)
        for name in entry.get('supplied_artifacts', []):
            path = route.public_path(ROOT, name)
            if not path.is_file() or path.is_symlink():
                raise ValueError('Required learner artifact missing or redirected: '+name)
        pages['effective/'+identifier+'.md'] = route.render_node(identifier)
    # Subject catalog remains a reference view, separately identified from order.
    for path in sorted((ROOT/'curriculum/modules').glob('*.md')):
        meta, body = parse(path)
        pages['catalog/'+meta['id']+'.md'] = '# '+meta['title']+'\n\n'+body
    renderer = MarkdownIt('commonmark', {'html': False}).enable('table')
    output.mkdir(parents=True, exist_ok=True)
    artifacts = {}
    unresolved = set()
    for name, body in pages.items():
        page = output/Path(name).with_suffix('.html')
        page.parent.mkdir(parents=True, exist_ok=True)
        content = '<!doctype html><html lang="en"><meta charset="utf-8"><title>C Mastery public handbook</title><body>'
        content += '<nav><a href="'+('../' if '/' in name else '')+'index.html">Active route</a></nav>'
        tokens = renderer.parse(body)
        for block in tokens:
            for token in block.children or []:
                if token.type != 'link_open':
                    continue
                link = token.attrGet('href') or ''
                if not link or link.startswith(('#', 'http:', 'https:', 'mailto:')):
                    continue
                path, _, fragment = link.partition('#')
                clean = path
                while clean.startswith('../'):
                    clean = clean[3:]
                if clean in pages:
                    target = output/Path(clean).with_suffix('.html')
                else:
                    # Effective cards contain excerpts from several canonical
                    # directories. Preserve local public source downloads.
                    candidates = [ROOT/clean]
                    candidates += [ROOT/base/clean for base in ('curriculum', 'curriculum/projects',
                                  'curriculum/modules', 'curriculum/guides', 'curriculum/v5',
                                  'curriculum/v4', 'curriculum/v5/exercises')]
                    source = next((p for p in candidates if p.is_file() and p.resolve().is_relative_to(ROOT.resolve())), None)
                    if source is not None:
                        source = source.resolve()
                    if source is None or {'assessor', 'private', 'reviews', 'tools', 'planning'} & set(source.relative_to(ROOT).parts):
                        unresolved.add(link)
                        # Historical source references remain visible as text;
                        # never expose a broken/private clickable destination.
                        token.attrSet('href', '#unavailable-source-reference')
                        token.attrSet('title', 'Archival reference: '+link)
                        continue
                    target = output/'source'/source.relative_to(ROOT)
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copyfile(source, target)
                    artifacts[target.relative_to(output).as_posix()] = hashlib.sha256(target.read_bytes()).hexdigest()
                token.attrSet('href', os.path.relpath(target, page.parent).replace(os.sep, '/')+('#'+fragment if fragment else ''))
        content += renderer.renderer.render(tokens, renderer.options, {})
        content += '<aside id="unavailable-source-reference">Historical references retain their canonical source locations; restricted author evidence is outside this public view.</aside></body></html>\n'
        page.write_text(content, encoding='utf8')
        artifacts[page.relative_to(output).as_posix()] = hashlib.sha256(page.read_bytes()).hexdigest()
    dump(output/'manifest.json', dict(schema_version=1, profile='public', pages=artifacts,
                                     archival_references=sorted(unresolved), limits=LIMITS,
                                     renderer='markdown-it-py; active route resolver'))
    return artifacts


def run(output=None, *, build=False):
    destination = Path(output or ROOT/'build/public').absolute()
    destination.mkdir(parents=True, exist_ok=True)
    raw = destination/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    raw.mkdir(parents=True, exist_ok=False)
    identity = source_identity()
    report = dict(schema_version=1, profile='public', passed=False, accepted=False,
                  run_directory=str(raw),
                  source_identity=identity, checks=[], limits=LIMITS,
                  hardware_measurement='not-assessed', independent_learner='not-assessed')
    try:
        report['tools'] = [identify(x) for x in ('gcc', 'clang', 'cmake', 'ninja')]
        report['structure'] = schema_checks()
        scripts = ['check_public_profiles.py', 'check_native_route.py', 'check_learning_v4.py',
                   'check_exercise_workflow.py']
        for script in scripts:
            report['checks'].append(run_command([sys.executable, ROOT/'tools'/script], raw, script[:-3]))
        from check_fixtures import selected_oracles
        from run_fixture_oracle import DRIVER_FAMILIES
        for oracle in selected_oracles():
            family = oracle.stem.removeprefix('verify_').removesuffix('_fixtures')
            command = [sys.executable, oracle]
            if family in DRIVER_FAMILIES:
                command = [sys.executable, ROOT/'tools/run_fixture_oracle.py', family]
            if oracle.name == 'verify_http_fixtures.py':
                command += ['--report', raw/'http_fixture_checks.json']
            report['checks'].append(run_command(command, raw, oracle.stem))
        report['checks'].append(run_command([sys.executable, ROOT/'tools/check_native_build_profiles.py',
                                             '--configure-only', '--report', raw/'build_profiles.json'],
                                            raw, 'build-profile-policy'))
        report['checks'].append(run_command([sys.executable, ROOT/'tools/run_native_pmu.py',
                                             '--protocol-only', '--report', raw/'pmu_protocol.json'],
                                            raw, 'pmu-protocol'))
        report['checks'].append(run_command([sys.executable, ROOT/'tools/run_deadline_pilot.py',
                                             '--no-measure', '--report', raw/'deadline_protocol.json'],
                                            raw, 'deadline-protocol'))
        report['checks'].extend(foundation_checks(raw))
        report['checks'].extend(algorithm_checks(raw))
        if build:
            report['html'] = build_handbook(destination/'html')
            report['handbook'] = read_json(destination/'html/manifest.json')
        report['final_source_sha256'] = source_identity()['sha256']
        report['source_stable'] = identity['sha256'] == report['final_source_sha256']
        report['passed'] = report['source_stable'] and all(row['passed'] for row in report['checks'])
    except Exception as error:
        report['error'] = type(error).__name__+': '+str(error)
    dump(raw/'validation.json', report)
    dump(destination/'validation.json', report)
    print(json.dumps(dict(passed=report['passed'], profile='public', report=str(destination/'validation.json'),
                          error=report.get('error'), failed=[r['label'] for r in report['checks'] if not r['passed']])))
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--build', action='store_true')
    args = parser.parse_args()
    return 0 if run(args.output, build=args.build)['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
