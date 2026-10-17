"""Mechanism-specific graduated hints; assessor answers live separately."""
ROWS = r'''
T23.01|Name which state belongs to a thread, a process and an open object.|Compare the same virtual address with the backing object's sharing policy; wait/join before inspecting the result.
T23.02|List every pipe end and who owns it before and after fork.|Use close-on-exec to make a successful transition observable without assigning special meaning to the helper's exit value.
T23.03|Draw descriptor-table entries separately from their referenced open-file descriptions.|Predict which operation changes the shared offset and which flag belongs only to a descriptor.
T23.04|Keep a cursor for bytes already committed, separately from bytes buffered.|Replay the schedule one return value at a time; a retry must start at the first uncommitted byte.
T23.05|Track the object reached by each open handle before changing any name.|Separate name resolution from operations on the already acquired object; identify where a replacement can occur.
T23.06|Write down the units and reference object of every metadata field.|Compare link metadata with target metadata, then apply the actual creation mask and filesystem rules.
T23.07|Determine the representable domain before converting or adding offsets.|Use a subtraction-based range check in the source type, then justify conversion to the selected off_t.
T23.08|Separate address-space reservation, backing and observed residency.|Ask which observation proves a page exists now and which says nothing about future faults or reclamation.
T23.09|Keep logical file extent distinct from rounded mapping extent.|A successful mapping owns a separate lifetime; closing its descriptor and truncating its backing file are different transitions.
T23.10|Choose one initializer and one explicit publication event.|Offsets solve address relocation, while process-shared synchronization and owner-death policy solve different problems.
T23.11|An EOF needs all writer references gone, including inherited duplicates.|Draw the pipeline before launching children and mark every end each process must close.
T23.12|Give each buffer and persistence boundary its own state.|After rename, ask which directory entry must survive a crash; a successful write is an earlier boundary.
T23.13|Choose a clock according to the event being measured, not its name.|Recompute remaining time from one absolute deadline; keep wall time, CPU consumption and suspended time distinct.
T23.14|Find the authority that promises each behavior.|An installed prototype and one successful run do not establish portability to every POSIX or C implementation.
T24.01|Separate disposition, blocked mask and pending state.|Repeated standard signals may collapse into one pending indication; a counted application queue has a different contract.
T24.02|Place the signal between checking the predicate and starting the wait.|The repair must make the mask transition and wait atomic with respect to the relevant wakeup, not just shorten the window.
T24.03|Compare the lifetime of bytes and endpoints after the sender disappears.|A shared memory region does not itself delimit a message or publish its initialized contents.
T24.04|Give each address attempt its own socket ownership state.|On nonblocking completion, readiness and the socket's pending error are distinct observations.
T24.05|Carry parser state across every possible split, including the length byte.|Compare incremental decoding with a complete-buffer decoder; end-of-stream inside a frame is not an empty frame.
T24.06|Inspect both returned length and output flags.|A zero-length datagram is a message; truncation loses message bytes and cannot be repaired by reading the remainder later.
T24.07|Distinguish ready, progress, would-block and terminal states.|Readiness can become stale before the operation; preserve the cursor and resume on the next scheduling opportunity.
T24.08|Ask whether an edge remains to be generated after an incomplete drain.|If a quota stops before EAGAIN, preserve runnable work explicitly rather than waiting for an edge already consumed.
T24.09|Determine the exact counter width and its accumulation semantics.|Multiple expirations can be reported by one read; a short or interrupted operation is not a new timer count.
T24.10|Select the VMIN/VTIME case before predicting blocking.|Use an owned PTY to test software framing and restoration; it cannot supply UART electrical timing evidence.
T24.11|State who owns a rejected frame and what progress shutdown promises.|Close admission first, then drain or reject existing work under a declared deadline and terminal result.
T24.12|List submission, cancellation and target-completion events separately.|A cancel result does not necessarily return buffer ownership; inspect the target operation's final completion contract.
T25.01|Record the successful creation prefix before touching any thread handle.|Failed pthread_create returns an error directly and does not create a joinable handle; keep all successful workers' argument storage live through join.
T25.02|Assign every ordinary object a writer and a permitted reader interval.|Disjoint result cells need no shared update lock, but publication and worker completion still require synchronization.
T25.03|Locate the full invariant, not just each individually valid field.|The lock must cover the transition that preserves the invariant; split locks can expose an impossible combined state.
T25.04|Write the predicate beside the mutex that protects it.|A wakeup is permission to recheck, not ownership of an item; include close and cancellation in the state machine.
T25.05|Identify whether the primitive counts, excludes, rendezvous or initializes.|Compare its state after a participant fails or a phase advances; similar waiting APIs can enforce different invariants.
T25.06|Draw reads-from before adding synchronizes-with.|An acquire operation that did not observe the relevant release cannot supply that publication edge.
T25.07|Distinguish the atomic object's lifetime from the value stored in it.|Atomic read-modify-write returns a prior value; copying bytes is not the same operation or a universal initialization contract.
T25.08|Describe allowed executions before measuring frequency.|A failure to observe an allowed weak outcome is not evidence that the memory model forbids it.
T25.09|Treat success as a read-modify-write and failure as a load.|The expected argument changes on failure; choose its failure ordering from the information the next iteration consumes.
T25.10|Name whose progress is missing and what the scheduler is allowed to do.|A lock-order proof removes a cycle but does not establish fairness, priority inheritance or bounded completion time.
T25.11|Inspect actual addresses and generated operations before timing.|Separate shared cache-line traffic from data races; keep useful work and correctness contracts equivalent across variants.
T25.12|Choose one logical defect and one data-race defect as different controls.|A schedule controller and a race detector answer different questions; neither a clean run nor coverage is a universal proof.
T26.01|Start with the sequential specification and real-time precedence constraints.|Overlapping operations may be ordered differently; nonoverlapping completed operations cannot be reversed arbitrarily.
T26.02|Write the progress claim with explicit participant quantifiers.|A bounded failure return may complete one API operation while failing the caller's requested service guarantee.
T26.03|Draw both publication and slot-reuse ownership arrows.|The reverse release/acquire edge prevents overwrite while the consumer still reads; modular distance also needs an occupancy bound.
T26.04|Separate reservation, linking, publication and helping.|A producer stalled after reservation exposes an algorithmic state that stronger atomic ordering alone cannot remove.
T26.05|Label object incarnations as well as addresses.|A tag can detect identity change without preserving lifetime; finite generation wrap needs an explicit policy.
T26.06|Pause removal between the first pointer load and hazard publication.|The protecting reader must validate before dereference; a collector must distinguish retirement incarnations before recycling.
T26.07|Track active intervals and retirement epochs, not elapsed wall time.|A stalled participant may preserve safety by preventing reclamation while exhausting the bounded pool.
T26.08|Separate new-root visibility from the end of old-reader rights.|A callback retaining a pointer after its read section needs another ownership mechanism or a longer legal lifetime.
T26.09|Write the kernel release/configuration and callback context before choosing an API.|Use only the original guest target; compiled host models cannot demonstrate user-copy, scheduling or module-unload behavior in the kernel.
T26.10|Choose the semantic model for each edge before naming a barrier.|Stop racing admission before synchronous drain; ordering an access cannot revoke a timer, callback or device reference.
T27.01|Resolve the exact part, package and board separately.|A core manual supplies architecture behavior; a reference manual, datasheet and schematic answer different remaining questions.
T27.02|Decode reset addresses before trusting an application's symbol names.|Boot aliases, ROM handoff and application vectors can be different locations with separate ownership transitions.
T27.03|Check image length before reading either initial vector word.|Validate stack region/alignment and reset instruction-state encoding independently; one valid field cannot repair the other.
T27.04|Draw a half-open stack interval beside every other live region.|Alignment at a public interface is distinct from proving enough space for nesting, exception frames and worst call paths.
T27.05|Represent each copy as a source interval and a destination interval.|Handle empty ranges explicitly and preserve canaries; link-time load and run addresses are different coordinates.
T27.06|List the dependencies of the first diagnostic instruction.|Clock readiness must be bounded, and the fallback cannot use a peripheral whose prerequisites failed.
T27.07|Give each hook a stage and prerequisites.|A function pointer being present in a table does not make its runtime dependencies initialized or its failure recoverable.
T27.08|Annotate each startup responsibility before replacing vendor code.|Compare your resulting image and link closure with the responsibility map; source resemblance is not behavioral equivalence.
T27.09|Put the validity marker after all bytes it authenticates in the declared model.|Enumerate reset cuts and record atomic-write assumptions; retained memory does not inherently imply a coherent record.
T27.10|Choose a marker channel that is valid at the earliest failing stage.|A missing later marker localizes only after earlier instrumentation and its dependencies are verified.
T28.01|Give each register its access-width and side-effect contract.|A read used for read-modify-write may itself clear status; RAM behavior cannot validate such an operation.
T28.02|Compare the intended bus transaction with generated load/store width.|Volatile constrains certain compiler accesses but does not supply atomicity, peripheral completion or cache maintenance.
T28.03|Make clock/reset readiness part of the adapter's state.|The first access must occur after the selected device's documented prerequisites, not after an arbitrary delay.
T28.04|Separate logical pin configuration from external wiring.|A BSRR operation avoids an output-latch read-modify-write but cannot establish pull strength or safe external drive.
T28.05|Derive the divider and rounding error before choosing register fields.|TXE reports a data-register condition; complete transmission and receiver error clearing have different contracts.
T28.06|Count the timer's inclusive endpoints before computing frequency.|The preload/update boundary determines when a new period takes effect; a correct arithmetic divider alone does not prove a glitch-free change.
T28.07|Place chip-select edges on the transaction timeline.|The last data write, last received word and bus-not-busy observation are separate milestones.
T28.08|Separate the seven-bit address from the shifted wire byte.|For one-byte receive, the ACK/ADDR/STOP order is controller-specific; do not import a longer-read sequence unchanged.
T28.09|Count acquisition time separately from conversion cycles.|A settled digital code still depends on analog source impedance, reference and sampling assumptions not supplied by the host model.
T28.10|Validate buffer reachability and ownership before enabling DMA.|Cancellation requests a transition; only confirmed quiescence returns the buffer for CPU reuse.
T28.11|Keep each layer's state and error translation visible.|Changing a register adapter should preserve service semantics while exposing target-specific limitations, not erase them.
T28.12|List controller, PHY/transceiver, pins and clock prerequisites independently.|A host diagnostic datagram exercises an application/ownership contract; CAN arbitration and USB enumeration need their own required routes.
T29.01|Decode EXC_RETURN separately from the stacked register values.|Frame kind, alignment and selected stack determine valid offsets; never assume every exception has the basic eight-word layout.
T29.02|Keep enable, pending, active and masking state separate.|Numerically lower priority values and priority grouping affect selection differently from whether an interrupt is pending.
T29.03|Bound the worst handler path and every called function.|Defer work only after defining who owns the captured record and what happens when the bounded queue is full.
T29.04|List the participants each exclusion mechanism actually stops.|Restoring the saved mask differs from blindly enabling interrupts, and masking a CPU does not stop DMA.
T29.05|Identify which context owns each ordinary payload access.|An index publication must also establish a legal compiler/architecture contract; hosted atomics and interrupt semantics are not automatically identical.
T29.06|State whether one slot is reserved to distinguish full and empty.|Trace ownership and drop accounting through wrap; a full queue must not overwrite a record still owned by the consumer.
T29.07|Represent completion and confirmed stop as competing terminal transitions.|A generation check rejects stale callbacks only while its identity cannot repeat; release ownership once, after quiescence.
T29.08|Classify compiler order, CPU order, cache visibility and device completion separately.|A barrier cannot flush arbitrary dirty cache lines or prove a DMA engine has stopped touching a buffer.
T29.09|Bound the interval between event, interrupt entry and counter sampling.|A coherent split-counter read is a snapshot algorithm; its retry bound and rollover assumptions need their own proof.
T29.10|Choose a synchronization point for each competing transition.|A sleep does not prove another participant reached that point; retain the first violated state and separate model from physical latency evidence.
'''
HINTS = {topic: [first, second] for topic, first, second in
         (row.split('|') for row in ROWS.strip().splitlines())}
