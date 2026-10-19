"""Actual bounded C draft and kernel circular-buffer source inspection."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def main():
 rows=[]
 for key,title,authors,url,locators,edition,year in [
  ('N1570','C11 draft unsigned arithmetic and synchronization',['WG14'],'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',['6.2.5 paragraph9','5.1.2.4 paragraphs5–25','7.17.3 paragraphs1–16'],'N1570 committee draft April2011',2011),
  ('CIRCULAR','Linux circular-buffer contracts',['David Howells','Paul E. McKenney'],'https://docs.kernel.org/core-api/circular-buffers.html',['Complete article lines15–138: exclusivity, N−1 convention, one-sided bounds, producer/consumer barrier examples'],'Primary HTML inspected2026-10-17',2026)]:
  rows.append(dict(id='R-ASPSC-'+key,title=title,authors=authors,edition=edition,year=year,kind='committee-draft' if key=='N1570' else 'manual',url=url,access='Exact listed primary passages directly read',domains=['T26'],role='primary',reason='Unsigned counter and two-way payload ownership proof, changed normalized-index comparison',prerequisites=['T25.06–.09 atomics','T14 ring invariants'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Direct primary PDF/HTML reading through source passage tool',scope='; '.join(locators),limits='N1570 is C11 draft, not final C17; Linux code follows kernel contracts, not interchangeable ISO C semantics; no physical/learner/reviewer evidence.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T26.03 bounded author source inspection; no independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact final C17/C23 text remains unavailable.','This SPSC wave does not close advanced MPMC, reclamation or kernel obligations.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
