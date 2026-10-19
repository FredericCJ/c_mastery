"""Migrate only inspected passages; preserve the old authority in a correction log."""
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
FOLDER=Path(__file__).parent
BOOK='R-FOLLOWTHROUGH-TLPI-2010'
MAN='R-FOLLOWTHROUGH-MAN-6-19'
POSIX='R-FOLLOWTHROUGH-POSIX-2017'
COURSE='R-FOLLOWTHROUGH-COURSE-CONTRACTS'
QUEUE='R-FOLLOWTHROUGH-MICHAEL-SCOTT-1996'
LOCATORS={
 'V-T23-fork':(BOOK,'24.2–24.2.1, printed pp.515–519'),
 'V-T23-wait-reaping':(BOOK,'26.1.1–26.1.3, printed pp.541–545'),
 'V-T23-zombie-process':(BOOK,'26.2, printed pp.553–554'),
 'V-T23-orphan-process':(MAN,'PR_SET_CHILD_SUBREAPER(2const), DESCRIPTION and HISTORY'),
 'V-T23-pipe':(BOOK,'44.1–44.2, printed pp.889–892'),
 'V-T23-redirection':(MAN,'dup(2), DESCRIPTION and NOTES'),
 'V-T23-shared-memory':(BOOK,'54.1–54.2, printed pp.1108–1111'),
 'V-T23-file-locking':(BOOK,'55.1–55.2, pp.1117–1120; 55.3, pp.1124–1127; 55.3.5–55.3.6, pp.1136–1137'),
 'V-T25-semaphore':(MAN,'sem_overview(7), DESCRIPTION; sem_post(3), ERRORS and NOTES'),
 'V-T25-read-write-lock':(POSIX,'pthread_rwlock_rdlock(3p), DESCRIPTION and ERRORS'),
 'V-T25-priority-inversion':(POSIX,'pthread_mutexattr_getprotocol(3p), DESCRIPTION and ERRORS'),
 'V-T25-deadlock':(BOOK,'30.1.4, printed p.639'),
 'V-T25-starvation':(BOOK,'55.3.6, printed p.1137'),
 'V-T44-core-dump':(BOOK,'22.1, printed pp.448–449'),
 'V-core-abnormal-termination':(BOOK,'20.15, printed pp.418–419'),
 'V-T25-bounded-queue':(QUEUE,'Introduction, PDF pp.1–2: progress and retained storage counterexample'),
 'V-T25-livelock':(QUEUE,'Introduction, PDF p.1: historical non-blocking definition and terminology caveat'),
 'V-core-claim-scope':(COURSE,'curriculum/v4/workflow.md: Accept the specific evidence demonstrated'),
 'V-core-working-unit':(COURSE,'curriculum/v4/workflow.md: Start a study session; active comprehensive scope correction'),
 'V-T25-thread-pool':(COURSE,'curriculum/projects/PR-P5-02.md: Contract, Steps and Acceptance; bounded pool is the course design choice'),
 'V-T44-symptom':(COURSE,'curriculum/guides/assessment.md: Debugging practice and feedback'),
 'V-T44-cause':(COURSE,'curriculum/guides/assessment.md: Debugging practice and feedback'),
 'V-T44-first-divergence':(COURSE,'curriculum/guides/assessment.md: Debugging practice and feedback'),
 'V-T44-competing-hypothesis':(COURSE,'curriculum/guides/assessment.md: Debugging practice and feedback')}
