"""Resolve the last eight glossary authorities from bounded actual readings."""
import hashlib
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[4]
FOLDER = Path(__file__).parent
BOOK = 'R-FOLLOWTHROUGH-CSAPP-3E'
PROV = 'R-FOLLOWTHROUGH-N2676'
CACHE = 'R-FOLLOWTHROUGH-FALSE-SHARING'
GCC = 'R-FOLLOWTHROUGH-GCC15-PROTECTOR'
LOCATORS = {
 'V-T23-virtual-memory': (BOOK, '9.6, printed pp.849–851: translation and fault path'),
 'V-core-page-fault': (BOOK, '9.6, printed pp.849–851: page-table present bit and fault path'),
 'V-core-copy-on-write': (BOOK, '9.8.1–9.8.2, printed pp.871–872: private mapping and fork'),
 'V-T25-cache-line': (BOOK, '6.4–6.4.1, printed pp.650–652: cache organization and S/E/B/m model'),
 'V-T45-stack-corruption': (BOOK, '3.10.3, printed pp.315–316: out-of-bounds stack writes'),
 'V-core-stack-protector': (GCC, 'Instrumentation Options: -fstack-protector, -all, -strong and -explicit'),
 'V-T25-false-sharing': (CACHE, 'What is False Sharing; How to detect; Possible mitigations'),
 'V-core-pointer-provenance': (PROV, '1.2–1.3, draft printed pp.5–7: proposed provenance and storage instances')}
DEFINITIONS = {
 'V-T23-virtual-memory': 'An execution environment mechanism translating a virtual address space to physical storage with mapping and protection state. The operating system manages mappings and the processor performs target-specific translation and access checks. A valid virtual mapping need not currently have resident backing. Translation caches, page sizes, permissions and faults belong to the target/OS contract, not ISO C.',
 'V-core-page-fault': 'A processor exception associated with virtual-memory translation or access protection, under the named architecture. It can represent a recoverable nonresident mapping or copy-on-write write, or an access the operating system cannot satisfy. It does not necessarily cause disk I/O or process termination. Do not identify every invalid C access with a guaranteed page fault.',
 'V-core-copy-on-write': 'A storage-sharing strategy that defers making a private copy until a write requires separation. For a private mapped page, a write-protected shared backing can fault and be copied before the write resumes. Linux fork can use this for private process memory; explicitly shared mappings retain shared semantics. Page granularity, allocation failure and fault costs are part of the environment contract, not ISO C or a promise that fork copies no memory.',
 'V-T25-cache-line': 'The fixed-size block tracked by a particular cache level, with data and associated tag/state. Its size and relation to the coherence unit depend on the architecture. Alignment and mapping affect conflicts and sharing; a compiler object is not inherently one line. State the level, line size and coherence assumptions before deriving traffic or layout claims.',
 'V-T45-stack-corruption': 'An invalid write that damages storage belonging to a stack frame or stack-management state. A local buffer overrun can overwrite nearby data or control information under a particular ABI/layout, but ISO C does not prescribe that layout or an exploitable result. Diagnose the first supported invalid write, compare the corrected bounds/lifetime contract and keep any observed control effect confined to the original teaching target.',
 'V-core-stack-protector': 'Compiler instrumentation that checks a guard associated with selected stack frames and transfers to a failure path on mismatch. GCC option variants select different functions; optimized-away objects need not cause instrumentation. It detects only corruptions that affect the checked guard before a check, and does not make an earlier invalid access defined or protect every object/control edge. Inspect generated code and the guard/failure runtime dependencies, especially in freestanding builds.',
 'V-T25-false-sharing': 'Coherence interference when concurrently accessed, logically independent data occupy the same coherence block and at least one participant writes. A reader can therefore suffer false sharing with a writer. Padding or separating data may reduce transfers but also increases footprint and cache/TLB pressure; measure under stated placement, affinity, access and coherence assumptions rather than assuming a universal speedup.',
 'V-core-pointer-provenance': 'Information about the storage origin of a pointer, used in proposed C abstract-machine models and in reasoning about permissible alias analysis. N2676 models a pointer using an abstract address and storage-instance provenance, which need not be stored as runtime bits. Equal representations do not by themselves justify crossing object/lifetime bounds. This draft technical specification is not the published C17 or C23 standard and does not establish that all compilers implement one agreed provenance model.'}

