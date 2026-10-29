"""Extract the exact frozen producer body, avoiding unrelated family functions."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
from c_source import function_span
def source():
 p=ROOT/'assessor/b045_systems/systems.c';raw=p.read_text(encoding='utf8');start,end=function_span(raw,'s_pipeline');body=raw[start:end];code='#include "systems.h"\n#include <limits.h>\n'+body+'\n';record=dict(source=p.relative_to(ROOT).as_posix(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),function='s_pipeline',body_sha256=hashlib.sha256(body.encode()).hexdigest(),policy='Exact unchanged original body; no other frozen family function compiled or modified.');return code,record
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);code,record=source();(out/'prior.c').write_text(code);(out/'provenance.json').write_text(json.dumps(record,indent=2)+'\n');print('OBS actual unchanged s_pipeline')
if __name__=='__main__':main()
