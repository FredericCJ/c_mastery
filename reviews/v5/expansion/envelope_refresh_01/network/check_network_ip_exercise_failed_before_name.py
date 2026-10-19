"""Actual isolated exercise init/check and editable-source/harness boundaries."""
from pathlib import Path
import hashlib,json,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import exercises
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 rows=[];errors=[];record=ROOT/'reviews/v5/expansion/legacy_network_ip/exercise_export.json'
 def run(label,args,expected):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,timeout=240);rows.append(dict(label=label,command=list(map(str,args)),expected=expected,returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=r.returncode==expected));return r
 pins={Path(__file__),ROOT/'curriculum/v5/exercises/X-T41-002.md',ROOT/'curriculum/v5/exercises.json',ROOT/'cmastery/exercises.py',ROOT/'cmastery/cli.py',ROOT/'labs/b045_systems/include/systems.h',ROOT/'assessor/b045_systems/systems.c',ROOT/'assessor/network_followthrough/ip/ip.c'}
 pins.update(p for p in (ROOT/'labs/network_followthrough').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)}
 with tempfile.TemporaryDirectory(prefix='network-ip-author-export-') as td:
  dest=Path(td)/'exercise';value=exercises.exercise('X-T41-002')
  run('actual exercise init',[sys.executable,'-m','cmastery','init','PR-T41-05',dest,'--exercise','X-T41-002'],0)
  if any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*') if p.is_file()):errors.append('assessor path leaked')
  run('compiling learner stub rejects',[sys.executable,'-m','cmastery','check',dest,'--exercise','X-T41-002','--compiler','gcc','--profile','debug'],1)
  run('explicit author reference passes',[sys.executable,'-m','cmastery','check',dest,'--exercise','X-T41-002','--compiler','clang','--profile','release','--assessor'],0)
  implementation=dest/'src/ip.c';before=implementation.read_bytes();implementation.write_bytes(before+b'\n/* learner edit boundary probe */\n')
  try:exercises.harness(value,dest);rows.append(dict(label='source editing allowed',passed=True))
  except ValueError as ex:errors.append(str(ex))
  implementation.write_bytes(before)
  trusted=dest/'ip/tests.c';trusted.write_bytes(trusted.read_bytes()+b'\n/* modified harness */\n')
  try:exercises.harness(value,dest);errors.append('modified harness accepted')
  except ValueError:rows.append(dict(label='harness editing rejected',passed=True))
  # Explicit assessor builds legitimately create private object code under the
  # build directory. Source publication is checked before those build outputs.
  private=ROOT/'assessor/network_followthrough/ip/ip.c'
  if any(p.read_bytes()==private.read_bytes() for p in dest.rglob('*.c')):errors.append('private source leaked')
  from selected_export_bindings import bind_completed_exports
  selected=bind_completed_exports(ROOT,before,rows)
  assert selected==['X-T41-002']
 d=dict(passed=not errors and len(rows)==5 and all(r['passed'] for r in rows),errors=errors,checks=rows,input_sha256=before,selected_exercises=selected,limits=['Isolated author exercise-export execution; no independent learner or delayed assessment.','Default checksum/IP templates compile and fail semantically; explicit --assessor selects identified original references.','Source edits under src are permitted, trusted harness edits rejected; compiled assessor outputs are not published sources.']);record.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],errors=errors,checks=len(rows)));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
