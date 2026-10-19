"""Observe compiler/header acceptance without inventing a normative matrix."""
import argparse,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('compiler');p.add_argument('directory',type=Path);a=p.parse_args();a.directory.mkdir(exist_ok=True)
rows=[]
for edition in ('c90','c99','c11','c17','c23'):
 for header in ('stdbool','stdint','stdatomic','threads','stdckdint','stdbit'):
  path=a.directory/(edition+'-'+header+'.c');path.write_text('#include <'+header+'.h>\nint main(void){return 0;}\n')
  command=[a.compiler,'-std='+edition,'-pedantic-errors','-Wall','-Wextra','-Werror','-fsyntax-only',str(path)]
  result=subprocess.run(command,capture_output=True,text=True,timeout=30)
  if result.returncode<0:raise RuntimeError('Compiler crashed')
  rows.append(dict(edition=edition,header=header,command=command,exit=result.returncode,stdout=result.stdout,stderr=result.stderr,accepted=result.returncode==0,claim='Observed header translation only; neither definitions nor link availability nor ISO edition entitlement'))
print(json.dumps(dict(cells=rows,count=len(rows))))