def dump(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')

changes=[]
for path in (ROOT/'curriculum/vocabulary').glob('*.json'):
    doc=json.loads(path.read_text(encoding='utf-8')); touched=False
    for entry in doc.get('entries', []):
        ident=entry['id']
        if ident in DEFINITIONS and entry['definition'] != DEFINITIONS[ident]:
            entry['definition']=DEFINITIONS[ident]; touched=True
        if ident in LOCATORS:
            rid, loc=LOCATORS[ident]
            for authority in entry['authority']:
                if authority.get('inspection')=='pending':
                    changes.append({'entry':ident,'file':path.relative_to(ROOT).as_posix(),
                        'prior_authority':dict(authority),'replacement_resource':rid,'replacement_locator':loc})
                    authority.update(resource_id=rid, locator=loc, inspection='registry'); touched=True
    if touched: dump(path,doc)
resources=[]
for rid,title,authors,edition,year,kind,url,scope,limits in [
 (BOOK,'Computer Systems: A Programmer\'s Perspective, bounded cache/VM/stack passages',['Randal E. Bryant','David R. O\'Hallaron'],'Third edition, Global edition',2016,'book','https://csapp.cs.cmu.edu/3e/home.html','Only listed printed pages (same PDF page numbers); stack protector discussion pp.322–324 also read.','No whole-chapter inspection. Linux/x86 examples are not ISO C or all-target guarantees; shared mappings are excluded from the private COW claim.'),
 (PROV,'A provenance-aware memory object model for C, draft N2676',['WG14'],'2021-03-24 draft technical specification N2676',2021,'standard','https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2676.pdf','Introduction opening and sections1.2–1.3 printed pp.5–7 actually read.','Not the published TS6010, C17 or C23. Proposed PNVI variants are not a universal compiler contract; no execution or complete model verification.'),
 (CACHE,'False Sharing, Linux kernel documentation',['Linux kernel contributors'],'Web documentation retrieved2026-10-17; no release inferred',None,'documentation','https://docs.kernel.org/kernel-hacking/false-sharing.html','Mechanism, detection and mitigation body sections actually read.','Kernel implementation examples and mitigation costs depend on workload/architecture. This reading is not a measurement.'),
 (GCC,'GCC instrumentation options: stack protector selection',['GNU Compiler Collection contributors'],'GCC15.2.0',2025,'documentation','https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Instrumentation-Options.html','The four stack-protector options and adjacent stack-check/stack-clash distinction actually read.','Compiler-specific selection/runtime contract; not a proof that a particular image is instrumented or free of corruption.')]:
    resources.append({'id':rid,'title':title,'authors':authors,'edition':edition,'year':year,'kind':kind,
      'url':url,'access':'Bounded actual passage inspection','domains':['T23','T25','T45'],
      'role':'oracle','reason':'Replace pending authority with precisely inspected material and preserve semantic/implementation boundaries.',
      'prerequisites':['C object bounds and lifetime','Language versus execution-environment contracts'],
      'locators':[loc for source,loc in LOCATORS.values() if source==rid],'assignable':True,
      'inspection':{'date':'2026-10-17','method':'Read actual bounded text, not catalog or search metadata','scope':scope,'limits':limits,
        'evidence_urls':[url],'local_document_id':None,'local_sha256':None}})
resources[0]['inspection'].update(local_document_id='3a8cc812-ba6a-5c70-86d7-df4ff59b7cef',local_sha256='12a2858c7572e1f337e381bd524d7b061f9068e39135fc09fc9135dd036bd021')
resources[1]['inspection']['local_sha256']=hashlib.sha256((ROOT/'research/private/v5/followthrough/n2676.pdf').read_bytes()).hexdigest()
dump(FOLDER/'memory_vocabulary_resources.json',{'resources':resources})
path=FOLDER/'memory_vocabulary_corrections.json'
if changes:
    old=json.loads(path.read_text()).get('changed_entries',[]) if path.exists() else []
    dump(path,{'schema_version':1,'changed_entries':old+changes,'limits':'Source inspection and correction, not execution or external review.'})
print({'updated_authorities':len(changes)})
