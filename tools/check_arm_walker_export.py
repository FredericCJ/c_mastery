"""Actual selected B06 CLI export and pure canonical attachment regeneration."""
import copy
import importlib.util
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

from common import ROOT, sha

sys.path.insert(0, str(ROOT))
from cmastery.project_assets import prepare

BASE = 'research/v5/expansion/arm_walker_sources'
REPORT = 'reviews/v5/expansion/arm_walker_sources/export_validation.json'
BRIEF = 'curriculum/projects/PR-T58-07.md'
AUTHOR = 'research/v5/expansion/b06/author_domains.py'
HISTORY = 'reviews/v5/expansion/arm_walker_sources/before_register_correction/manifest.json'


def load(path, name):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
    mod.ROOT = ROOT
    return mod


def main():
    rows = []
    def run(args, label, expected=0, marker=None):
        p = subprocess.run([str(v) for v in args], cwd=ROOT, capture_output=True,
                           text=True, timeout=120)
        row = dict(command=[str(v) for v in args], label=label,
                   returncode=p.returncode, stdout=p.stdout, stderr=p.stderr,
                   expected=expected, passed=p.returncode==expected and
                   (marker is None or marker in p.stdout+p.stderr))
        rows.append(row)
        assert row['passed'], row
        return row
    author = load(AUTHOR, 'arm_source_b06_author')
    sys.path.insert(0, str(ROOT / 'research/v5/expansion/b06'))
    captured = {}
    author.write = lambda path, text: captured.__setitem__(path, text)
    author.author(author.load('spec_t58'))
    actual = (ROOT / BRIEF).read_text(encoding='utf-8')
    assert captured[BRIEF] == actual, 'canonical producer reproduces selected project'
    match = re.match(r'```json\n(.*?)\n```\n\n(.*)', actual, re.S)
    meta, body = json.loads(match[1]), match[2]
    m2, b2 = author.attach_project_support('PR-T58-07', copy.deepcopy(meta), body)
    assert m2 == meta and b2 == body, 'attachment idempotent'
    history = json.loads((ROOT / HISTORY).read_text())
    prior_author = next(r for r in history['files'] if r['path']==AUTHOR)
    old = load(prior_author['preserved_path'], 'arm_source_old_b06_author')
    # The later, separately guarded publication successor is additive. Compare
    # its exact composition with the preserved hook rather than claiming the
    # old hook already supplied these UTF-8 renditions.
    import rebind_b06_documents as documents
    assert not documents.validate(documents.read(documents.PACKET),documents.read(documents.REPORT),documents.read(documents.MAP))
    renditions=load('research/v5/expansion/b06_document_utf8/author_support.py','arm_document_renditions')
    for pid in ['PR-T59-07','PR-T58-06']:
        seed = dict(shared_contracts=['curriculum/v5/contracts/b06.md'])
        prior_meta,prior_body=old.attach_project_support(pid,copy.deepcopy(seed),'base body')
        assert renditions.attach(pid,prior_meta,prior_body)==author.attach_project_support(pid,copy.deepcopy(seed),'base body'), 'exact retained hook plus guarded publication successor'
    prior_brief = next(r for r in history['files'] if r['path']==BRIEF)
    before = (ROOT / prior_brief['preserved_path']).read_text()
    old_match = re.match(r'```json\n(.*?)\n```\n\n(.*)', before, re.S)
    old_meta, old_body = json.loads(old_match[1]), old_match[2]
    assert body.startswith(old_body.rstrip()), 'original task/rubric body preserved'
    for k,v in old_meta.items():
        assert meta[k][:len(v)]==v if isinstance(v,list) else meta[k]==v, 'original front matter preserved'
    assets = prepare(ROOT, 'PR-T58-07')
    assert any(r['path']=='labs/architecture_ports_source/WALKER_MANUAL.md' for r in assets)
    with tempfile.TemporaryDirectory(prefix='cm-arm-source-export-') as tmp:
        dest = Path(tmp) / 'learner'
        run([sys.executable,'-m','cmastery','init','PR-T58-07',dest], 'actual learner init')
        for asset in assets:
            assert (dest / asset['path']).is_file() and sha(dest / asset['path'])==asset['sha256'], 'exported asset identity'
        assert not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*')), 'private assessor excluded'
        assert (dest / 'src/core.c').read_bytes()==(ROOT/'labs/architecture_ports/src/core.c').read_bytes(), 'original learner stub'
        run(['cmake','-S',dest,'-B',dest/'build-learner','-DCMAKE_C_COMPILER=gcc'], 'learner configure')
        run(['cmake','--build',dest/'build-learner','-j2'], 'learner build')
        run(['ctest','--test-dir',dest/'build-learner','-R','^architecture_ports.privilege$','--output-on-failure'], 'learner semantic refusal',8,'B06_CHECK')
        run(['cmake','-S',dest,'-B',dest/'build-assessor','-DCMAKE_C_COMPILER=gcc',
             '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')], 'separate assessor configure')
        run(['cmake','--build',dest/'build-assessor','-j2'], 'separate assessor build')
        run(['ctest','--test-dir',dest/'build-assessor','-R','^architecture_ports.privilege$','--output-on-failure'], 'separate assessor selected test',0,'100% tests passed')
    pins = {p:sha(ROOT/p) for p in [BRIEF,AUTHOR,HISTORY,BASE+'/attachment.json',
        'research/v5/expansion/b06/spec_t58.py',
        'tools/check_arm_walker_export.py',
        'cmastery/cli.py','cmastery/project_assets.py','cmastery/exercises.py',
        'cmastery/learning.py','cmastery/assessments.py']}
    for path in [ROOT/'labs/architecture_ports', ROOT/'assessor/architecture_ports']:
        for p in path.rglob('*'):
            if p.is_file() and '__pycache__' not in p.parts:
                pins[p.relative_to(ROOT).as_posix()] = sha(p)
    pins.update({a['path']:a['sha256'] for a in assets})
    pins.update({p:sha(ROOT/p) for p in [documents.PACKET,documents.REPORT,documents.MAP,
        'tools/rebind_b06_documents.py','research/v5/expansion/b06_document_utf8/author_support.py',
        'research/v5/expansion/b06_document_utf8/bindings.json']})
    out = dict(passed=True, commands=rows, generator_checked=True,
        cacheable_attachment_preserved=True, original_task_preserved=True,
        document_rendition_binding_validated=True,
        learner_failed_semantically=True, assessor_passed=True,
        selected_project='PR-T58-07', project_assets=assets, source_sha256=pins,
        limits=['Actual selected CLI export and selected original core test; guest execution has a separate source-bound report.',
                'No independent learner success, physical evidence or entire catalog/export claim.'])
    p = ROOT / REPORT; p.parent.mkdir(parents=True,exist_ok=True)
    p.write_text(json.dumps(out,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(passed=True, commands=len(rows), assets=len(assets), report=REPORT)))


if __name__ == '__main__':
    main()