DEFINITIONS={
 'V-T23-fork':'Creates a child process with inherited process state and separate private memory state. On success it returns the child PID in the parent and zero in the child; on failure no child is created. Duplicated descriptors refer to the same open file descriptions. Copy-on-write is a Linux implementation strategy, not a POSIX requirement; explicitly shared mappings retain their shared contract. Only the calling thread survives in the child of a multithreaded process.',
 'V-T23-zombie-process':'A terminated child whose waitable termination information has not been collected. Most execution resources have been released, but kernel bookkeeping including identity and status remains. It no longer runs and cannot be removed by sending it SIGKILL; the appropriate parent or reaper must collect its status. It is inaccurate to say that it occupies no memory.',
 'V-T23-pipe':'A channel conventionally used as a unidirectional byte stream with read and write descriptors and finite buffering. A read can be short and does not preserve write boundaries; end of file occurs only after buffered data is drained and every write-end reference is closed. PIPE_BUF bounds atomic writes relative to other writers, not total capacity. Blocking and nonblocking calls have different progress/error contracts.',
 'V-T23-redirection':'Arranging for a descriptor, commonly standard input/output/error, to refer to a selected open file description. Shell redirections can use open and dup2 before exec; dup2 atomically replaces its destination descriptor. Closing that descriptor first introduces a reuse race if another context can allocate descriptors. Redirection order matters and does not by itself serialize application writes.',
 'V-T23-shared-memory':'Storage mapped for shared access by multiple processes, for example a POSIX shared memory object mapped with MAP_SHARED. Mapping the same storage does not provide a synchronization protocol, identical virtual addresses or valid cross-process raw pointers. Initialization, publication, cleanup and process-shared synchronization require explicit contracts. It can avoid copying, but is not universally the fastest IPC.',
 'V-T23-file-locking':'Locks on a file or byte range used by cooperating participants to coordinate access. Ordinary advisory locks do not prevent a participant from ignoring them. Traditional process-associated fcntl record locks and flock locks have different inheritance and release rules; closing any descriptor for a file can release that process\'s traditional record locks. Interaction of lock families depends on platform/filesystem, so noninteraction is not a universal guarantee.',
 'V-T25-semaphore':'A synchronization count with atomic decrement/admission and increment operations. A zero count can block a wait; nonblocking and timed alternatives have explicit failure behavior. There is no mutex-style ownership requirement tying a post to the thread that waited. POSIX sem_post is async-signal-safe and can fail on count overflow; this says nothing about an arbitrary hardware interrupt handler or an RTOS API.',
 'V-T25-read-write-lock':'A synchronization object allowing concurrent readers or an exclusive writer. Reader count has an implementation limit; writer admission, fairness and priority policy must be specified. Extra concurrency can help suitable workloads, but neither read frequency nor section length alone proves a speedup over a mutex. Initialization, lifetime and matching unlock obligations remain.',
 'V-T25-priority-inversion':'A higher-priority task is delayed by a resource owner of lower priority; intervening runnable tasks can extend that delay. Priority inheritance raises an owner according to blocked waiters, while a priority-protection protocol uses configured ceilings. A finite blocking bound still requires correct protocol configuration, bounded critical sections and a stated scheduling/resource model; choosing an attribute alone is not a deadline proof.',
 'V-T25-deadlock':'A set of participants cannot progress because each waits for a resource or event whose enabling action depends on another blocked participant. For ordinary resource-lock cycles, enforcing one strict lock acquisition order removes circular wait. This does not prevent every deadlock involving condition predicates, joins, callbacks or external events.',
 'V-T25-starvation':'Indefinite postponement of a participant while other participants continue to progress. Lock admission and scheduling policies can permit it. Fairness must be defined precisely: eventual-service fairness does not supply a finite time or bypass bound. A bounded-waiting claim needs an explicit bound and scheduling assumptions.',
 'V-T44-core-dump':'A recorded process image usable with matching executable/debug information to examine captured registers and memory. It may be produced on signal termination or captured from a running process; selected mappings can be omitted. Permissions, limits and collection policy affect availability. A captured state is evidence at a point in an execution, not a complete causal history.',
 'V-core-abnormal-termination':'Termination outside normal program completion, for example the terminating action of an unhandled POSIX signal or abort. Distinguish the language-level operation from the execution environment\'s reported signal and available fault context. Invalid C access has no portable guarantee of a particular signal, fault address or instruction report.',
 'V-T25-bounded-queue':'A queue with an explicit maximum number of admitted elements and a specified full/empty policy such as rejection, blocking or dropping. Capacity bounds admitted elements, not automatically total allocated memory, retained nodes or latency. Memory and waiting bounds additionally depend on ownership, reclamation, service and scheduling. Backpressure must be propagated to producers under the declared policy.',
 'V-T25-livelock':'Repeated active transitions that fail to complete the required operation, such as two participants continually abandoning and retrying a conflicting acquisition. Under the modern system-wide progress definition, a correctly proved lock-free algorithm excludes system-wide livelock while allowing an individual operation to starve. A loop that merely uses CAS has no such guarantee; scheduler, allocator and reclamation assumptions matter. The1996 paper used lock-free more loosely for absence of locks.',
 'V-core-working-unit':'A dependency-linked increment of a project with concrete steps, checks, recall and, where applicable, an executable rejection gate. Earlier editions assigned typical hours to units and projects; those records remain descriptive historical accounting. They are not a maximum learning duration or a limit on required depth in the comprehensive extension.',
 'V-T25-thread-pool':'A reusable group of worker threads that execute submitted tasks. Fixed worker count and bounded admission are explicit choices in this curriculum, not universal properties of all pools. Contracts must specify queued and running ownership, overload behavior, cancellation, completion context and shutdown. Reuse can avoid repeated thread creation but does not alone bound memory or completion latency.',
 'V-T44-cause':'A supported mechanism or violated precondition/invariant explaining a failure under stated conditions. Establish it through a relevant execution history, competing explanations and discriminating evidence. The earliest observed invalid transition may narrow the cause without proving it is the unique or earliest causal event in an unobserved history.',
 'V-T44-first-divergence':'The earliest supported mismatch between an observed state and a justified reference contract or prediction at the available observation points. Instrumentation gaps can hide an earlier mismatch; an incorrect prediction can create an apparent divergence. Locating it narrows investigation but does not by itself prove the complete cause.'}

