"""Register only primary passages actually opened during T76 author research."""
import hashlib
from register_ad_mp_sources import write, ROOT

ROWS = [
 ('EVENTS','Event Sourcing','Martin Fowler','https://martinfowler.com/eaaDev/EventSourcing.html',['How it Works; Application State Storage; Reversing Events; External Updates; External Queries; Code Changes'],'A 2005 draft describing replay and effects; not a crash-consistency protocol.'),
 ('CQRS','CQRS','Martin Fowler','https://martinfowler.com/bliki/CQRS.html',['Separate conceptual update and query models, paragraphs beginning CQRS stands and As this occurs'],'Model separation does not itself establish exactly-once effects.'),
 ('LAMPORT','Time, Clocks, and the Ordering of Events in a Distributed System','Leslie Lamport','https://www.microsoft.com/en-us/research/wp-content/uploads/2016/12/Time-Clocks-and-the-Ordering-of-Events-in-a-Distributed-System.pdf',['Clock Condition and its non-converse, PDF p3; IR1/IR2, PDF p3; Ordering Events Totally, PDF pp3-4'],'Causal order implies increasing logical timestamps; timestamp order need not imply causality.'),
 ('HLC','Logical Physical Clocks and Consistent Snapshots in Globally Distributed Databases','Kulkarni, Demirbas, Madeppa, Avva and Leone','https://cse.buffalo.edu/tech-reports/2014-04.pdf',['Section2 logical/vector clocks; section3.3 Figure5 HLC algorithm; Theorems1-3 and Corollary1'],'Implement all receive branches and identify physical-clock assumptions behind bounded drift.'),
 ('CRDT','A comprehensive study of Convergent and Commutative Replicated Data Types, RR7506','Shapiro, Preguica, Baquero and Zawirski','https://dsf.berkeley.edu/cs286/papers/crdt-tr2011.pdf',['Sections3.1.2-3.1.3, Specifications6-7 G/PN counters; section3.2.1 LWW register; section3.3.5 Specification15 observed-remove set; section3.5.1 Specification19 RGA'],'Fixed membership, unique identities and retained tombstones are substantive assumptions.'),
 ('DELTA','Delta State Replicated Data Types','Almeida, Shoker and Baquero','https://arxiv.org/pdf/1603.01529',['Figure2 delta counter; section5 basic anti-entropy; section6 Definitions5-6 causal delta interval condition; Algorithm2 crash and acknowledgment state'],'A delta is joinable state, not a non-idempotent increment command; causal intervals need context.'),
 ('OT','Concurrency Control in Groupware Systems','Ellis and Gibbs','https://www.lri.fr/~mbl/ENS/CSCW/2015/papers/Ellis-SIGMOD89.pdf',['Section4.1 transformation matrix; section4.2 state vectors; sections4.3-4.4 algorithm and quiescence; section5 insertion transforms'],'Pairwise commuting transformed operations alone do not establish a complete multi-site protocol.'),
 ('MERKLE','RFC9162 Certificate Transparency Version2.0','IETF','https://www.rfc-editor.org/rfc/rfc9162.html',['Section2.1.1 empty/leaf/node tree hashes and split rule; section2.1.2 stack construction; section2.1.3 inclusion paths'],'Use only the defined Merkle construction on original synthetic leaves; no CT service interaction.'),
 ('SHA','RFC6234 US Secure Hash Algorithms','IETF','https://www.rfc-editor.org/rfc/rfc6234.html',['Section4.1 padding; section5.1 SHA-256 functions and constants; section6.1 initial values; section6.2 processing'],'Implement the bounded SHA-256 primitive from its equations; no external hash library in target code.'),
 ('WIRE','Protocol Buffers Encoding','Protocol Buffers project','https://protobuf.dev/programming-guides/encoding/',['Base128 Varints; Message Structure; Signed Integers; Field Order and deterministic serialization limits'],'Distinguish wire compatibility, minimal varints and a separately specified canonical encoding.'),
 ('ROLLBACK','GGPO Rollback Networking SDK','Tony Cannon / GGPO','https://www.ggpo.net/',['How Does It Work: deterministic engine, predicted inputs, divergence replay, save/load/advance without rendering'],'Separate pure simulation replay from presentation effects; no GGPO dependency in target code.'),
]

def main():
    rows=[]
    for key,title,author,url,locators,reason in ROWS:
        rows.append(dict(id='R-RP-'+key,title=title,authors=[author],edition='Named public primary passages inspected 2026-10-17',year=2026,kind='manual',url=url,access='Public primary passages inspected',domains=['T76'],role='oracle',reason=reason,prerequisites=['C17 arithmetic, storage, pure state machines and message framing'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Opened and read the named primary web or PDF passages',scope='; '.join(locators),limits='Author inspection of bounded passages only; not whole-document review, independent acceptance, deployment guidance or final ISO C wording.',evidence_urls=[url])))
    path='research/v5/modern/replicated_resources.json'
    write(path,dict(schema_version=1,resources=rows,gaps=[dict(id='T76-C17-TEXT',status='unavailable',source='Published ISO C17',reason='Published edition text not accessed; active implementation baseline remains C17.'),dict(id='T76-C23-TEXT',status='unavailable',source='Published ISO C23',reason='No final edition clauses inspected. No changed guarantee is attributed to C23.')]))
    write('research/v5/modern/replicated_research_gate.json',dict(status='passage-inspected',domains=['T76'],reviewer='author inspection only; no independent review',research_input_sha256={path:hashlib.sha256((ROOT/path).read_bytes()).hexdigest()}))
    print('Registered 11 bounded primary-source inspections for T76.')

if __name__=='__main__': main()
