"""Retain contextual terminology in canonical module/project authoring outputs.

This authoring helper changes prose only. It never edits fenced metadata,
exercise identifiers, laboratory contracts, evidence records or executable code.
"""
import re

DOMAINS = {'T65', 'T66', 'T67', 'T68', 'T73', 'T74', 'T75', 'T76'}
TEACHING = {
    'T65.01': 'A result value carries either its success payload or its failure information, selected by the validated tag. Reading the success member after receiving a failure is not error handling; it violates the representation contract even if the storage happens to contain a plausible number.',
    'T65.02': 'Error propagation must preserve the failing stage and its offset while returning control to the caller. In this parser, propagating the second field\'s failure cannot publish the first field as a successful complete configuration; ownership cleanup and publication remain separate obligations.',
    'T65.04': 'Short-circuit composition invokes the next callback only for the admitted success alternative. Prove both the returned result and the invocation count: eager evaluation of a callback on an error may preserve the final number while introducing an otherwise absent effect.',
    'T65.05': 'Error accumulation evaluates independent predicates in the declared order and reports total failures separately from stored diagnostics. Capacity zero or truncation does not turn an invalid configuration into a valid one, and dependent predicates still require their own admission order.',
    'T65.06': 'A nonlocal jump transfers control to a saved environment that is still valid; it does not invoke destructors or release the resources skipped on that path. The explicit cleanup stack therefore owns each registered action until the boundary consumes it, including when registration itself fails.',
    'T65.07': 'Serial-number ambiguity occurs at the exact half-range distance: in the 16-bit model, 32768 and 0 have no admitted ordering. This intentional refusal differs from arithmetic overflow, and avoiding that one pair still does not make unrestricted modular comparison a transitive sorting order.',
    'T65.08': 'Contract observation records a failed predicate while the independently bounded teaching effect continues. Contract enforcement instead rejects that transition while retaining the specified reporting side effect; neither mode can make undefined C operations safe merely by logging a warning.',
    'T66.03': 'A consuming operation transfers or ends the caller\'s ownership and leaves the source unusable for another consumption. Check the emptied source and the destination\'s release obligation separately; ordinary C structure assignment can still create an unauthorized duplicate of the same owning representation.',
    'T66.05': 'Interior mutability here means mutation through a shared interface mediated by an exclusive live guard. The guard protocol, not a const-qualified pointer or descriptive type name, enforces admitted access; its non-atomic bookkeeping still requires external serialization.',
    'T66.06': 'The implicit weak reference retains the control block while any strong reference exists. The last strong release destroys the value and then drops that implicit retention; an explicit weak reference may keep the control block alive afterward, but cannot resurrect the destroyed value.',
    'T66.07': 'Copy-on-write detach first acquires and copies private backing, then changes the selected owner. Inject failure before publication and account for the staged buffer: both logical values must remain unchanged, while a unique owner needs no detach allocation.',
    'T66.09': 'Allocation failure injection selects a particular acquisition attempt so every construction prefix can be tested for complete rollback. Allocation poisoning writes a diagnostic pattern during the instrumented backing store\'s lifetime transitions; recognizing that pattern can expose selected misuse, but does not authorize access through an expired borrow or protect against arbitrary aliases.',
    'T73.01': 'A task nursery owns its admitted children and retains their contexts until every join completes. Each cooperative checkpoint observes the cancellation flag and allows normal cleanup; a child that never reaches one can prevent timely joining, so cancellation does not imply a bounded shutdown latency.',
    'T73.02': 'Selection registration must connect the readiness check to waiting without an unprotected notification gap. The hub\'s common mutex and condition variable provide that exclusion boundary: recheck readiness in a loop after every wakeup, including a spurious one.',
    'T73.03': 'Operation completion ownership determines who may release the connected state at the terminal callback. Because that callback may free the state, the completing backend must retain everything it needs beforehand and perform no further access through the consumed state afterward; a stop request alone does not transfer this right.',
    'T73.05': 'Restart intensity counts qualifying failures inside the declared sliding time window and escalates when its finite budget is exhausted. Restart dependency order is a separate invariant: stop affected children in reverse order, establish quiescence, then start them forward so no restarted dependent uses an old live dependency.',
    'T73.07': 'The transaction read version is the snapshot timestamp captured at begin. Each admitted read must reject a locked cell or a version newer than that timestamp; commit validation then checks the retained read dependencies while the write locks protect publication. Speculative external effects are not rolled back by aborting these cells.',
    'T73.08': 'Visible-version retention preserves the current version and the version selected by each live snapshot pin. It does not preserve every historical version: compute the retained set explicitly before reusing a slot, and show why dropping a stale generation cannot remove a current reader\'s retention claim.',
    'T73.10': 'A terminal signal reports the stream\'s final success or error and needs no item demand credit. It must be delivered at most once without overlapping another signal; after it, no item is admitted. Cancellation follows its separate suppression rule and is not evidence of cross-thread callback quiescence.',
    'T75.01': 'The remaining budget is the inherited allowance minus elapsed work and explicitly modeled propagation costs, not a fresh full timeout at every hop. A cancellation acknowledgement must establish the borrower\'s terminal release event before scope storage is reclaimed; the atomic cancellation flag supplies only the request.',
    'T75.02': 'In the required readiness adapter, define flow credit as permission to occupy a bounded amount of receiver capacity and state exactly when that credit returns. The four-slot queue alone does not supply this protocol or bound producer-held offers; credit return also does not acknowledge an application effect.',
    'T75.03': 'A timer-wheel bucket contains candidates sharing a modular index, so due times1 and5 can collide without expiring together. In the required two-level extension, a timer cascade moves a coarse timer into a finer level while preserving its absolute due time and cancellation identity; moving it is not callback execution.',
    'T75.04': 'Burst credit is the stored token allowance available for an immediate admission burst. Lazy refill replenishes it only up to capacity, and spending it does not reserve an active-operation permit; a separate concurrency bound is needed when admitted requests can remain outstanding.',
    'T75.05': 'The retry budget bounds extra attempts even when the draw is zero, while retry jitter spreads their start times under the stated draw distribution. Neither changes the operation\'s effect semantics: the shared deadline, repeat-safety contract and per-dispatch resource charge still apply.',
    'T75.07': 'A hedged request deliberately allows overlapping attempts under one deadline. Its first acceptable result can be available before operation quiescence: every started loser still needs a terminal completion before its response storage and permits can be reclaimed.',
    'T75.08': 'The deduplication window is the finite interval during which a request identity retains its original parameters and result. Once that record expires, the same identity may apply again; persistence across restart and indefinitely preventing duplicate effects need additional contracts beyond this volatile lookup table.',
    'T75.09': 'A committed notification outbox retains the delivery obligation alongside the effect. The public anchor reserves a volatile pending slot before its synthetic commit; the required durable extension must instead define the persistence boundary and recover that obligation after process termination. Channel acceptance alone proves neither consumer delivery nor durable recording.',
}

