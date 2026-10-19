"""Instantiate each T11 parent and verify public successors without solution bodies."""
from datetime import datetime,timezone
from pathlib import Path
import hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[4]
stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ');base=ROOT/'build/legacy_systems_freestanding_exports'/stamp;rows=[];errors=[]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
public=ROOT/'labs/machine_image/freestanding_depth';sources=list(sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T11-*.json')))
for source in sources:
 data=json.loads(source.read_text(encoding='utf-8'));destination=base/data['project'];argv=[sys.executable,'-m','cmastery','init',data['project'],str(destination)]
 r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf-8');files=[p for p in destination.rglob('*') if p.is_file()];issues=[]
 if r.returncode:issues.append('Initialization failed')
 for p in public.rglob('*'):
  if p.is_file() and '__pycache__' not in p.parts:
   target=destination/p.relative_to(ROOT)
   if not target.is_file() or sha(target)!=sha(p):issues.append('Missing/changed public successor: '+str(p.relative_to(ROOT)))
 if any('assessor' in p.relative_to(destination).parts for p in files):issues.append('Assessor path copied')
 # Private implementation bytes must not appear under an innocent filename.
 solutions={sha(p) for p in (ROOT/'assessor/legacy_systems/freestanding_depth').glob('*.c')}
 if any(sha(p) in solutions for p in files):issues.append('Assessor implementation copied')
 if not issues:
  copied=json.loads((destination/'ASSESSMENT.json').read_text(encoding='utf-8'))
  if copied['criteria']!=data['criteria']:issues.append('Assessment changed during export')
  if not all(len(c['variants'])==1 and c['variants'][0]['minimum_days']==7 for c in copied['criteria']):issues.append('Delayed assessment contract missing')
  if 'T11.04.c' in [c['objective'] for c in copied['criteria']]:
   row=next(c for c in copied['criteria'] if c['objective']=='T11.04.c')
   if 'LSYS-T11-CARRY-2026-01' not in row.get('versioned_correction',''):issues.append('Carry correction missing')
 errors += [data['project']+': '+e for e in issues];rows.append(dict(project=data['project'],argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,files=[p.relative_to(destination).as_posix() for p in files],issues=issues))
pins=[*sources,Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',*[p for p in public.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
report=dict(passed=not errors and len(rows)==5,errors=errors,exports=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},scope='Five actual learner CLI instantiations; public content identity and assessor exclusion. This is not learner execution.')
out=ROOT/'reviews/v5/expansion/legacy_systems_freestanding/exports.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n');print(dict(passed=report['passed'],exports=len(rows),errors=errors));raise SystemExit(not report['passed'])
