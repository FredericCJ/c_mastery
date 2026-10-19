"""Primary history/queue passage inspection; no bibliography-only claim."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def main():
 rows=[]
 for key,title,authors,url,locators,edition,year in [
  ('HW','Linearizability: A Correctness Condition for Concurrent Objects',['Maurice Herlihy','Jeannette Wing'],'https://cs.brown.edu/~mph/HerlihyW90/p463-herlihy.pdf',['Sections2.1–2.2, printed467–469: histories, well-formed participants, real-time order, pending-response extensions'],'ACM TOPLAS12(3),1990',1990),
  ('MS-CORRECTED','Corrected Michael–Scott concurrent queue pseudocode',['Maged Michael','Michael Scott'],'https://www.cs.rochester.edu/research/synchronization/pseudocode/queues.html',['Introductory corrected-version and type-preserving allocator premises','Complete nonblocking pseudocode initialize,E1–E17,D1–D20'],'Author-maintained corrected HTML, last-change2008',2008),
  ('MS-PROOF','Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms',['Maged Michael','Michael Scott'],'https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf',['Section2 Algorithms and section3.1 Safety, PDF pages3–5','Sections3.2 Linearizability and3.3 Liveness, PDF pages5–6'],'PODC1996 original paper; corrected HTML used for pseudocode',1996)]:
  rows.append(dict(id='R-AH-'+key,title=title,authors=authors,edition=edition,year=year,kind='paper' if key!='MS-CORRECTED' else 'manual',url=url,access='Listed primary passages directly read',domains=['T26'],role='primary',reason='Separate abstract histories, pending completion, concrete queue points and conditional progress',prerequisites=['T25 memory model','T26 original queue specification'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Direct primary PDF/HTML source passage reading',scope='; '.join(locators),limits='Algorithm/source premises are not a full C17 hazard-pointer proof or final ISO text; no independent learner or physical evidence.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T26.01/.02 author source inspection; no independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact final C17/C23 remains a distinct source gap.','Other T26 construction/reclamation/kernel tasks remain required.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
