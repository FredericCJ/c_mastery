"""Create distributable artifacts and independently rebuild an extracted source copy."""
from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import sys
import zipfile
from common import ROOT, dump, sha, read_json

def write_zip(path, files, base):
    entries=[]
    with zipfile.ZipFile(path,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
        for file in sorted(files):
            relative=file.relative_to(base).as_posix()
            if 'private' in Path(relative).parts or '.venv' in Path(relative).parts:
                raise ValueError('Private path in export '+relative)
            z.write(file,relative)
            entries.append({'path':relative,'bytes':file.stat().st_size,'sha256':sha(file)})
        z.writestr('artifact_manifest.json',json.dumps({'files':entries},indent=2)+'\n')
    return entries

def main():
    if (ROOT/'curriculum/v3_taxonomy.json').exists():
        from package_v3 import main as v3_main
        return v3_main()
    if (ROOT/'curriculum/edition.json').exists():
        from package_v2 import main as v2_main
        return v2_main()
    acceptance=read_json(ROOT/'reviews/final_acceptance.json')
    if acceptance.get('accepted') is not True: raise ValueError('Accept the current teaching/publication before packaging')
    canonical_roots=['curriculum','fixtures','references','research','site','tools']
    current={p.relative_to(ROOT).as_posix() for name in canonical_roots for p in (ROOT/name).rglob('*')
             if p.is_file() and not {'private','__pycache__'} & set(p.relative_to(ROOT).parts)}
    if current!={r['path'] for r in acceptance['canonical_source_snapshot']}:
        raise ValueError('Accepted canonical source file set changed before packaging')
    for record in acceptance['canonical_source_snapshot']:
        if sha(ROOT/record['path'])!=record['sha256']:
            raise ValueError('Accepted source changed before packaging: '+record['path'])
    dist=ROOT/'dist'; dist.mkdir(exist_ok=True)
    source_files=[]
    roots=['curriculum','fixtures','references','research','tools','site','planning','reviews']
    for name in roots:
        for file in (ROOT/name).rglob('*'):
            relative=file.relative_to(ROOT)
            if file.is_file() and not {'private','__pycache__'}&set(relative.parts) and file.suffix not in ['.pyc']:
                if file.name not in ['package_report.json','package_build.log']: source_files.append(file)
    for name in ['README.md','build.ps1','serve.ps1','requirements.in','requirements-lock.txt','.gitignore',
        'c_programming_mastery_curriculum.md','c_programming_mastery_curriculum_prompt.md',
        'c_programming_mastery_curriculum_prompt_expanded.md','c_programming_mastery_fine_grained_taxonomy.md']:
        source_files.append(ROOT/name)
    source=dist/'c_mastery_sources.zip'; site=dist/'c_mastery_html.zip'
    source_entries=write_zip(source,source_files,ROOT)
    html=ROOT/'build/html'; html_files=[p for p in html.rglob('*') if p.is_file() and '.doctrees' not in p.parts and p.name not in ['.buildinfo','.buildinfo.bak']]
    html_entries=write_zip(site,html_files,html)
    check=ROOT/'build/reproducibility'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    check.mkdir(parents=True,exist_ok=False)
    if not check.resolve().is_relative_to((ROOT/'build/reproducibility').resolve()): raise ValueError('Unsafe extraction root')
    with zipfile.ZipFile(source) as z:
        for name in z.namelist():
            if not (check/name).resolve().is_relative_to(check.resolve()): raise ValueError('Unsafe archive member')
        z.extractall(check)
    records=[]
    for tool in ['write_program.py','validate.py','check_fixtures.py','check_contract_guards.py',
                 'check_baseline_preservation.py','check_performance_debugging_preservation.py',
                 'publish.py','check_html.py','check_markdown.py']:
        run=subprocess.run([sys.executable,'-X','utf8',str(check/'tools'/tool)],cwd=check,capture_output=True,text=True,encoding='utf-8')
        records.append({'tool':tool,'exit_code':run.returncode})
        with (ROOT/'reviews/package_build.log').open('a',encoding='utf-8') as log:
            log.write(tool+'\n'+run.stdout+'\n'+run.stderr+'\n')
        if run.returncode: raise ValueError('Extracted source rebuild failed: '+tool)
    rebuilt=check/'build/html'
    rebuilt_files=[p for p in rebuilt.rglob('*') if p.is_file() and '.doctrees' not in p.parts and p.name not in ['.buildinfo','.buildinfo.bak']]
    if {p.relative_to(html).as_posix() for p in html_files}!={p.relative_to(rebuilt).as_posix() for p in rebuilt_files}:
        raise ValueError('Rebuilt public file set differs')
    differences=[]; compared=0
    for original in html_files:
        relative=original.relative_to(html)
        candidate=rebuilt/relative
        if not candidate.is_file() or sha(original)!=sha(candidate): differences.append(relative.as_posix())
        compared+=1
    if differences: raise ValueError('Rebuilt public HTML differs: '+str(differences[:20]))
    markdown='c_programming_mastery_curriculum.md'
    if sha(ROOT/markdown)!=sha(check/markdown):
        raise ValueError('Rebuilt standalone Markdown differs')
    report={'passed':True,'sources':{'path':source.name,'sha256':sha(source),'files':len(source_entries)},
      'html':{'path':site.name,'sha256':sha(site),'files':len(html_entries)},'rebuild':records,
      'reproduced_public_files':compared,'exact_public_file_set':True,'extraction_root':check.relative_to(ROOT).as_posix(),
      'reproduced_standalone_markdown':{'path':markdown,'sha256':sha(check/markdown)},
      'excluded_build_metadata':['.buildinfo','.buildinfo.bak','.doctrees'],
      'scope':'Fresh source extraction and build; existing isolated publishing dependencies reused. No Knowledge/SWE access, private captures or learner C solutions required.'}
    dump(ROOT/'reviews/package_report.json',report)
    print(json.dumps(report,indent=2))

if __name__=='__main__': main()
