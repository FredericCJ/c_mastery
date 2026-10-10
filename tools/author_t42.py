"""Materialize original T42 teaching and data after its scoped research gate."""
import json
from common import ROOT, read_json, dump, write, require_research_gates

def ref(key):
    return '{ref}' + chr(96) + key + chr(96)

def document(path, meta, body):
    fence=chr(96)*3
    write(ROOT/path, fence+'json\n'+json.dumps(meta,ensure_ascii=False,indent=2)+'\n'+fence+'\n\n'+body.strip()+'\n')

def fixture(name, description, inputs, expected, method):
    path='fixtures/network_http/'+name+'.json'
    dump(ROOT/path, {'id':'FX-HTTP-'+name,'description':description,'input':inputs,'expected':expected,'verification_method':method})
    return path

def main():
    require_research_gates()
    gate=read_json(ROOT/'reviews/enhancement_research_gate.json')
    if 'T42' not in gate.get('accepted_domains',[]):
        raise ValueError('T42 research has not been accepted')
    proposed=read_json(ROOT/'research/enhancement/networking_taxonomy.json')['domains']
    domain=next(d for d in proposed if d['id']=='T42')
    contract=next(f for f in read_json(ROOT/'research/enhancement/networking_contracts.json')['families'] if f['domain']=='T42')
    contract_path=fixture('contract','Original shared bounded HTTP profile, used by hosted and custom-TCP adapters.',
        contract, {'health_representation_hex':'6f6b0a','health_octets':3,'routes':['/health','/echo'],'physical_layers_owned_by_server':False},
        'Check exact capacities and route/body/ownership declarations; these are project policies, not observed server results.')
    fixtures=[]
    fixtures.append(fixture('01_lifecycle','Bounded listener/connection ownership events.',
        {'capacity':4,'accepted_fds':[11,12,13,14,15],'close_fd':12,'replacement_fd':16,
         'events':['listener_readable','accept_EAGAIN','accept_EINTR','accept_fd']},
        {'admitted':[11,12,13,14],'rejected':[15],'after_release_and_replacement':[11,13,14,16],
         'required_flags':['nonblocking','close_on_exec'],'race_leaks':0},
        'Independent set/capacity accounting; real accept flags and OS errors must be observed separately.'))
    fixtures.append(fixture('02_grammar','Original header octets and grammar outcomes; every split has the same header boundary.',
        {'cases':[
            {'wire':'GET /health HTTP/1.1\r\nHost: localhost:8080\r\n\r\n','accepted':True},
            {'wire':'GET http://localhost:8080/health HTTP/1.1\r\nHost: other.example\r\n\r\n','accepted':True,'authority':'localhost:8080'},
            {'wire':'GET /health HTTP/1.1\nHost: localhost:8080\n\n','accepted':False},
            {'wire':'GET /health HTTP/1.1\r\nHost : localhost\r\n\r\n','accepted':False},
            {'wire':'GET /health HTTP/1.1\r\nHost: localhost\r\n folded\r\n\r\n','accepted':False},
            {'wire':'GET /health HTTP/1.1\r\nHost: local\u0000host\r\n\r\n','accepted':False}
        ]},
        {'accepted_case_indices':[0,1],'rejected_case_indices':[2,3,4,5],
         'valid_header_end_octets':[46,66],'absolute_target_authority':'localhost:8080'},
        'Independent octet/regular-language checks for these finite examples; no general HTTP parser is supplied.'))
    fixtures.append(fixture('03_framing','Finite header-policy table for body length selection.',
        {'cases':[
          {'headers':[['Host','localhost']]},
          {'headers':[['Content-Length','5']]},
          {'headers':[['Content-Length','5'],['content-length','5']]},
          {'headers':[['Transfer-Encoding','chunked'],['Content-Length','5']]},
          {'headers':[['Transfer-Encoding','chunked']]},
          {'headers':[['Transfer-Encoding','gzip, chunked']]},
          {'headers':[['Content-Length','+5']]},
          {'headers':[['Content-Length','4294967296']]},
          {'headers':[['Content-Length','1025']]}
        ],'length_integer_max':4294967295,'body_limit':1024},
        {'outcomes':[['NONE',0,False],['LENGTH',5,False],['ERROR',400,True],['ERROR',400,True],
                     ['CHUNKED',0,False],['ERROR',501,True],['ERROR',400,True],
                     ['ERROR',400,True],['ERROR',413,True]]},
        'Separately normalize finite header pairs and derive duplicate/precedence/checked-decimal outcomes.'))
    fixtures.append(fixture('04_chunks','Chunk boundaries, extensions and trailer separation.',
        {'valid_wire':'3;x=y\r\nabc\r\n2\r\nde\r\n0\r\nX-Debug: ignored\r\n\r\n',
         'incomplete_wire':'3\r\nabc\r\n0\r\nX-Debug: ignored\r\n',
         'invalid_wire':'3\r\nabcX\r\n0\r\n\r\n','terminal_only':'0\r\n\r\n'},
        {'body_hex':'6162636465','body_octets':5,'trailers_discarded':[['X-Debug','ignored']],
         'incomplete_publishes':0,'invalid_status':400,'terminal_body_hex':''},
        'Reconstruct known chunk data by explicit sizes and delimiters; verify exact terminal trailer boundary and EOF predicates.'))
    fixtures.append(fixture('05_semantics','Route, HEAD and response metadata expectations.',
        {'utc_epoch_seconds':1672531200,'cases':[
            {'method':'GET','target':'/health','body_hex':''},
            {'method':'HEAD','target':'/health','body_hex':''},
            {'method':'POST','target':'/echo','body_hex':'00ff410a'},
            {'method':'GET','target':'/unknown','body_hex':''},
            {'method':'POST','target':'/health','body_hex':''},
            {'method':'FROB','target':'/health','body_hex':''}
        ],'expectation_cases':['admissible 100-continue','unknown expectation','invalid TE+CL with 100-continue']},
        {'date':'Sun, 01 Jan 2023 00:00:00 GMT',
         'responses':[
           {'status':200,'length':3,'body_hex':'6f6b0a'},
           {'status':200,'length':3,'body_hex':''},
           {'status':200,'length':4,'body_hex':'00ff410a'},
           {'status':404,'length':10,'body_hex':'6e6f7420666f756e640a'},
           {'status':405,'length':19,'body_hex':'6d6574686f64206e6f7420616c6c6f7765640a','allow':'GET, HEAD'},
           {'status':501,'length':16,'body_hex':'6e6f7420696d706c656d656e7465640a'}
         ],'expectation_first_status':[100,417,400]},
        'UTC date from independent standard-library formatting; literal response byte counts, route table and HEAD exception checked separately.'))
    fixtures.append(fixture('06_output','Short-send ownership and bounded FIFO close behavior.',
        {'output_hex':'414243444546474849','send_results':[4,'EINTR',2,'EAGAIN',3],
         'request_ids':[1,2,3],'queue_capacity':2,'close_request_id':2},
        {'offsets_after_each_result':[4,4,6,6,9],'emitted_hex':'414243444546474849',
         'initial_queue':[1,2],'paused_request':3,'response_order':[1,2],'cancelled_after_close':[3]},
        'Independent offset arithmetic and FIFO model; counts are local acceptance, not peer delivery.'))
    fixtures.append(fixture('07_deadlines','Independent idle and total deadlines resist slow-progress renewal.',
        {'idle_ms':5000,'request_total_ms':15000,'output_total_ms':15000,'close_drain_ms':1000,
         'request_start':0,'progress':[4000,8000,12000,14900],'now':15000,
         'output_start':100,'output_now':15100,'drain_start':200,'drain_now':1200},
        {'idle_expired':False,'request_total_expired':True,'output_expired':True,'drain_expired':True,
         'request_status':408,'output_success_claim':False},
        'Deadline comparisons at exact boundaries; UTC wall time never substitutes for elapsed-time scheduling.'))
    fixtures.append(fixture('08_security','Close policy and independent-client evidence expectations.',
        {'requests':[
          {'name':'CONNECT plus optimistic bytes','method':'CONNECT','following_request':True},
          {'name':'Upgrade plus optimistic bytes','method':'GET','upgrade':True,'following_request':True},
          {'name':'TE+CL followed by apparent request','ambiguous_framing':True,'following_request':True},
          {'name':'Connection close followed by pipeline','method':'GET','connection_close':True,'following_request':True}
        ]},
        {'first_status':[501,501,400,200],'close':[True,True,True,True],
         'following_requests_dispatched':[0,0,0,0],'tls_owned':False,'lower_layers_owned':'OS'},
        'Finite policy oracle; independent curl/raw-client execution and application C inspection remain learner gates.'))
    wire_path=fixture('09_wire','Exact shared close-mode HTTP serialization for hosted and custom-TCP adapters.',
        {'mode':'one request per connection, close','utc_epoch_seconds':1672531200,
         'routes':['GET /health','HEAD /health','POST /echo'],'echo_body_hex':'00ff410a',
         'header_order':['status-line','Date','Content-Type','Content-Length','Connection'],
         'interim_continue':'HTTP/1.1 100 Continue\r\n\r\n'},
        {'messages':[
          {'method':'GET','header':'HTTP/1.1 200 OK\r\nDate: Sun, 01 Jan 2023 00:00:00 GMT\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 3\r\nConnection: close\r\n\r\n','body_hex':'6f6b0a'},
          {'method':'HEAD','header':'HTTP/1.1 200 OK\r\nDate: Sun, 01 Jan 2023 00:00:00 GMT\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 3\r\nConnection: close\r\n\r\n','body_hex':''},
          {'method':'POST','header':'HTTP/1.1 200 OK\r\nDate: Sun, 01 Jan 2023 00:00:00 GMT\r\nContent-Type: application/octet-stream\r\nContent-Length: 4\r\nConnection: close\r\n\r\n','body_hex':'00ff410a'}
        ]},
        'Independently parse literal header octets and verify Date, declared representation length, HEAD exception, binary body and terminal CRLF. Fixed field order is original fixture policy, not a universal RFC requirement.')

    lessons=[
"""A socket delivers an ordered stream whose chunk boundaries belong to the current call, not to HTTP. OS TCP performs sequence/retransmission work. The application owns descriptor lifecycle, byte retention and message interpretation. Reuse the earlier Linux accept/event adapter, then attach four explicitly owned connection records. Accept readiness is only a hint: another action can consume the pending connection, so EAGAIN returns to the event loop. On Linux an accepted descriptor does not inherit the listener's nonblocking status; set and verify flags explicitly. A failed initialization must close the descriptor exactly once before publishing a record.

Trace accepted descriptors 11,12,13,14 into four slots. Descriptor15 cannot acquire a slot and is closed. Releasing12 allows16 to enter; no reference to12 may survive in a queued event. Add a generation number to distinguish a recycled descriptor number from the old connection. Record this invariant: every live descriptor has one owner and every published record has a valid descriptor. A readable listener does not guarantee a successful accept, and an open socket does not establish an HTTP session. Evidence includes flag inspection, an accept-race trace, injected exhaustion, resource counters and an explicit layer-ownership map.""",
"""Treat input as bounded octets. A recv call may stop between CR and LF, between a field name and colon, or inside a request target. Save state and the precise consumed offset; never append a NUL then assume all octets are text. The strict project grammar uses one SP between request-line elements and CRLF delimiters. Field names use the HTTP token alphabet and case-insensitive matching; whitespace before the colon, folding, NUL and invalid controls cause rejection before publication. Strict recipient policy is allowed here but is not a claim that the RFC has no tolerant parsing provisions.

For a GET header ending at offset46, feed prefixes of lengths0 through46. Every proper prefix reports NEED_MORE with no complete request; the final LF completes the header once. Feeding two headers in one span completes only the first and returns its consumed length, leaving the second unchanged. Absolute-form http://localhost:8080/health selects authority localhost:8080, even if the syntactically valid single Host says other.example. Origin-form uses the single Host. Validate bounded configured host/port authorities; do not implement filesystem normalization. Missing/duplicate/malformed HTTP1.1 Host returns400 and close. A complete header is not yet a complete body. Retain split/coalescing, target-authority and smallest malformed-octet evidence.""",
"""After headers, select exactly one body mode. The method name does not define the wire boundary: a GET can carry framed content, while a POST without length fields has zero request-body length. This project consumes a framed body before routing even when a route later rejects it. NONE means zero body; LENGTH requires exact unsigned decimal syntax; CHUNKED means the decoder owns termination. Header lookup must retain duplicate occurrence counts rather than overwriting one field with another.

Derive a checked decimal accumulation condition before each multiply/add using the declared maximum4294967295. Then apply the1024-byte body cap. An overflowing numeral is400; a representable declared body above the cap is413. Identical duplicate Content-Length is conservatively400+close. TE+CL is always400+close, rather than choosing whichever parser seems convenient. A transfer-coding other than the supported sole final chunked coding is501+close. For LENGTH5 and input abcdeGET..., consume exactly five body octets; GET belongs to the next message. Premature EOF after four bytes is incomplete and cannot dispatch a successful echo. Produce the framing-decision table, byte-conservation ledger and overflow counterexamples.""",
"""Chunked transfer encoding is a framing grammar. Maintain SIZE_LINE, DATA, DATA_CRLF, TRAILERS and COMPLETE states with independent counters. Hex accumulation and aggregate size are checked before accepting bytes. A valid bounded unknown extension is syntactically parsed and ignored; rejecting every unknown extension would evade the receiver contract. Trailer fields are validated in separate bounded storage and discarded. They never replace Host, Content-Length or routing decisions.

Trace 3;x=y CRLF abc CRLF 2 CRLF de CRLF 0 CRLF X-Debug: ignored CRLF CRLF. The representation is the five octets abcde. The zero-size chunk ends data but does not finish the message until the trailer section's final empty line. A CR split across reads is still pending state, not an optional delimiter. The supplied invalid trace puts X where the data-ending CR belongs and returns400+close; the incomplete trace has no terminal empty line and publishes no body. Check every byte split, extension/trailer capacities and EOF in every state. Keep decoded content separate from wire byte counts; the wire overhead is not echoed.""",
"""Routing begins only after a complete admissible request, except that header-determined errors and100-continue need early handling. GET and HEAD /health select the representation hex6f6b0a: three octets, including LF. HEAD sends its GET-equivalent Content-Length3 with no representation body. POST /echo returns identical bounded octets, including NUL and non-UTF8 values. Unknown route returns404. A recognized method disallowed on the selected route returns405 and route-specific Allow; an unknown method returns501. Declare bounded supported methods GET,HEAD,POST and distinguish recognition from permission.

For an admissible POST with Expect100-continue and body not yet received, enqueue the interim100 immediately, then wait for the framed body. If headers already establish an error, enqueue that final error instead. Waiting for the body before100 can deadlock a cooperative client. Emit Date using a reasonable UTC clock; fixture epoch1672531200 yields Sun, 01 Jan 2023 00:00:00 GMT in the required fixed-date format. Use a separate monotonic clock for deadlines. Length is an octet count, not Unicode character count. User data never becomes unchecked header syntax or a format string. Evidence includes exact serialization, binary echo, HEAD boundary, Allow and expectation traces.""",
"""Keep immutable response storage until every queued byte is locally accepted or the connection aborts. A send of four bytes advances the offset by four; EINTR and EAGAIN advance it by zero. Sending again from offset0 duplicates output. POLLOUT interest is needed only while pending bytes remain; a permanent writable interest can spin an otherwise idle event loop. Local send success is not evidence that the peer application consumed the response.

For nine output octets and results4,EINTR,2,EAGAIN,3, the retained offsets are4,4,6,6,9. A two-entry request queue holds requests1,2 and pauses parsing3. Persistent responses remain FIFO even if their computation costs differ. The default maximum is four requests per connection; the fourth response announces close. If request2 requests Connection:close,3 is never dispatched. Drain the final response, shutdown the write direction, then receive/discard remaining bytes only until EOF or the1000ms drain deadline, and close. The discarded bytes are not new requests. Record read backpressure, output retention, FIFO and close-state transitions with tiny-send and coalesced-input tests.""",
"""Resource limits are part of the state machine. Idle activity and total elapsed request time are independent. A byte every four seconds can keep a five-second idle timer fresh while violating a fifteen-second total request deadline. Do not renew the total deadline on every byte. Bound headers at8192 octets,32 fields, target1024 octets, body1024 octets, extension128 octets per chunk and trailer1024 octets/eight fields; apply limits before storing. Track four connections, two complete queued requests and four requests per connection. Derive actual memory from chosen representations, including retained spans, output, queues and error paths.

With request start0, progress4000,8000,12000,14900 and now15000ms, idle has not expired but the total deadline has. A request timeout returns bounded408+close where output remains usable; output timeout aborts and makes no completed-response claim. POLLHUP may arrive with readable buffered bytes: consume those before treating recv0 as EOF. A half-close after a complete request can still allow its response; one inside a declared body remains incomplete. EPIPE/reset aborts output and releases ownership. Retain monotonic-clock traces, failure-state distinctions and allocator/descriptor high-water evidence rather than calling every close successful.""",
"""A parser boundary is a security boundary because two participants can disagree on where a request ends. Compare your exact framing decisions with an independent raw client and the current RFC contract. A mature client is an oracle peer, not permission to import an HTTP parser/server library. Reject unsupported CONNECT or Upgrade with a final error and close; optimistic following bytes never become another dispatched request. RFC9931's inspected CONNECT proxy requirements are background authority, while this origin-server rejection policy is an original declared policy.

Run curl for GET,HEAD,binary POST and expected100-continue behavior, and a separate raw-octet peer for duplicate lengths, TE+CL, invalid controls, chunks, pipelines and half-close. Packet segmentation, socket read chunks and HTTP messages are three different units. Save normalized status/header/body results and exact byte traces where framing matters. Test that the same request under all partitions produces one result and that a close/error prevents any second application effect. Publish component/source/dependency ownership, unresolved deviations and limits. Plain loopback HTTP has no TLS, proxy or production deployment claim. Static fixture checks cannot replace executing and inspecting the learner's original C."""
    ]
    titles=['Owned socket lifecycle','Incremental header and authority parser','Unambiguous body framing',
            'Chunk decoder with separate trailers','Router and HTTP response serializer','Persistent output and close state',
            'Deadline and failure campaigns','Independent client and parsing-security defense']
    hours=[6,8,8,8,8,10,8,8]
    specifics=[
        ('Reuse the prior listener/event code; implement bounded records and their acquisition/release transitions.',
         'Observe listener flags and accepted flags; inject readiness races and the fifth concurrent connection.'),
        ('Write a portable header state machine with consumed-length reporting and configured authorities.',
         'Run every split of the valid examples and selected two-request coalescings; demonstrate no early publication.'),
        ('Add occurrence-counted normalized fields and checked framing selection, then consume the exact selected body.',
         'Inject every framing-table case and EOF at each Content-Length prefix; preserve the following message unchanged.'),
        ('Implement chunk-size, extension, data-CRLF and trailer states with independent caps.',
         'Split at every byte, corrupt each delimiter, exceed each cap and verify no partial body publication.'),
        ('Add the original route table, HEAD metadata, status/Allow, Date and100-continue state.',
         'Verify exact binary content, generated UTC metadata and no wait-before100 deadlock.'),
        ('Attach immutable output and FIFO request records to the socket adapter, then stage closure.',
         'Force the short-send script, two-entry queue pressure and close followed by apparent pipeline bytes.'),
        ('Add independent monotonic deadlines and distinguish input EOF from reset/output failure.',
         'Inject the recorded clock cases, readable hangup, partial-body half-close and stalled output.'),
        ('Combine the preceding modules and defend the declared original C/OS/library boundary.',
         'Use curl and an independent raw peer; save expected/actual semantic results and minimized disagreements.')
    ]
    pids=[]
    for index,(title,budget,lesson,specific) in enumerate(zip(titles,hours,lessons,specifics),1):
        pid=f'PR-T42-{index:02}'; pids.append(pid)
        meta={'id':pid,'title':title,'module':'T42','topics':[f'T42.{index:02}'],
              'status':'core','hours':budget,'profile':'C' if 2<=index<=5 else 'A',
              'fixtures':[contract_path,fixtures[index-1]]+([wire_path] if index in [5,6,8] else [])}
        body=f"""# {title}

## Contract

{specific[0]} This is milestone{index} of the hosted HTTP family. The shared contract is the supplied contract fixture; preserve its capacities, routes, body bytes and close behavior. The application owns the studied mechanism. The operating system supplies TCP/IP and the physical network. Reuse earlier C/ownership/Linux components and accepted evidence; this budget charges the new HTTP boundary work. No equivalent HTTP parser/server library may implement the target.

## Inputs and expected results

Use the accompanying finite fixture and shared contract. Its expected object is an oracle for the declared examples, not proof of every runtime path. The /health representation is exactly hex6f6b0a. Record consumed, retained, published and discarded octets separately. Match every listed status, length, ownership and timing result, and explain any environment-dependent evidence separately.

## Steps

1. Read the corresponding T42 topic and its assigned source locators; predict the fixture before coding.
2. State input preconditions, live storage, output ownership, error results and progress measure.
3. {specific[0]}
4. {specific[1]}
5. Compare the smallest observed trace with the independent fixture calculation and preserve the first disagreement.
6. Audit component dependencies, record actual commands/versions and retain separate evidence for objectives a,b,c.

{lesson.split(chr(10)+chr(10))[1]}

## Failure cases

Include empty input, every relevant incomplete prefix, malformed syntax, exact capacity and one beyond capacity. Exercise error then retry/cleanup, pending output and a later event referring to a recycled descriptor. For a portable parser, no failure may publish a partial request or retain a span after its owner releases storage. For an adapter, no failure may leak a descriptor, advance an unsent offset or claim peer consumption from local send success.

## Acceptance and evidence

{specific[1]} Run the saved Python fixture verifier, then execute the learner's original C against these and independently added counterexamples. Retain a per-objective checklist, dependency/symbol audit, byte/state trace and memory/descriptor ledger. A successful static oracle does not establish actual parser, socket, hardware or full HTTP conformance. Explain the selected RFC requirement, any stricter project policy and every omitted capability independently.

## Hints and recovery

Reduce a failing exchange to one request, one state and one split point. Compare the promised consumed offset and ownership before adding logging. Keep the raw input immutable for diagnosis. If the oracle disagrees, recompute byte lengths or deadline arithmetic independently before changing expected data. Return to the corresponding entry foundation when the defect is a pointer lifetime, integer overflow, partial-I/O or event-ownership mistake.

## Transfer challenge

After a delay, change one split, capacity, connection-generation or time boundary and predict the outcome from the contract. Demonstrate which evidence survives and which must be regenerated. Carry the portable HTTP core into T41 only after parser/serializer gates pass; its original implementation hours remain here, while T41 pays for its different transport adapter and physical closure.
"""
        document(f'curriculum/projects/{pid}.md',meta,body)

    readings=[
      ('R-NET-UNP-3','3.9 readn, writen, and readline Functions','Explain stream prefixes and contrast blocking convenience routines with a nonblocking event adapter.',1),
      ('R-NET-LINUX-IO-6-19','accept(2): NOTES (readiness race)','Derive accepted-descriptor flags, races and acquisition failure handling.',1),
      ('R-NET-LINUX-IO-6-19','recv(2): RETURN VALUE','Distinguish stream prefixes, EOF and retry/wait conditions.',1),
      ('R-NET-LINUX-IO-6-19','send(2): MSG_NOSIGNAL','Derive partial-output ownership and broken-peer behavior.',1),
      ('R-NET-LINUX-IO-6-19','poll(2): DESCRIPTION (POLLIN/POLLOUT/POLLHUP, timeout and event masks)','Design event interest, buffered hangup and bounded waiting.',1),
      ('R-NET-RFC9112','2.2 Message Parsing','Extract octet-parsing obligations and strict policy choices.',1),
      ('R-NET-RFC9112','3.2.2 absolute-form','Derive target-authority precedence and bounded absolute-target support.',1),
      ('R-NET-RFC9112','6.3 Message Body Length','Construct framing precedence and failure/close table.',1),
      ('R-NET-RFC9112','7.1 Chunked Transfer Coding','Derive incremental chunk and terminal-trailer boundaries.',1),
      ('R-NET-RFC9112','7.1.1 Chunk Extensions','Parse then ignore bounded valid unknown extensions.',1),
      ('R-NET-RFC9112','7.1.2 Chunked Trailer Section','Keep trailers separate from routing and framing.',1),
      ('R-NET-RFC9110','9.3.2 HEAD','Separate representation length from emitted body.',1),
      ('R-NET-RFC9110','9.3.1 GET','Extract GET representation semantics before defining the health route.',1),
      ('R-NET-RFC9110','9.3.3 POST (opening semantics)','Define bounded echo processing under the selected POST behavior.',1),
      ('R-NET-RFC9110','10.1.1 Expect','Prevent waiting-for-body deadlock and emit early final errors.',1),
      ('R-NET-RFC9110','6.6.1 Date','Distinguish UTC response metadata from monotonic deadlines.',1),
      ('R-NET-RFC9112','9.6 Tear-down','Stage write completion, shutdown/drain and close.',1),
      ('R-NET-RFC9931','8 Requirements for HTTP CONNECT','Identify proxy-transition scope and defend original unsupported-transition origin policy.',1)
    ]
    meta={'id':'T42','title':domain['title'],'track':'HTTP application and Linux systems','status':'core',
          'prerequisites':['T15','T18','T23','T24','T32'],'prelude_objectives':[],
          'workload':{'reading':20,'implementation':46,'verification':18,'assessment':6,'consolidation':6},
          'readings':[{'resource_id':r,'locator':l,'purpose':p,'hours':h} for r,l,p,h in readings],
          'projects':pids,'profile':['A','C'],'equipment':['Linux x86-64 host or WSL','C17 toolchain and debugger','curl and independent raw-octet client','Injected UTC and monotonic clocks']}
    body="""# From-scratch HTTP server over OS sockets

## Purpose and outcomes

Build an original bounded HTTP/1.1 origin server in C. It owns incremental octet parsing, framing, routes, serialization, descriptor/buffer lifetime, bounded queues and deadlines. It listens on loopback127.0.0.1:8080. The OS supplies TCP/IP and lower layers. This is the third communication family; the first is the original seven-layer protocol and the second supplies its own Ethernet-to-TCP implementation. The portable HTTP core developed here is reused by the second family.

## Entry check and remediation

Demonstrate span bounds and retained-pointer lifetimes; predict short read/write/EINTR/EAGAIN behavior; trace an event-loop resource acquisition and cleanup; derive checked integer accumulation. Reuse T15/T18 parsing/ownership and T23/T24 Linux adapters. A failed prediction returns to those exact mechanisms before implementing a larger server. Inspect the shared contract and distinguish octets, TCP stream, HTTP message and application representation. Do not assume that one read contains a request.

## Study sequence and teaching

"""
    for topic,lesson in zip(domain['subtopics'],lessons):
        body+=f"### {topic['id']} — {topic['title']}\n\n{lesson}\n\n"
    body+="""## Reading guidance

Read UNP for the short-I/O mechanism, then extract the current Linux return/error and readiness rules. Its blocking helpers are explanation rather than event-loop code to copy. Read RFC9112 in the order grammar → framing → chunking → persistence/teardown, constructing a decision table for the project instead of reading the entire protocol at once. RFC9110 supplies method/expectation/Date semantics; it cannot replace wire-framing rules. RFC9931 updates HTTP1.1 transition security with explicitly scoped CONNECT proxy requirements.

The assigned table contains exact inspected passages. Adjacent RFC sections may be consulted as reference; assigning new obligatory passages requires inspection and registry extension. Distinguish the RFC obligation, a stricter original resource/syntax policy and an excluded capability in the evidence ledger. Current primary contracts take precedence over old textbook behavior.

## Implementation projects

"""
    body+='\n'.join('- '+ref(p)+' — '+str(h)+' h; core.' for p,h in zip(pids,hours))
    body+="""\n\n## Mastery gate and feedback

Every topic needs separate a,b,c evidence. Defend one successful path and one minimized failure without reading the implementation. Publish a byte-conservation ledger and demonstrate identical semantic results under every selected partition/coalescing of the input. Show one output backpressure trace, one deadline that survives slow progress, and one close/error followed by apparent request bytes with no second dispatch. The complete gate includes actual original C execution with independent curl/raw peers, current source contracts, memory/descriptor limits, symbol/dependency inspection and delayed reconstruction.

A fixture verifier checks static oracles only. Parser bugs, send success, coverage percentages and cooperative-client success do not establish every HTTP receiver requirement. Minimize disagreement before changing the contract. A body/framing defect returns to the portable parser; an event/lifetime defect returns to the adapter. Record excluded features and unresolved actual execution explicitly.

## Consolidation and transfer

After at least one day reconstruct framing precedence, HEAD length and the partial-send invariant from memory. Change a split, duplicate header, EOF point or timeout boundary and predict the new result. Carry the accepted portable parser/serializer into T41 through a span/event adapter. Keep its original project IDs and hours; the new adapter must demonstrate preservation of bytes and ownership over custom TCP rather than charge for rewriting HTTP again. See the communication-family guide for the cross-domain route.

## Environment and limits

The hosted executable uses profileA; parser/router/serializer components use profileC with caller storage, allowed type headers and explicit time/config callbacks. Audit compiler helpers and callbacks separately; no external HTTP library implements the studied mechanisms. Linux-specific MSG_NOSIGNAL, descriptor flags and poll behavior are declared platform contracts. curl and raw clients are verification peers only. The server is plain loopback HTTP with bounded routes, not TLS, a proxy, a filesystem server, a tunnel or a production deployment. Hardware lower-layer closure belongs to T40/T41 and cannot be claimed from these sockets.
"""
    document('curriculum/modules/T42.md',meta,body)
    print('Authored T42: eight topics, eight bounded milestones, ten shared/specific fixtures.')

if __name__=='__main__':
    main()
