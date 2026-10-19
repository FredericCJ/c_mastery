"""Exercise real T19 learner export and exclude assessor bodies."""
from datetime import datetime,timezone
from pathlib import Path
import subprocess,sys
from common import ROOT,read_json,dump,sha
support=read_json(ROOT/'research/v5/expansion/legacy_abstractions/support.json');projects=sorted({r['project'] for r in support['criteria']});base=ROOT/'build/legacy_abstractions_exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[];pins=[]
public=[p for dirname in ('labs/api_depth/abstractions','labs/api_depth/contracts','labs/api_depth/stream') for p in (ROOT/dirname).glob('*') if p.is_file()]+[ROOT/'labs/api_depth/include/api_depth.h',ROOT/'labs/foundations/include/foundation.h'];solutions={sha(p) for p in (ROOT/'assessor/api_depth/abstractions').glob('*.c')}
for project in projects:
 source=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');data=read_json(source);target=base/project;argv=[sys.executable,'-m','cmastery','init',project,str(target)];r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf-8');issues=[]
 if r.returncode:issues.append('CLI initialization failed')
 for p in public:
  copied=target/p.relative_to(ROOT)
  if not copied.is_file() or sha(copied)!=sha(p):issues.append('Public successor missing/changed: '+p.relative_to(ROOT).as_posix())
 files=[p for p in target.rglob('*') if p.is_file()]
 if any('assessor' in p.relative_to(target).parts or sha(p) in solutions for p in files):issues.append('Private solution leaked')
 if not issues:
  exported=read_json(target/'ASSESSMENT.json')
  if exported['criteria']!=data['criteria']:issues.append('Original criterion/changed contract altered')
  if not all(c['variants'][0]['minimum_days']==7 for c in exported['criteria']):issues.append('Delayed contract lost')
 errors += [project+': '+x for x in issues];rows.append(dict(project=project,argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,issues=issues,files=[p.relative_to(target).as_posix() for p in files]));pins.append(source)
pins += [Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',*public]
dump(ROOT/'reviews/v5/expansion/legacy_abstractions/exports.json',dict(passed=not errors and bool(projects),errors=errors,exports=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},scope='Actual supported-parent CLI exports, identity and assessor exclusion; no learner attempt.'))
print(dict(passed=not errors,exports=len(rows),errors=errors));raise SystemExit(bool(errors))
