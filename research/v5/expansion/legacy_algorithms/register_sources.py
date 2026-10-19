"""Record only the primary passages actually read for the T15 successor."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
CLRS=[
'2.1 insertion records and invariant obligations; held PDF41-44',
'7.1 partition regions, updates and initial quicksort complexity distinction; held PDF232-235',
'8.2 counting representation/stability and8.3 stable LSD radix/cost; held PDF261-267',
'9.3 groups-of-five selection and recurrence/substitution; held PDF294-299',
'14.3 optimal substructure and subproblem-space choice opening; held PDF462-464',
'15.1 half-open activity selection and exchange proof/recursive procedure; held PDF504-509',
'15.2 greedy-choice distinction and0/1 versus fractional knapsack; held PDF514-518',
'20.2 BFS queue/discovery invariants and aggregate cost; held PDF658-663',
'20.3 DFS forest/discovery/finish and aggregate cost; held PDF669-672',
'20.4 topological definition, DFS algorithm and back-edge lemma; held PDF679-681',
'22.1 Bellman-Ford edge relaxation/reachable cycle check and initial proof; held PDF724-726',
'22.3 Dijkstra nonnegative precondition, settled-set proof and scan/heap costs; held PDF732-736',
'32.2 rolling recurrence, exact-hit verification and stated probability assumptions; held PDF1124-1129',
'32.4 prefix function, KMP procedures and aggregate fallback count; held PDF1139-1144']
def record(ident,title,authors,edition,year,url,locators,sha,method,limits,document=None):
 return dict(id=ident,title=title,authors=authors,edition=edition,year=year,kind='primary-source',url=url,access='Actual bounded primary passages inspected',domains=['T15'],role='primary',reason='Evidence for mechanism invariants, distinctions and the original C transfer work.',prerequisites=['C object bounds, integer arithmetic and loop invariants'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method=method,scope='; '.join(locators),limits=limits,evidence_urls=[url],local_sha256=sha,local_document_id=document))
resources=[record('R-ALGORITHMS-CLRS','Introduction to Algorithms: selected T15 mechanism passages',['Thomas H. Cormen','Charles E. Leiserson','Ronald L. Rivest','Clifford Stein'],'Fourth edition',2022,'https://www.cs.dartmouth.edu/thc/',CLRS,'4f45c515e713ecd1170a1f829d9bbdf46da16faf6a35272ebb0efb4bd9632370','Pinned read-only held ordinary PDF exact ID/hash; bounded text pages actually read; private extracts not published','No entire-book inspection; opening source proofs do not establish all later lemmas. Original C code adds bounds, failure and ownership contracts absent from mathematical pseudocode.','99f5aeb0-74bc-5125-afce-59aa34ee286e'),
record('R-ALGORITHMS-N1570','WG14 N1570 integer representations and conversion',['ISO/IEC JTC1/SC22/WG14'],'C11 committee draft2011-04-12',2011,'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',['6.2.6.2p1-6 signed/unsigned representations; printed45-46','6.3.1.3p1-3 integer conversions','7.20.1.1p1-3 exact-width types;7.20.1.2p1-3 minimum-width types;printed290'],'208969fec021e4f4ce87a1873765298c73e7bbb5209b1c5868967c7adfc8492e','Actual downloaded primary PDF; bounded extracted passages read','C11 draft is not exact published C17 or C23; those edition-access gaps remain.'),
record('R-ALGORITHMS-SCANNING','Crafting Interpreters: token representation and number scanning',['Robert Nystrom'],'Author-maintained book source snapshot',2021,'https://raw.githubusercontent.com/munificent/craftinginterpreters/master/book/scanning.md',['Lines192-275 lexemes/types/literal values and offset/length locations','Lines557-657 number literal grammar, ASCII digit choice, lookahead and Java conversion boundary'],'ad5f489b9669841c5304eb74e61a9be086f35291eca92279f0be8c1e54f02761','Actual original author repository Markdown fetched and selected lines read','Published website fetch failed; author repository source succeeded and is pinned. Referenced generated code snippets not expanded here; Java conversion delegation is explicitly forbidden for the C target.'),
record('R-ALGORITHMS-PARSING','Crafting Interpreters: ambiguity, recursive descent and errors',['Robert Nystrom'],'Author-maintained book source snapshot',2021,'https://raw.githubusercontent.com/munificent/craftinginterpreters/master/book/parsing-expressions.md',['Lines66-175 ambiguity, precedence and associativity','Lines336-442 left-recursion removal and grammar-to-code mapping','Lines621-704 syntax-error requirements and recovery distinction'],'d4c18389676590e19566fd6166888dcd41d8f221f4df4280b44effb2ebc1341a','Actual original author repository Markdown fetched and selected lines read','Bounded prose/grammar inspection, not all generated Java snippets or complete parser implementation; original C uses precedence climbing and a smaller explicit grammar.')]
(HERE/'resources.json').write_text(json.dumps(dict(resources=resources),indent=2)+'\n',encoding='utf8')
files=[HERE/'resources.json',HERE/'sources.md',pathlib.Path(__file__)]
gate=dict(status='passage-inspected',scope='T15 author source support; no independent content acceptance',resource_ids=[r['id'] for r in resources],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact published C17/C23 text remains inaccessible in this inspection.','Book/source inspection and author finite experiments do not supply independent learner or physical evidence.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
