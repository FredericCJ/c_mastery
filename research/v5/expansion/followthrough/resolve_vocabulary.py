"""Apply the bounded, actually inspected N3096 glossary correction set."""
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[4]
NEW='R-wg14-v5-followthrough-n3096'
LOCATORS=[
    '3.3 argument','3.4.1 implementation-defined behavior','3.4.2 locale-specific behavior',
    '3.4.3 undefined behavior','3.4.4 unspecified behavior','3.6 byte','3.7 character',
    '3.8 constraint','3.10 diagnostic message','3.14 memory location','3.15 object',
    '3.16 parameter','3.19 value','3.19.2 indeterminate representation',
    '3.19.3 unspecified value','3.19.4 non-value representation','4. Conformance',
    '5.1.1.1 Program structure','5.1.1.2 Translation phases','5.1.1.3 Diagnostics',
    '5.1.2.1 Freestanding environment','5.1.2.2 Hosted environment',
    '5.1.2.3p1–8 Program execution','6.3.1.1 Boolean, characters, and integers',
    '6.3.1.3 Signed and unsigned integers',
    '6.2.1 Scopes of identifiers, type names, and compound literals',
    '6.2.2 Linkages of identifiers','6.2.6.1 Representations of types — General',
    '6.2.6.2 Integer types','6.2.7 Compatible type and composite type',
    '6.3.1.8 Usual arithmetic conversions','6.3.2.1 Lvalues, arrays, and function designators',
    '6.3.2.3 Pointers','6.4 Lexical elements','6.4.2.1 General (identifiers)',
    '6.5 Expressions — paragraph 5 (exceptional conditions)',
    '6.5 Expressions — paragraphs 6–7 (effective type)',
    '6.5.1.1p1–4 Generic selection','6.5.2.5p1–10 Compound literals',
    '6.5.6 Additive operators','6.7 Declarations','6.7.3 Type qualifiers',
    '6.7.3.1p1–12 Formal definition of restrict','6.7.6.3p1–20 Function declarators',
    '6.9p1–5 External definitions','6.9.2 External object definitions',
    '7.20 Checked Integer Arithmetic and 7.20.1 ckd_ macros',
    '5.1.2.4 Multi-threaded executions and data races',
    '7.17.1 Introduction','7.17.4 Fences','7.17.5 Lock-free property',
    '7.17.7.4 The atomic_compare_exchange generic functions',
    '6.5.2.2 Function calls','6.7.2.1p1–20 Structure and union specifiers',
    '6.7.4 Function specifiers','6.7.6 Declarators','6.7.6.2 Array declarators',
    '6.7.10p1–24 Initialization','6.8p1–5 Statements and blocks',
    '7.1.1 Definitions of terms','Annex C Sequence points']
CORRECTIONS={'3.19.3 non-value representation':'3.19.4 non-value representation',
             '3.19.5 unspecified value':'3.19.3 unspecified value',
             '5.1.2.3 Program execution':'5.1.2.3p1–8 Program execution',
             '6.2.1 Scopes of identifiers':'6.2.1 Scopes of identifiers, type names, and compound literals',
             '6.4.2 Identifiers':'6.4.2.1 General (identifiers)',
             '6.5.1.1 Generic selection':'6.5.1.1p1–4 Generic selection',
             '6.5.2.5 Compound literals':'6.5.2.5p1–10 Compound literals',
             '6.7.3.1 Formal definition of restrict':'6.7.3.1p1–12 Formal definition of restrict',
             '6.7.6.3 Function declarators (including prototypes)':'6.7.6.3p1–20 Function declarators',
             '7.17 Atomics <stdatomic.h>':'7.17.1 Introduction',
             '6.7.2.1 Structure and union specifiers':'6.7.2.1p1–20 Structure and union specifiers',
             '6.7.10 Initialization':'6.7.10p1–24 Initialization',
             '6.8 Statements and blocks':'6.8p1–5 Statements and blocks'}
