"""Register only actually inspected progress/history primary passages."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
SOURCES=[
 ('PI','POSIX mutex priority protocol','The Open Group/IEEE','POSIX.1-2017 man7 reprint',2017,'https://man7.org/linux/man-pages/man3/pthread_mutexattr_getprotocol.3p.html',['DESCRIPTION31–108 including recursive inheritance50–104;ERRORS117–129']),
 ('SCHED','Linux scheduling interfaces and declared policies','Linux man-pages contributors','Primary man7 HTML inspected2026-10-17',2026,'https://man7.org/linux/man-pages/man7/sched.7.html',['API86–122;FIFO124–171;RR172–182;SCHED_OTHER273–288']),
 ('CLOCK','Monotonic clock interface','Linux man-pages contributors','Linux man-pages5.07/2020',2020,'https://man7.org/linux/man-pages/man3/clock_gettime.3.html',['CLOCK_MONOTONIC91–117;CPU clocks130–137;return values161–165']),
 ('HW90','Linearizability: A Correctness Condition for Concurrent Objects','Maurice P. Herlihy and Jeannette M. Wing','ACM TOPLAS12(3),1990',1990,'https://www.cs.columbia.edu/~wing/publications/HerlihyWing90.pdf',['Printed463–467 opening through history introduction','Printed468–470 sections2.1/2.2 and2.3 queue examples; PDF indexes5–7']),
 ('CACHE','False sharing mechanism and investigation','Linux kernel documentation contributors','Primary kernel documentation inspected2026-10-17',2026,'https://docs.kernel.org/kernel-hacking/false-sharing.html',['Complete article;What is false sharing1–36;pitfalls38–50;possible mitigations77–118']),
 ('N1570','C11 draft publication and atomic ordering','WG14','N1570 committee draft2011',2011,'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',['5.1.2.4 paragraphs1–28;7.17.3 paragraphs1–10'])]
def main():
 rows=[]
 for key,title,author,edition,year,url,locators in SOURCES:
  rows.append(dict(id='R-TPROG-'+key,title=title,authors=[author],edition=edition,year=year,kind='paper' if key=='HW90' else 'committee-draft' if key=='N1570' else 'manual',url=url,access='Listed primary passages directly inspected',domains=['T25'],role='primary',reason='Progress, conditional bounds, resource measurement and finite concurrent history obligations',prerequisites=['T25.03–.05 synchronization/queue ownership','T25.06–.09 publication and reuse'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Direct primary HTML/PDF passage reading; PDF468–470 via private pypdf extraction',scope='; '.join(locators),limits='Selected passages/editions only; not final published C17/C23, host policy proof, hardware observation or independent assessment.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'sources.md',HERE/'audit.md',pathlib.Path(__file__)]
 gate=dict(status='passage-inspected',scope='T25.10–.12 author source inspection, not independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Final published C17/C23 text remains unavailable.','Inspected POSIX2017 is not silently promoted to POSIX2024.','Bounded source reading and author experiments are distinct from independent learner/reviewer/physical evidence.'])
 (HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
