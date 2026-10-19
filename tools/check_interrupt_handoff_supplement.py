"""Actual ELF placement, capture-checker controls and inspected-source bindings."""
from pathlib import Path
import datetime,hashlib,json,re,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_handoff';REPORT='reviews/v5/expansion/legacy_advanced_handoff/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in [Path(__file__),ROOT/REPORT,ROOT/'tools/check_interrupt_handoff_capture.py',ROOT/'fixtures/embedded/interrupt_handoff_capture.json',ROOT/'curriculum/guides/interrupt-handoff-board-protocol.md',HERE/'resources.json',HERE/'source_access.json']};rows=[];artifacts={};external={}
 report=json.loads((ROOT/REPORT).read_text());assert report['passed']
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for ins in ['0','1']:
    label=cc+'-O'+opt+'-i'+ins+'-v0';row=next(r for r in report['checks'] if r['label']==label+' build');cmd=row['command'];elf=Path(cmd[cmd.index('--build')+1])/'image.elf';artifacts[elf.relative_to(ROOT).as_posix()]=sha(elf)
    r=subprocess.run(['arm-none-eabi-nm','-S',str(elf)],capture_output=True,text=True);assert r.returncode==0
    found=re.findall(r'^([0-9a-f]+)\s+([0-9a-f]+)\s+[bB]\s+queue$',r.stdout,re.M);assert len(found)==1
    address,size=[int(v,16) for v in found[0]];assert address%4==0 and size==252
    rows.append(dict(label=label,command=['arm-none-eabi-nm','-S',str(elf)],returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,queue_address=address,queue_size=size,atomic_alignment=4,passed=True))
 for selftest in [True,False]:
  cmd=[sys.executable,str(ROOT/'tools/check_interrupt_handoff_capture.py'),*(['--self-test'] if selftest else [str(ROOT/'fixtures/embedded/interrupt_handoff_capture.json')])];r=subprocess.run(cmd,capture_output=True,text=True)
  if selftest:assert r.returncode==0 and json.loads(r.stdout)==dict(passed=True,negative_controls=10,physical_observed=False)
  else:assert r.returncode==1 and 'AssertionError' in r.stderr
  rows.append(dict(label='capture selftest' if selftest else 'unexecuted template rejected',command=cmd,returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=True))
 resources=json.loads((HERE/'resources.json').read_text())['resources'];assert len(resources)==6
 for r in resources:
  inspection=r['inspection'];assert inspection['scope'] and inspection['method']
  for field in ['source_sha256','external_source_sha256']:
   for p,h in inspection.get(field,{}).items():
    f=Path(p) if Path(p).is_absolute() else ROOT/p;assert f.is_file() and sha(f)==h,(r['id'],p)
    if f.is_relative_to(ROOT):pins[f.relative_to(ROOT).as_posix()]=h
    else:external[str(f)]=h
  rows.append(dict(label='source '+r['id'],passed=True,scope=inspection['scope']))
 d=dict(passed=True,checks=rows,input_sha256=pins,outputs_sha256=artifacts,external_input_sha256=external,limits=['Four-byte atomic alignment and252-byte queue observed under the selected Arm ABI; not a portable layout guarantee.','Capture checker controls use explicitly synthetic data; supplied physical claims are not independently authenticated.','Source identity binds actual inspected passages, not an entire book or final ISO edition.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_handoff/supplemental_validation.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=True,checks=len(rows)))
if __name__=='__main__':main()