def dump(path,data): path.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

changes=[]
for path in (ROOT/'curriculum/vocabulary').glob('*.json'):
    document=json.loads(path.read_text(encoding='utf-8')); touched=False
    for entry in document.get('entries',[]):
        ident=entry['id']
        if ident in DEFINITIONS and entry['definition']!=DEFINITIONS[ident]:
            entry['definition']=DEFINITIONS[ident]; touched=True
        if ident=='V-T25-livelock':
            entry['misconception']='Using CAS in a retry loop proves lock-free progress (the algorithm and its dependencies need a progress argument).'
        if ident in LOCATORS:
            resource,locator=LOCATORS[ident]
            for authority in entry['authority']:
                if authority.get('inspection')=='pending':
                    changes.append({'entry':ident,'file':path.relative_to(ROOT).as_posix(),
                        'prior_authority':dict(authority),'replacement_resource':resource,'replacement_locator':locator})
                    authority.update(resource_id=resource,locator=locator,inspection='registry'); touched=True
    if touched: dump(path,document)

def resource(ident,title,authors,edition,year,kind,url,scope,limits):
    return {'id':ident,'title':title,'authors':authors,'edition':edition,'year':year,'kind':kind,
        'url':url,'access':'Bounded actual passage inspection','domains':['T23','T25','T44'],
        'role':'oracle','reason':'Resolve unsupported glossary locators and correct contracts without broadening historical inspection claims.',
        'prerequisites':['Language, library and operating-system contract distinctions'],
        'locators':list(dict.fromkeys(loc for rid,loc in LOCATORS.values() if rid==ident)),
        'assignable':True,'inspection':{'date':'2026-10-17','method':'Actual bounded source text read',
            'scope':scope,'limits':limits,'evidence_urls':[url] if url else [],'local_document_id':None,'local_sha256':None}}
book=resource(BOOK,'The Linux Programming Interface, additional bounded process and IPC passages',['Michael Kerrisk'],'First edition',2010,'book','https://man7.org/tlpi/',
    'Only the listed sections/printed pages; PDF page number equals printed page plus44.',
    'Historical2010 implementation details are not current Linux guarantees. No full-book inspection. Extracted text remains private; code is not copied into teaching targets.')
book['inspection'].update(local_document_id='f68a4c3b-28ea-50fe-a6ab-9e507814b4b4',local_sha256='99df6ce877588872fa15b63a490b9857b00d2c7a2d5ae3ceb75b9a05ff324848')
man=resource(MAN,'Linux man-pages process and semaphore contracts',['Linux man-pages contributors'],'6.19',2026,'documentation','https://man7.org/linux/man-pages/',
    'Named function DESCRIPTION/NOTES/ERRORS/HISTORY only. Individual URLs retained in source notes.',
    'Linux interfaces are not ISO C guarantees; no real-time latency or target execution established.')
posix=resource(POSIX,'POSIX Programmer\'s Manual synchronization contracts',['IEEE','The Open Group'],'POSIX.1-2017, Issue7 2018 authorized reproduction',2018,'standard','https://man7.org/linux/man-pages/man3/pthread_rwlock_rdlock.3p.html',
    'Read-lock and priority-protocol function contracts as reproduced by man-pages.',
    'Not POSIX.1-2024. The official2024 rdlock URL returned403; no claim to inspect it or verify a Linux implementation.')
course=resource(COURSE,'C Mastery operational evidence and component contracts',['C Mastery curriculum authors'],'Existing workflow plus2026-10-17 comprehensive scope clarification',2026,'source','https://github.com/FredericCJ/c_mastery',
    'Named workflow, assessment and PR-P5-02 contract sections actually read; original operational definitions.',
    'Internal course conventions, not independent external authority or a claim that TLPI/CSAPP defines these terms.')
course['inspection']['input_sha256']={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ('curriculum/v4/workflow.md','curriculum/guides/assessment.md','curriculum/guides/comprehensive_v5.md','curriculum/projects/PR-P5-02.md')}
queue=resource(QUEUE,'Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue Algorithms',['Maged M. Michael','Michael L. Scott'],'PODC1996 paper',1996,'paper','https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf',
    'Introduction, PDF pp.1–2, progress definitions and bounded-live-queue/unbounded-retention counterexample.',
    'Historical lock-free terminology differs from the modern system-wide-progress usage. The paper is a counterexample/comparison, not an ISO C17 memory-order or bounded-latency specification.')
dump(FOLDER/'systems_vocabulary_resources.json',{'resources':[book,man,posix,course,queue]})
path=FOLDER/'systems_vocabulary_corrections.json'
if changes:
    old=json.loads(path.read_text()).get('changed_entries',[]) if path.exists() else []
    dump(path,{'schema_version':1,'changed_entries':old+changes,'limits':'Only specifically inspected locators migrated; unresolved sources remain pending.'})
print({'updated_authorities':len(changes)})
