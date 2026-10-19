"""Independent CEIR byte-decoder and raw-CFG fuzz campaigns."""
from pathlib import Path
import subprocess,json,hashlib,datetime,struct,tempfile
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/subset_compiler/extension';invest=ROOT/'labs/subset_compiler/investigations';ref=ROOT/'assessor/subset_compiler/extension';rows=[]
 def run(label,cmd,marker=None):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==0 and (marker is None or marker in p.stdout+p.stderr)));return p
 run('compiler identity',['clang','--version'])
 with tempfile.TemporaryDirectory(prefix='cm-b06-ir-') as tmp:
  w=Path(tmp)
  for mode in ['ir','verifier']:
   exe=w/mode;corpus=w/(mode+'-corpus');corpus.mkdir()
   # Independently encoded CONST17,RET, plus a loop and a mismatched stack branch.
   programs=[[(1,17),(16,0)],[(1,1),(13,4),(14,0),(1,17),(16,0)],[(1,0),(13,5),(1,0),(9,0),(14,6),(1,0),(16,0)]]
   for i,code in enumerate(programs):
    if mode=='ir':
     header=b'CEIR'+bytes([1,1])+struct.pack('<H',len(code));fn=struct.pack('<HHB',0,len(code),1)+bytes([1])+bytes(15)+b'main'+bytes(12);body=b''.join(bytes([op,0,0,0])+struct.pack('<I',arg) for op,arg in code);data=header+fn+body
    else:data=bytes(3)+b''.join(struct.pack('<II',op,arg) for op,arg in code)
    (corpus/str(i)).write_bytes(data)
   if run(mode+' build',['clang','-std=c17','-O1','-g','-fsanitize=fuzzer,address,undefined','-I',lab,ref/'extension.c',ref/'serialization.c',ref/'thumb1.c',invest/(mode+'_fuzz.c'),'-o',exe]).returncode==0:run(mode+' independently seeded 20000-input campaign',[exe,'-runs=20000','-seed=581605','-max_len=4252',corpus],marker='DONE')
 files=[Path(__file__),lab/'extension.h',*[ref/n for n in ['extension.c','serialization.c','thumb1.c']],*[p for p in invest.iterdir() if p.is_file()]];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['20000 inputs per independently seeded entrance are bounded falsification, not soundness proof','Raw-CFG harness restricts one function; full call-graph campaign remains separately represented in ordinary extension tests','Canonical roundtrip is an additional invariant, not a substitute for independently encoded bytes and semantic tests']);(ROOT/'research/v5/expansion/b06/ir_fuzz_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows))));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
