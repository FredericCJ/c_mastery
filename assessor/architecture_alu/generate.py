"""Extract the actual unchanged prior four-bit core for direct comparison."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
from c_source import function_span
def source():
 p=ROOT/'assessor/algorithms/algo.c';text=p.read_text(encoding='utf8');a,b=function_span(text,'algo_alu4');body=text[a:b];return '#include "algo.h"\n'+body+'\n',dict(path=p.relative_to(ROOT).as_posix(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),function='algo_alu4',body_sha256=hashlib.sha256(body.encode()).hexdigest())
if __name__=='__main__':
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);code,provenance=source();(out/'prior.c').write_text(code);(out/'provenance.json').write_text(json.dumps(provenance,indent=2)+'\n');print('OBS actual unchanged prior ALU')
