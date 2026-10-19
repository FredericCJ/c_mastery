"""Register directly inspected identity, protection and atomic-query passages."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def main():
 rows=[]
 for key,title,authors,url,locators,edition,year in [
  ('MICHAEL','Hazard Pointers: Safe Memory Reclamation for Lock-Free Objects',['Maged Michael'],'https://www.cs.otago.ac.nz/cosc440/readings/hazard-pointers.pdf',['Section2.3 ABA and still-live node reinsertion, printed493','Section3.3 continuous protection condition, printed494–496','Section4 adaptation outline, printed496–497'],'IEEE TPDS15(6),2004',2004),
  ('MS-ABA','Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms',['Maged Michael','Michael Scott'],'https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf',['Introduction ABA/tag-width/index-encoding discussion, PDF page2'],'PODC1996 original paper',1996),
  ('N1570','C11 draft atomic lock-free query contracts',['WG14'],'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',['7.17.5 paragraphs1–2','7.17.5.1 paragraphs1–3'],'N1570 committee draft April2011',2011)]:
  rows.append(dict(id='R-AI-'+key,title=title,authors=authors,edition=edition,year=year,kind='committee-draft' if key=='N1570' else 'paper',url=url,access='Listed primary passages directly inspected',domains=['T26'],role='primary',reason='Distinguish incarnation, protection, logical ABA, finite tag capacity and measured atomic dependency',prerequisites=['T25 atomic semantics','T26 identity and progress'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Direct primary PDF source passage reading',scope='; '.join(locators),limits='N1570 is not final C17/C23; historical hardware remarks are not current ISA guarantees; excerpts do not certify a complete concurrent hazard-pointer implementation.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T26.05 identity author inspection; no independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Final C17/C23 source comparison remains distinct.','No universal logical-ABA, target execution or full reclamation claim.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
