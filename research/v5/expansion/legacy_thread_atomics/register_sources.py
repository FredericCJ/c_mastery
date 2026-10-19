"""Bounded primary-source records for the original atomic/CAS obligations."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
SOURCES=[
 ('N1570','C11 committee draft atomic semantics','WG14','N1570 committee draft2011',2011,'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',['3.14 definition/example;5.1.2.4 paragraphs1–28','7.17.2.2;7.17.3 paragraphs1–10 and7.17.3.1','7.17.4 through7.17.4.2;7.17.5/.1;7.17.7.1 through7.17.7.4']),
 ('N3096','C23 working draft atomic comparison','WG14','N3096 working draft April2023',2023,'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf',['Cover/status;7.17.2.2 paragraphs2–6','7.17.3 paragraphs1–11 and7.17.3.1;7.17.4 through7.17.4.2','7.17.7.1 through7.17.7.5']),
 ('GCC','GCC atomic builtins and helper fallback','GNU Compiler Collection contributors','Primary current HTML inspected2026-10-17',2026,'https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html',['Memory-order introduction, consume implemented as acquire and target/runtime fallback','Atomic load/store/exchange/CAS and fetch arithmetic descriptions','Thread/signal fences and always/is_lock_free descriptions'])]
def main():
 rows=[]
 for key,title,author,edition,year,url,locators in SOURCES:
  rows.append(dict(id='R-TATOM-'+key,title=title,authors=[author],edition=edition,year=year,kind='committee-draft' if key!='GCC' else 'manual',url=url,access='Actual listed primary passages read',domains=['T25'],role='primary',reason='Required atomic object, publication, CAS and compiler/helper distinctions',prerequisites=['T25.01 thread lifetime','T25.03 synchronization ownership'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Direct primary PDF/HTML passage reading',scope='; '.join(locators),limits='Drafts and selected compiler interface only; not final published C17/C23, full documents, independent review or execution.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'sources.md',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T25.06–.09 bounded author source inspection, not independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact final C17 and published C23 text remains unavailable.','Source inspection and compiler modes do not replace target, physical, external-review or independent-learner observations.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