REPLACEMENTS = {
    'adds descriptor retirement': 'adds shape metadata record retirement',
    'Acquire the replacement descriptor': 'Acquire the replacement shape metadata record',
    'a weak cache owns no descriptor reference': 'a weak cache owns no shape metadata record reference',
    'unique random value': 'unique randomly generated identifier',
    'A quiet race detector': 'A quiet ThreadSanitizer run',
    'Delayed assessment races the last strong drop with weak upgrade': 'Delayed assessment executes the last strong drop concurrently with weak upgrade',
    'cancellation races': 'competing cancellation and completion transitions',
    'owner/thief races': 'competing owner and thief operations',
    'an internal validated descriptor': 'an internal validated range record',
    'a validated in-range descriptor, but no unchecked descriptor': 'a validated in-range record, but no unchecked range record',
    'Move the descriptor across an FFI': 'Move the validated range record across a foreign-function interface',
    'Descriptor count2, offsets areoffsetof(struct mp_record,code/value), independent of wire positions0/2.': 'The field schema table has two records; native offsets are offsetof(struct mp_record,code/value), independent of wire positions0/2.',
    'Generate descriptor count from the actual array.': 'derive the field schema table length from the actual array.',
    'Add a parsed format DSL with typed placeholder descriptors and partial streaming sinks.': 'Add a parsed format domain-specific language with typed placeholder records specifying each argument category and format operation. Add partial streaming sinks with explicit capacity and progress.',
    'Add a parsed format DSL with typed placeholder records specifying each argument category and format operation and partial streaming sinks.': 'Add a parsed format domain-specific language with typed placeholder records specifying each argument category and format operation. Add partial streaming sinks with explicit capacity and progress.',
    'A smaller crashing input that violates preconditions is not necessarily a valid law counterexample.': 'A smaller input that violates the callback preconditions is not a valid counterexample to its stated law, even if it triggers a diagnostic or abnormal termination.',
    'Simulate crashes during artifact publication and hash collisions': 'Inject process termination before and after artifact publication, and separately simulate hash collisions',
    'short-I/O, crash and recovery evidence': 'short-I/O, process-termination and recovery evidence under a declared persistence model',
    'short writes and crash schedules': 'short writes and process-stop schedules at write, flush and publication boundaries',
    'the crash point between query-state and cursor persistence': 'process termination between query-state and cursor persistence',
    'Enumerate crash/retry points': 'Enumerate process-termination and retry points',
    'Distinguish crash fault tolerance from Byzantine tolerance': 'Distinguish fail-stop tolerance, where a replica stops sending, from Byzantine tolerance, where it may send arbitrary state',
    'disconnection and crash/restart': 'disconnection and fail-stop termination followed by restart from declared durable state',
    'explicit crash points around record/effect publication': 'explicit process-termination points around record/effect publication',
    'The SC arguments': 'The arguments under sequential consistency',
    'SC correctness': 'correctness under sequential consistency',
    'what SC proves': 'what sequential consistency proves',
    'Weaken SC to acquire/release/fences': 'Replace sequentially consistent operations with acquire/release operations and fences',
}
STATUS = {
    'T65': ('RANGE', 'AGAIN', 'INVALID', 'THROWN', 'ENFORCE', 'OBSERVE', 'OFF', 'CONTRACT'),
    'T66': ('DEAD', 'BUSY', 'STALE', 'INVALID', 'NOMEM', 'ALWAYS', 'ON_SUCCESS', 'ON_ERROR'),
    'T67': ('NUMBER', 'PAIR', 'EMPTY'),
    'T73': ('CLOSED', 'BUSY', 'FULL', 'EMPTY', 'CONFLICT', 'NEW', 'STARTED', 'DONE'),
    'T75': ('FULL', 'OPEN', 'CLOSED', 'BUSY', 'STALE', 'INVALID', 'CANCELLED', 'BLOCK', 'REJECT', 'DROP', 'OVERWRITE', 'DEGRADE', 'DEGRADED'),
}
EXPANSIONS = {
    'T73': {'SC': 'sequentially consistent'},
    'T76': {'HLC': 'hybrid logical clock', 'HAMT': 'hash array mapped trie',
            'CQRS': 'command–query responsibility segregation', 'OT': 'operational transformation'},
}


