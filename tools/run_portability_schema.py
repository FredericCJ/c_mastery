"""Schema campaign retaining every generated compiler input and actual build recipe."""
from datetime import datetime,timezone
from pathlib import Path
import hashlib,json,shutil
import run_portability_scoped as campaign
from common import ROOT,sha
from run_resilience_scoped import dependency_paths as original_dependencies
def run():
 retained=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('schema-generated-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));retained.mkdir(parents=True)
 def dependencies(root,build,commands,ninja):
  build=Path(build).resolve();names={'config.h','rules.h','fields.h','codec.h','manifest.json'};found=set();lines=[]
  for line in ninja.splitlines():
   if line.startswith('    ') and line.strip():
    path=Path(line.strip());path=path if path.is_absolute() else build/path;path=path.resolve()
    if path.is_relative_to(build/'generated'):
     assert path.parent==build/'generated' and path.name in names;found.add(path.name);continue
   lines.append(line)
  repo,external,generated=original_dependencies(root,build,commands,'\n'.join(lines));assert found
  provider=('labs' if build.name=='learner' else 'assessor')+'/language_portability/schema/generate.py';dest=retained/build.name;dest.mkdir()
  for name in names:
   path=build/'generated'/name;saved=dest/name;shutil.copyfile(path,saved)
   generated[str(path)]={'sha256':sha(path),'retained_path':saved.relative_to(ROOT).as_posix(),'producer_source':provider,'input_source':'labs/language_portability/schema/schema.json','recipe_contract':'labs/language_portability/schema/CMakeLists.txt'}
  if build.name!='learner':
   manifest=json.loads((dest/'manifest.json').read_text());assert set(manifest['outputs'])==names-{'manifest.json'}
   for name,h in manifest['outputs'].items():assert sha(dest/name)==h
  saved=dest/'build.ninja';shutil.copyfile(build/'build.ninja',saved);generated['build_recipe']={'sha256':sha(saved),'retained_path':saved.relative_to(ROOT).as_posix()}
  return repo,external,generated
 campaign.dependency_paths=dependencies;return campaign.run('schema')
if __name__=='__main__':raise SystemExit(0 if run() else 1)
