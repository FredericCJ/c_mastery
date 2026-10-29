"""Extract the unchanged selected predecessor; do not replace its algorithm."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
from c_source import function_span
def source():
 p=ROOT/'assessor/algorithms/algo.c';raw=p.read_text(encoding='utf8');start,end=function_span(raw,'algo_cache');body=raw[start:end];code='#include "algo.h"\n'+body+'\n';record=dict(source=p.relative_to(ROOT).as_posix(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),function='algo_cache',body_sha256=hashlib.sha256(body.encode()).hexdigest(),policy='Exact unchanged selected function; unrelated predecessor functions not compiled.');return code,record
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);code,record=source();(out/'prior.c').write_text(code);(out/'provenance.json').write_text(json.dumps(record,indent=2)+'\n');print('OBS actual unchanged algo_cache')
if __name__=='__main__':main()