def polish_document(path, text):
    """Preserve canonical metadata and code; edit only the selected prose bodies."""
    match = re.fullmatch(r'curriculum/(modules/(T\d\d)|projects/PR-(T\d\d)-\d+)\.md', str(path).replace('\\', '/'))
    if not match:
        return text
    domain = match.group(2) or match.group(3)
    if domain not in DOMAINS:
        return text
    fenced = re.match(r'(\A```json\n.*?\n```\n\n)(.*)\Z', text, re.S)
    if not fenced:
        raise ValueError('Expected canonical fenced metadata: ' + str(path))
    header, body = fenced.groups()
    parts = re.split(r'(```.*?```|`[^`\n]*`)', body, flags=re.S)
    for index in range(0, len(parts), 2):
        prose = parts[index]
        for old, new in REPLACEMENTS.items():
            prose = prose.replace(old, new)
        for old, new in EXPANSIONS.get(domain, {}).items():
            prose = re.sub(r'(?<![\w-])'+re.escape(old)+r'(?![\w-])', new, prose)
        prose = prose.replace(': command–query responsibility segregation', ': Command–query responsibility segregation')
        prose = prose.replace('— command–query responsibility segregation', '— Command–query responsibility segregation')
        prose = prose.replace('. hybrid logical clock local', '. For a hybrid logical clock, local')
        for token in STATUS.get(domain, ()):
            prose = re.sub(r'(?<![\w-])'+re.escape(token)+r'(?![\w-])', '`'+token+'`', prose)
        parts[index] = prose
    body = ''.join(parts)
    if '/modules/' in str(path).replace('\\', '/'):
        for topic, paragraph in TEACHING.items():
            if topic.startswith(domain+'.') and paragraph not in body:
                pattern = r'(^### '+re.escape(topic)+r'[^\n]*\n\n)'
                body, count = re.subn(pattern, lambda m: m[1]+paragraph+'\n\n', body, count=1, flags=re.M)
                if count != 1:
                    raise ValueError('Missing teaching section ' + topic)
    return header + body
