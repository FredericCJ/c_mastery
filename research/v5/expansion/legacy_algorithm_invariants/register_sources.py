"""Bounded fresh mechanism reads, without reproducing source extracts."""
import hashlib,json,pathlib,urllib.request
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 cache=pathlib.Path('/home/frede/cmastery-algorithm-invariant-sources');cache.mkdir(exist_ok=True);rows=[]
 specs=[('ODS','Open Data Structures:2.1 ArrayStack','https://opendatastructures.org/ods-cpp/2_1_Fast_Stack_Operations_U.html',['2.1 representation/get/set/add/remove','2.1.2 resize code and aggregate copying argument'],'Pat Morin','C++ online edition',2013),('EWD','Notes on Structured Programming EWD249','https://www.cs.utexas.edu/~EWD/transcriptions/EWD02xx/EWD249/EWD249.html',['Mathematical induction: repetition example, accessible lines168–232','An example of a correctness proof: remainder derivation, accessible lines255–343'],'Edsger W. Dijkstra','Author archive transcription',1969)]
 for key,title,url,locators,author,edition,year in specs:
  p=cache/(key+'.html')
  if not p.exists():p.write_bytes(urllib.request.urlopen(url,timeout=45).read())
  rows.append(dict(id='R-AI-'+key,title=title,authors=[author],edition=edition,year=year,kind='primary-text',url=url,access='Listed actual primary passages inspected',domains=['T36'],role='primary',reason='Representation/cost and induction/preservation distinctions for original C experiments',prerequisites=['C objects, finite arithmetic and loop invariants'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Actual primary HTML code/prose reads',scope='; '.join(locators),limits='Listed passages only; ODS C++ is not C lifetime semantics; Dijkstra remainder proof is not an inspected Euclid implementation. No wider book inspection.',external_source_sha256={str(p):sha(p)})))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8');files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='Three exact T36.01 author ordinary/changed tasks',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Exact final published C17/C23 uninspected.','Later EWD249 open timed out; no later passages credited.','Finite source/host observations do not provide independent learner, external or physical acceptance.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