DEFINITIONS={
 'V-core-padding':'Storage bits or bytes that do not encode an object’s value, including gaps between or after structure members. Padding is still part of the object representation and can affect bytewise comparisons; equal member values do not make memcmp an equality test. Initialization and stores have edition-specific padding rules. Layout and alignment must be obtained from the actual type/ABI, not assumed from member sizes.',
 'V-core-designated-initializer':'An initializer preceded by an array-index or member designator that selects the subobject to initialize. Later entries continue from that position unless another designator redirects them. Subobjects without explicit initializers follow the applicable default-initialization rules; this does not mean unnamed members or padding always have portable specified bytes.',
 'V-core-inline-function':'A function declared with inline, suggesting fast calls without requiring inline substitution. In the external-linkage case, if all its file-scope declarations in one translation unit include inline without extern, its definition there is an inline definition and does not supply the program’s external definition. Other declaration arrangements and static inline differ. The ISO rules must be distinguished from GNU89 inline semantics.',
 'V-core-flexible-array-member':'The permitted final incomplete-array member of a structure with more than one named member. Its accessible extent depends on the actual containing object, while sizeof the structure excludes element storage but may include trailing padding. A zero-element extent does not permit access to a fictitious element or creation of a pointer one past that fictitious element. Such structures also have restrictions on embedding and array use.',
 'V-core-variable-length-array':'An array type whose size or element type does not meet the known-constant-size conditions, including the special prototype-scope star form. An evaluated nonconstant size must be positive, and each instance’s size stays fixed during its lifetime. In C11/C17 VLA and variably modified type support is conditional; the inspected C23 draft retains conditional automatic VLA objects while requiring the other variably modified type facilities.',
 'V-core-full-expression':'An expression that is not part of another expression or a declarator, with edition-specific treatment of nonconstant size expressions for variably modified types. Examples include expression statements, controlling expressions and return expressions. A full expression is sequenced before the next full expression evaluated; that does not impose left-to-right evaluation among its own unsequenced operands.',
 'V-T25-release-sequence':'In C17 and the inspected N3096 draft, the maximal contiguous part of one atomic object’s modification order headed by a release operation, with subsequent operations performed by that same thread or being atomic read-modify-write operations. An acquire reading a value written by that sequence synchronizes with its head. This relation alone does not prove a reference-counting or reclamation algorithm correct; counter lifetime, resurrection, ordering and ownership still require arguments.',
 'V-T25-compare-and-exchange':'An atomic operation that compares the object representation specified by the selected edition with the expected representation; on success it replaces the object with desired, and on failure it updates expected. A weak operation may fail spuriously, so its caller must tolerate that possibility; a strong operation does not fail for that reason. Success and failure have separate memory-order restrictions. It is not a general semantic equality operation for arbitrary C values.',
 'V-core-happens-before':'The language’s ordering relation built from intra-thread sequencing and the specified inter-thread synchronization or dependency relations. It is not elapsed wall-clock order. Visibility follows the applicable non-atomic or atomic coherence rules; acquire/release arguments must identify which value is read. The consume dependency case has additional composition restrictions and cannot be replaced by an unconditional transitive sequencing slogan.',
 'V-core-lock-free-atomic':'The lock-free property of atomic operations on the specified type or object, exposed by the ATOMIC_*_LOCK_FREE macros and atomic_is_lock_free. A value of sometimes-lock-free requires the applicable runtime query. This property is distinct from wait freedom, latency bounds and the progress guarantee of a larger algorithm built from those operations. Signal handlers impose additional restrictions beyond this query.',
 'V-core-fence':'A language-level synchronization operation without an associated atomic memory location. A release/acquire fence synchronization argument still requires the specified intervening atomic accesses and read-from relation; two fences alone do not publish data. A relaxed thread fence has no effects. atomic_signal_fence orders a thread with a handler in that same thread. Compiler and hardware barriers have their own implementation contracts.',
 'V-core-lvalue':'An expression with an object type other than void that potentially designates an object. Lvalue conversion obtains the stored value in the contexts specified by the selected edition; it does not occur for every operand position. An lvalue need not be modifiable, and evaluating one that does not designate an object can have undefined behavior.',
 'V-core-definition':'A declaration that reserves storage for an object, supplies a function body, or first declares a typedef name or enumeration constant. The separate external-definition rule depends on linkage and use: an unused external declaration need not have a definition. Completing a structure type is not by itself a definition of an object of that type.',
 'V-core-function-prototype':'A function declaration that specifies parameter types. Under C17, an empty parameter list in a declaration supplies no parameter-type information, whereas (void) specifies no parameters. In the inspected C23 draft an empty list also specifies no parameters; every function declarator provides a prototype.',
 'V-core-compatible-type':'Types related by the edition-specific compatibility rules for qualifiers, arrays, functions and tagged types. Compatible need not mean textually identical. Cross-translation-unit declarations must satisfy those rules even when the linker accepts their symbol names; a permitted pointer conversion does not by itself establish type compatibility.',
 'V-core-effective-type':'The type governing an access to an object’s stored value. A declared object keeps its declared type. For an object with no declared type, the access rules distinguish a non-character store, copying an object representation, and other accesses; this is not simply the type of the last write. The permitted aliasing types include qualified and corresponding signed/unsigned types, qualifying aggregate/union types, and character types. A cast alone does not establish effective type.',
 'V-core-usual-arithmetic-conversions':'The operator-specific rules that select a common real type for arithmetic operands. Floating types are considered before the integer-only branch. In that branch, integer promotions precede rank and representability tests; a higher-ranked signed type does not necessarily win over an unsigned operand. For example, -1 < 1u is false after conversion to unsigned int.',
 'V-core-pointer':'A value of a pointer type, with validity for a particular operation governed by object lifetime, alignment, type and bounds rules. Valid states include pointers to objects or functions, null pointers and one-past array pointers. C also permits invalid or indeterminate pointer states; being stored in a pointer-typed object does not make a value dereferenceable.',
 'V-core-one-past-the-end-pointer':'A pointer produced just past an array’s final element, usable in the array arithmetic, subtraction and comparison cases the language permits, but not as the operand of an evaluated unary indirection. A non-array object is treated as a one-element array for these arithmetic rules. Arbitrary comparisons or arithmetic involving other objects are not licensed by this rule.',
 'V-core-volatile':'A qualifier for objects whose accesses must obey the abstract-machine rules despite changes or effects outside ordinary program execution. What constitutes an access is implementation-defined. Volatile alone supplies neither inter-thread synchronization nor a device-specific memory barrier; MMIO additionally needs the compiler, ISA and board contracts, and signal handlers have separate restrictions.',
 'V-core-restrict':'A pointer qualification imposing a based-on access discipline for each execution of its associated block. When an object accessed through that restricted association is also modified, the qualifying accesses and assignments must satisfy the restrict rules. Derived pointers may be used, and read-only aliasing is permitted; this is neither a blanket prohibition on equal pointer values nor a lifetime-long ownership type.',
 'V-core-compound-literal':'An expression that designates an unnamed object initialized by a braced initializer and yields an lvalue. In the C17 baseline, file-scope objects have static storage duration and block-scope objects have automatic storage duration associated with the enclosing block. The inspected C23 draft additionally admits specified storage classes and special function-prototype-scope cases; it is not a cast or a heap allocation.'}
