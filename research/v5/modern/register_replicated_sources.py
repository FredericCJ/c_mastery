"""Register only primary passages actually opened during T76 author research."""
import hashlib
from register_ad_mp_sources import write, ROOT

ROWS = [
 ('EVENTS','Event Sourcing','Martin Fowler','https://martinfowler.com/eaaDev/EventSourcing.html',['How it Works; Application State Storage; Reversing Events; External Updates; External Queries; Code Changes'],'A 2005 draft describing replay and effects; not a crash-consistency protocol.'),
 ('CQRS','CQRS','Martin Fowler','https://martinfowler.com/bliki/CQRS.html',['Separate conceptual update and query models, paragraphs beginning CQRS stands and As this occurs'],'Model separation does not itself establish exactly-once effects.'),
 ('LAMPORT','Time, Clocks, and the Ordering of Events in a Distributed System','Leslie Lamport','https://www.microsoft.com/en-us/research/wp-content/uploads/2016/12/Time-Clocks-and-the-Ordering-of-Events-in-a-Distributed-System.pdf',['Clock Condition and its non-converse, PDF p3; IR1/IR2, PDF p3; Ordering Events Totally, PDF pp3-4'],'Causal order implies increasing logical timestamps; timestamp order need not imply causality.'),
 ('HLC','Logical Physical Clocks and Consistent Snapshots in Globally Distributed Databases','Kulkarni, Demirbas, Madeppa, Avva and Leone','https://cse.buffalo.edu/tech-reports/2014-04.pdf',['Section2 logical/vector clocks; section3.3 Figure5 HLC algorithm; Theorems1-3 and Corollary1'],'Implement all receive branches and identify physical-clock assumptions behind bounded drift.'),
 ('CRDT','A comprehensive study of Convergent and Commutative Replicated Data Types, RR7506','Shapiro, Preguica, Baquero and Zawirski','https://dsf.berkeley.edu/cs286/papers/crdt-tr2011.pdf',['Sections3.1.2-3.1.3, Specifications6-7 G/PN counters; section3.2.1 LWW register; section3.3.5 Specification15 observed-remove set; section3.5.1 Specification19 RGA','Section3.5.1 and Specification19 RGA timestamp/tombstone/downstream premises; section4.1 stability and fixed-membership liveness'],'Fixed membership, unique identities and retained tombstones are substantive assumptions.'),
 ('DELTA','Delta State Replicated Data Types','Almeida, Shoker and Baquero','https://arxiv.org/pdf/1603.01529',['Figure2 delta counter; section5 basic anti-entropy; section6 Definitions5-6 causal delta interval condition; Algorithm2 crash and acknowledgment state','Section2 system model; section6 Definitions4-6 and Proposition2; section6.1 Algorithm2 and durable-counter/full-state-fallback rationale'],'A delta is joinable state, not a non-idempotent increment command; causal intervals need context.'),
 ('OT','Concurrency Control in Groupware Systems','Ellis and Gibbs','https://www.lri.fr/~mbl/ENS/CSCW/2015/papers/Ellis-SIGMOD89.pdf',['Section4.1 transformation matrix; section4.2 state vectors; sections4.3-4.4 algorithm and quiescence; section5 insertion transforms','Sections4.1-4.4 transformation necessity, state-vector/request-log context and quiescence; sections5-6 insert/delete transforms and mixed-priority counterexample'],'Pairwise commuting transformed operations alone do not establish a complete multi-site protocol.'),
 ('UTF8','UTF-8, a transformation format of ISO 10646','Francois Yergeau','https://www.rfc-editor.org/rfc/rfc3629.txt',['Section3 scalar ranges, octet construction and surrogate exclusion'],'Use scalar values and length-delimited bytes; no grapheme or normalization equivalence is inferred.'),
 ('MERKLE','RFC9162 Certificate Transparency Version2.0','IETF','https://www.rfc-editor.org/rfc/rfc9162.html',['Section2.1.1 empty/leaf/node tree hashes and split rule; section2.1.2 stack construction; section2.1.3 inclusion paths','Sections2.1.1-2.1.3.2 raw tree domain/split, recursive inclusion path and fn/sn exact-path verifier'],'Use only the defined Merkle construction on original synthetic leaves; no CT service interaction.'),
 ('SHA','RFC6234 US Secure Hash Algorithms','IETF','https://www.rfc-editor.org/rfc/rfc6234.html',['Section4.1 padding; section5.1 SHA-256 functions and constants; section6.1 initial values; section6.2 processing'],'Implement the bounded SHA-256 primitive from its equations; no external hash library in target code.'),
 ('HMAC','HMAC: Keyed-Hashing for Message Authentication','Hugo Krawczyk, Mihir Bellare and Ran Canetti','https://www.rfc-editor.org/rfc/rfc2104.txt',['Section2 HMAC definition; section3 keys; section4 implementation note; section6 final paragraph on implementation and key protection'],'Derive the keyed construction separately from authorization and replay policy.'),
 ('HMAC-VECTORS','Identifiers and Test Vectors for HMAC-SHA-224, HMAC-SHA-256, HMAC-SHA-384, and HMAC-SHA-512','Magnus Nystrom','https://www.rfc-editor.org/rfc/rfc4231.txt',['Sections4.2,4.3,4.7 SHA256 full-output test cases1,2,6; section5 limits of the test-vector security claim'],'Published expected values test algorithm agreement, not security of a deployment.'),
 ('WIRE','Protocol Buffers Encoding','Protocol Buffers project','https://protobuf.dev/programming-guides/encoding/',['Base128 Varints; Message Structure; Signed Integers; Field Order and deterministic serialization limits','Length-Delimited Records and Submessages; Last One Wins; Field Order and Implications'],'Distinguish wire compatibility, minimal varints and a separately specified canonical encoding.'),
 ('WIRE-EVOLUTION','Proto Serialization Is Not Canonical; proto2 Unknown Fields','Protocol Buffers project','https://protobuf.dev/programming-guides/serialization-not-canonical/',['Implications on Fingerprinting and Cryptographic Hashing; Deterministic is not Canonical; Inherent Barriers to Stable Serialization','proto2 Language Guide: Unknown Fields and Retaining Unknown Fields'],'Original bounded schema identity must distinguish opaque bytes, future nested interpretation and preservation versus reconstruction.'),
 ('ROLLBACK','GGPO Rollback Networking SDK','Tony Cannon / GGPO','https://www.ggpo.net/',['How Does It Work: deterministic engine, predicted inputs, divergence replay, save/load/advance without rendering'],'Separate pure simulation replay from presentation effects; no GGPO dependency in target code.'),
]

