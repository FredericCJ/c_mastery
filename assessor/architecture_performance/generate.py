"""Extract unchanged selected C bodies, with their actual accounting macro."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def body(path,signature):
 text=path.read_text();start=text.index(signature);end=text.index('{',start)+1;depth=1
 while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]
def source():
 a=ROOT/'assessor/algorithms/algo.c';b=ROOT/'assessor/algorithm_abi/bound.c';c=a.read_text();macro=c[c.index('#define WORK'):c.index('int algo_ring_init')];pieces=[body(a,'int algo_lower('),body(b,'int ab_checked(')];code='#include "bound.h"\n'+macro+'\n'+'\n'.join(pieces)+'\n';record=dict(sources={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in [a,b]},bodies={n:hashlib.sha256(s.encode()).hexdigest() for n,s in zip(['algo_lower','ab_checked'],pieces)},macro_sha256=hashlib.sha256(macro.encode()).hexdigest(),policy='Exact selected function bodies and WORK macro; unrelated predecessor functions not compiled.');return code,record
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);code,record=source();(out/'prior.c').write_text(code);(out/'provenance.json').write_text(json.dumps(record,indent=2)+'\n');print('OBS exact checked lower-bound C providers')
if __name__=='__main__':main()