changed=[]
for path in sorted((ROOT/'curriculum/vocabulary').glob('*.json')):
    doc=json.loads(path.read_text(encoding='utf-8')); touched=False
    for entry in doc.get('entries',[]):
        if entry['id'] in DEFINITIONS and entry['definition'] != DEFINITIONS[entry['id']]:
            entry['definition']=DEFINITIONS[entry['id']];touched=True
        for a in entry.get('authority',[]):
            locator=CORRECTIONS.get(a['locator'],a['locator'])
            if a['resource_id']=='R-wg14-n3096' and a.get('inspection')=='pending' and locator in LOCATORS:
                changed.append({'entry':entry['id'],'file':path.relative_to(ROOT).as_posix(),
                                'prior_authority':dict(a),'replacement_locator':locator})
                a.update(resource_id=NEW,locator=locator,inspection='registry');touched=True
        if entry['id']=='V-core-unspecified-value':
            entry['definition']='A value allowed by its type whose selection is not fixed by the specification. This classification alone does not make a surrounding computation defined: other operations still have their own preconditions.'
            entry['misconception']='An unspecified value guarantees a safe result for every operation using it (for example, a possibly zero value still requires admission before division).'
        elif entry['id']=='V-core-argument':
            entry['definition']='For a function call, an expression supplied in its parenthesized argument list. For a function-like macro, a preprocessing-token sequence supplied to the macro invocation; macro arguments are not automatically evaluated or assigned to C objects.'
        elif entry['id']=='V-core-indeterminate-representation':
            entry['definition']='In the inspected C23 draft, an object representation that represents an unspecified value or represents no value of the object type. Apply the relevant access and initialization rules before using it; physical leftover bits are not an ordinary initialized value.'
        elif entry['id']=='V-core-freestanding-implementation':
            entry['definition']='A conforming implementation whose required library facilities and startup conventions follow the selected edition’s freestanding rules. Additional facilities are implementation-specific; the curriculum’s strict profile imposes a narrower explicit runtime closure.'
            entry['edition_notes']=[{'edition':'C17 baseline / N3096 comparison',
                'note':'N3096 clause4 adds a specified string-library subset and memalignment, with conditional floating facilities. This inspected draft is not the published C23 text. Do not infer the curriculum dependency policy from a compiler language flag or from header availability.'}]
    if touched:
        path.write_text(json.dumps(doc,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
pdf=ROOT/'research/private/language/n3096_public_source.pdf'
resource={'id':NEW,'title':'Programming languages — C: N3096 glossary and environment passages',
    'authors':['ISO/IEC JTC 1/SC 22/WG 14'],'edition':'N3096, April 1, 2023','year':2023,
    'kind':'standard','url':'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf',
    'access':'Retained public primary PDF, bounded passages actually inspected',
    'domains':['T02','T03','T04','T05','T06','T07','T25'],
    'role':'oracle','reason':'Correct wrong clause numbers and overbroad semantic claims while resolving specifically inspected pending glossary locators.',
    'prerequisites':['Language versus implementation contracts','C declarations and expressions'],
    'locators':LOCATORS,'assignable':True,
    'inspection':{'date':'2026-10-17','method':'Read actual retained pdftotext passage bodies; verified public PDF SHA256',
        'scope':'Actual passage bodies for the exact locators listed above, including bounded paragraph ranges; no inspection claim for entire referenced chapters.',
        'limits':'Pre-publication C23 draft, not published ISO C17 or C23; no full standard or implementation-conformance claim.',
        'evidence_urls':['https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf'],
        'local_document_id':None,'local_sha256':hashlib.sha256(pdf.read_bytes()).hexdigest()}}
(Path(__file__).parent/'vocabulary_resources.json').write_text(json.dumps({'resources':[resource]},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
receipt=Path(__file__).parent/'vocabulary_corrections.json'
if changed:
    prior=json.loads(receipt.read_text(encoding='utf-8')).get('changed_entries',[]) if receipt.exists() else []
    receipt.write_text(json.dumps({'schema_version':1,'inspected_source':NEW,
        'original_locator_corrections':CORRECTIONS,'changed_entries':prior+changed,
        'limits':'Only listed actual passages close these source gaps. Remaining pending authorities and published-edition gaps are unchanged.'},ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print({'updated_authorities':len(changed),'resource':NEW})
