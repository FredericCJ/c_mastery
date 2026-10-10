"""Export current source, unsolved learner, assessor and HTML bundles; rebuild source."""
from datetime import datetime, timezone
from pathlib import Path
import subprocess
import sys
import zipfile
from common import ROOT, read_json, sha, dump
from v2_common import source_identity
from package import write_zip

def main():
    acceptance=read_json(ROOT/'reviews/v2/acceptance.json')
    identity=source_identity()
    if not acceptance.get('accepted') or acceptance['input_sha256']!=identity['sha256']:
        raise ValueError('Current accepted source identity is required')
    dist=ROOT/'dist';dist.mkdir(exist_ok=True)
    files=[ROOT/p for p in identity['files']]
    for folder in ['planning','reviews']:
        files += [p for p in (ROOT/folder).rglob('*') if p.is_file() and not {'private','__pycache__'} & set(p.parts)
                  and not p.name.startswith('package')]
    for name in ['serve.ps1','c_programming_mastery_curriculum.md','c_programming_mastery_curriculum_prompt.md',
                 'c_programming_mastery_curriculum_prompt_expanded.md','c_programming_mastery_fine_grained_taxonomy.md']:
        if (ROOT/name).exists(): files.append(ROOT/name)
    files=sorted(set(files))
    source=dist/'c_mastery_v2_sources.zip'
    source_entries=write_zip(source,files,ROOT)
    learner_files=[p for p in files if 'assessor' not in p.relative_to(ROOT).parts and p.relative_to(ROOT).parts[0] not in {'reviews','planning'}]
    learner=dist/'c_mastery_v2_learner.zip'
    learner_entries=write_zip(learner,learner_files,ROOT)
    assessor=dist/'c_mastery_v2_assessor.zip'
    assessor_entries=write_zip(assessor,[p for p in files if p.relative_to(ROOT).parts[0]=='assessor'],ROOT)
    with zipfile.ZipFile(learner) as archive:
        if any('assessor' in Path(name).parts for name in archive.namelist()): raise ValueError('Assessor implementation leaked to learner bundle')
    html=ROOT/'build/html'
    html_files=[p for p in html.rglob('*') if p.is_file() and '.doctrees' not in p.parts and p.name not in {'.buildinfo','.buildinfo.bak'}]
    html_zip=dist/'c_mastery_v2_html.zip';html_entries=write_zip(html_zip,html_files,html)
    check=ROOT/'build/reproducibility'/datetime.now(timezone.utc).strftime('v2_%Y%m%dT%H%M%S%fZ')
    check.mkdir(parents=True,exist_ok=False)
    if not check.resolve().is_relative_to((ROOT/'build/reproducibility').resolve()): raise ValueError('Unsafe extraction root')
    with zipfile.ZipFile(source) as archive:
        for name in archive.namelist():
            if not (check/name).resolve().is_relative_to(check.resolve()): raise ValueError('Unsafe archive member')
        archive.extractall(check)
    run=subprocess.run([sys.executable,'-X','utf8',str(check/'tools/build_v2.py')],cwd=check,capture_output=True,text=True,encoding='utf-8')
    (ROOT/'reviews/v2/package_build.log').write_text(run.stdout+'\n'+run.stderr,encoding='utf-8')
    if run.returncode: raise ValueError('Extracted-source build failed; see package_build.log')
    rebuilt=check/'build/html'
    rebuilt_files=[p for p in rebuilt.rglob('*') if p.is_file() and '.doctrees' not in p.parts and p.name not in {'.buildinfo','.buildinfo.bak'}]
    original_map={p.relative_to(html).as_posix():sha(p) for p in html_files}
    rebuilt_map={p.relative_to(rebuilt).as_posix():sha(p) for p in rebuilt_files}
    if original_map!=rebuilt_map: raise ValueError('Extracted build differs from published HTML')
    markdown='c_programming_mastery_curriculum.md'
    if sha(ROOT/markdown)!=sha(check/markdown): raise ValueError('Extracted standalone Markdown differs')
    report={'schema_version':2,'status':'passed','passed':True,'input_sha256':identity['sha256'],
        'archives':[{'path':str(p.relative_to(ROOT)),'sha256':sha(p),'files':len(entries)} for p,entries in
                    [(source,source_entries),(learner,learner_entries),(assessor,assessor_entries),(html_zip,html_entries)]],
        'learner_excludes_assessor':True,'fresh_extracted_source_rebuild':True,'reproduced_html_files':len(original_map),
        'standalone_sha256':sha(check/markdown),'extraction':str(check.relative_to(ROOT)),
        'scope':'Fresh source rebuild with existing locked publishing environment. Hardware and learner mastery are not asserted.'}
    dump(ROOT/'reviews/v2/package_report.json',report)
    print('Four v2 bundles exported; fresh HTML/Markdown reproduction passed.')

if __name__=='__main__': main()