def main():
    rows=[]
    for key,title,author,url,locators,reason in ROWS:
        rows.append(dict(id='R-RP-'+key,title=title,authors=[author],edition='Named public primary passages inspected 2026-10-17',year=2026,kind='manual',url=url,access='Public primary passages inspected',domains=['T76'],role='oracle',reason=reason,prerequisites=['C17 arithmetic, storage, pure state machines and message framing'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method='Opened and read the named primary web or PDF passages',scope='; '.join(locators),limits='Author inspection of bounded passages only; not whole-document review, independent acceptance, deployment guidance or final ISO C wording.',evidence_urls=[url])))
        if key=='WIRE-EVOLUTION':
            rows[-1]['inspection']['date']='2026-10-18'
            rows[-1]['inspection']['evidence_urls'].append('https://protobuf.dev/programming-guides/proto2/#unknown-fields')
            rows[-1]['edition']='Named current primary passages inspected2026-10-18; no complete schema/library inspection'
        if key=='EVENTS':
            rows[-1].update(year=2005,edition='Draft dated12 December2005; named primary passages inspected2026-10-17')
        if key=='CQRS':
            rows[-1].update(year=2011,edition='Article dated 14 July 2011; named primary passages reinspected 2026-10-17')
        if key=='LAMPORT':
            rows[-1].update(year=1978,edition='Communications of the ACM 21(7), July 1978; PDF pages 3-4 reinspected 2026-10-17',kind='paper')
        if key=='HLC':
            rows[-1].update(year=2014,edition='University at Buffalo technical report 2014-04; section 2, Figure 5 and Theorems 1-3/Corollary 1 reinspected 2026-10-17',kind='paper')
        if key=='DELTA':
            rows[-1].update(year=2016,authors=['Paulo Sérgio Almeida','Ali Shoker','Carlos Baquero'],edition='arXiv:1603.01529v1, 4 March 2016; named passages reinspected 2026-10-17',kind='paper')
            rows[-1]['inspection']['method']='Read named PDF passages and visually inspected rendered Algorithm2 on PDF page12'
        if key=='CRDT':
            rows[-1].update(year=2011,edition='INRIA RR7506, version1, 13 January2011; named primary passages reinspected2026-10-17',kind='paper')
        if key=='OT':
            rows[-1].update(year=1989,authors=['Clarence A. Ellis','Simon J. Gibbs'],edition='ACM SIGMOD1989, pages399-407; named passages reinspected2026-10-17',kind='paper')
        if key=='UTF8':
            rows[-1].update(year=2003,edition='RFC3629, November2003; section3 inspected2026-10-17')
        if key=='MERKLE':
            rows[-1].update(year=2021,authors=['Ben Laurie','Eran Messeri','Rob Stradling'],edition='RFC9162, December2021, Experimental; sections2.1.1-2.1.3.2 reinspected2026-10-17')
            rows[-1]['inspection']['evidence_urls'].append('https://www.rfc-editor.org/rfc/rfc9162.txt')
        if key=='SHA':
            rows[-1].update(year=2011,authors=['Donald Eastlake 3rd','Tony Hansen'],edition='RFC6234, May2011; sections4.1,5.1,6.1,6.2 reinspected2026-10-17')
            rows[-1]['inspection']['evidence_urls'].append('https://www.rfc-editor.org/rfc/rfc6234.txt')
        if key=='HMAC':
            rows[-1].update(year=1997,authors=['Hugo Krawczyk','Mihir Bellare','Ran Canetti'],edition='RFC2104; named passages inspected2026-10-17')
        if key=='HMAC-VECTORS':
            rows[-1].update(year=2005,edition='RFC4231; named passages inspected2026-10-17')
    path='research/v5/modern/replicated_resources.json'
    write(path,dict(schema_version=1,resources=rows,gaps=[dict(id='T76-C17-TEXT',status='unavailable',source='Published ISO C17',reason='Published edition text not accessed; active implementation baseline remains C17.'),dict(id='T76-C23-TEXT',status='unavailable',source='Published ISO C23',reason='No final edition clauses inspected. No changed guarantee is attributed to C23.')]))
    write('research/v5/modern/replicated_research_gate.json',dict(status='passage-inspected',domains=['T76'],reviewer='author inspection only; no independent review',research_input_sha256={path:hashlib.sha256((ROOT/path).read_bytes()).hexdigest()}))
    print('Registered '+str(len(rows))+' bounded primary-source inspections for T76.')

if __name__=='__main__': main()
