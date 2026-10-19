"""Add warning-clean incomplete copies; preserve the original learner files."""
import hashlib,json,re,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];source=ROOT/'labs/execution/src/cmx.c';folder=ROOT/'labs/mcu_followthrough/baseline';folder.mkdir(parents=True,exist_ok=True)
def rewrite(m):
 names=[]
 for parameter in m.group(2).split(','):
  name=re.search(r'([A-Za-z_]\w*)\s*$',parameter.strip())
  if name and name.group(1)!='void':names.append(name.group(1))
 return m.group(1)+' {\n'+''.join('  (void)'+name+';\n' for name in names)
text=source.read_text();pattern=r'((?:enum cmx_result|void|int)\s+\w+\(([^)]*)\))\s*\{'
updated=re.sub(pattern,rewrite,text);(folder/'cmx.c').write_text('/* Additive warning-clean incomplete baseline. Original source unchanged. */\n'+updated)
shutil.copyfile(ROOT/'labs/execution/src/support.c',folder/'support.c')
record={'original_source':source.relative_to(ROOT).as_posix(),'original_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'change':'Only explicit unused-parameter casts; no return behavior or algorithm completed. Original learner source unchanged.','generated':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in folder.glob('*.c')}}
(ROOT/'research/v5/expansion/legacy_mcu_followthrough/scaffold_registration.json').write_text(json.dumps(record,indent=2)+'\n')
