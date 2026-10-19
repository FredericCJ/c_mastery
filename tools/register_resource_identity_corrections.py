"""Freeze narrowly reviewed unaccepted bibliographic corrections, never a bypass."""
from pathlib import Path
import hashlib,json,shutil
ROOT=Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/resource_merge_repair'
GENERATED={'work_id','edition_id','inspection_ids'}
def digest(row):return hashlib.sha256(json.dumps({k:v for k,v in row.items() if k not in GENERATED},sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf8',newline='\n')
def main():
 dest=ROOT/'research/v5/resource_identity_corrections.json';assert not dest.exists()
 conflicts=json.loads((HERE/'registry_identity_conflicts.json').read_text())
 evidence={
 'R-MOD-TL2':dict(url='https://people.csail.mit.edu/shanir/publications/Transactional_Locking.pdf',locator='PDF page1 title and byline',observed='Title agrees; byline identifies Dave Dice, Ori Shalev and Nir Shavit. Previous generic authors text was a placeholder, not another work.'),
 'R-RP-EVENTS':dict(url='https://martinfowler.com/eaaDev/EventSourcing.html',locator='Title/byline/date immediately below title',observed='Same Event Sourcing article; date12 December2005. Inspection2026 is not publication year.'),
 'R-RP-CQRS':dict(url='https://martinfowler.com/bliki/CQRS.html',locator='Title and date immediately below title',observed='Same CQRS article; date14 July2011.'),
 'R-RP-LAMPORT':dict(url='https://www.microsoft.com/en-us/research/wp-content/uploads/2016/12/Time-Clocks-and-the-Ordering-of-Events-in-a-Distributed-System.pdf',locator='Publication footer July1978, volume21 number7',observed='Same article/publication. Hosting path2016 and inspection2026 do not change1978 publication.'),
 'R-RP-HLC':dict(url='https://engineering.buffalo.edu/computer-science-engineering/research/technical-reports.html',locator='Technical report2014-04 entry',observed='University catalogue identifies matching title/authors and May7,2014; actual2014-04.pdf title page agrees.',capture='build/replicated_sources/hlc.pdf'),
 'R-RP-CRDT':dict(url='https://dsf.berkeley.edu/cs286/papers/crdt-tr2011.pdf',locator='Retained actual PDF cover: report7506/version1/date',observed='Cover identifies RR7506, January2011 and version1 dated13 January2011. Current web reopening failed; local retained PDF was read.',capture='build/replicated_sources/crdt.pdf'),
 'R-RP-DELTA':dict(url='https://arxiv.org/pdf/1603.01529v1',locator='PDF page1 arXiv identifier/date/byline',observed='Same Delta State paper; v1 dated4 March2016, byline Almeida, Shoker, Baquero.',capture='build/replicated_sources/delta.pdf'),
 'R-RP-OT':dict(url='https://www.lri.fr/~mbl/ENS/CSCW/2015/papers/Ellis-SIGMOD89.pdf',locator='Retained PDF page1 byline/copyright and last page number',observed='Same title and C.A.Ellis/S.J.Gibbs byline; ACM1989 copyright, pages399–407. Full-name expansion is retained from the canonical producer; current web reopening failed, and no new full-author-name attribution from the abbreviated PDF is claimed.',capture='build/replicated_sources/ot.pdf'),
 'R-RP-MERKLE':dict(url='https://www.rfc-editor.org/rfc/rfc9162.txt',locator='RFC header and Authors Addresses',observed='RFC9162 is Experimental, December2021; named authors Ben Laurie, Eran Messeri and Rob Stradling.'),
 'R-RP-SHA':dict(url='https://www.rfc-editor.org/rfc/rfc6234.txt',locator='RFC header and Authors Addresses',observed='RFC6234 is May2011; D.Eastlake3rd/T.Hansen header and Donald Eastlake/Tony Hansen addresses resolve previous corporate placeholder.'),
 }
 assert {r['id'] for r in conflicts}==set(evidence)
 for e in evidence.values():
  e.update(inspection_date='2026-10-18',scope='Bibliographic identity only; original algorithmic inspection scope/eligibility not expanded.')
  if 'capture' in e:e['sha256']=sha(ROOT/e['capture'])
 evidence_path=HERE/'identity_evidence.json';put(evidence_path,dict(schema_version=1,records=evidence,limits=['New inspection checks identity metadata only, not algorithm proofs or whole-document access.','Prior register corrections and their existing bounded inspection remain authoritative for substantive source claims.']))
 history=HERE/'history_identity_01';assert not history.exists();history.mkdir()
 shutil.copyfile(ROOT/'references/resources.json',history/'previous_registry.json')
 rows=[]
 for row in conflicts:
  assert row['accepted'] is False
  old=row['previous'];new=row['incoming'];fields=sorted(k for k in ('title','authors','edition','year','url') if old.get(k)!=new.get(k))
  assert fields and set(fields)<= {'authors','edition','year'} and old['title']==new['title'] and old['url']==new['url']
  rows.append(dict(id=row['id'],previous_sha256=digest(old),incoming_sha256=digest(new),identity_fields=fields,previous=old,incoming=new,evidence=dict(path=evidence_path.relative_to(ROOT).as_posix(),sha256=sha(evidence_path),id=row['id']),reason='Replace a generic author or inspection-year placeholder with the already inspected publication identity of the same title and URL. Never changes an accepted resource.'))
 put(dest,dict(schema_version=1,normalization='UTF8 JSON sorted keys, ensure_ascii=False, compact separators; omit only work_id/edition_id/inspection_ids',previous_registry=dict(path=(history/'previous_registry.json').relative_to(ROOT).as_posix(),sha256=sha(history/'previous_registry.json')),corrections=rows))
 print(dict(corrections=len(rows),scope='Exact old/new normalized rows, same title/URL, unaccepted only'))
if __name__=='__main__':main()
