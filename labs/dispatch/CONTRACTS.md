# Dispatch and dynamic modules: contract version1

C17 hosted Linux is the implementation baseline. Headers fix exact signatures. All pointed-to objects, operation tables, callback contexts and loaded-code handles remain live for their stated use. Control/input/output objects are disjoint, except the explicit move/transfer operations. Callbacks use exactly compatible function types. Unless an explicit extension below declares a partial-progress or resource-result protocol, callbacks return a DSStatus and their output is meaningful only on success; wrappers stage outputs so a callback's failed partial output is not published. Callbacks may not reenter/mutate their owner, close a module, escape through longjmp or retain borrowed storage. No concurrency is admitted in this family; adding it requires a new protocol. Void initialization/drop/close operations require a valid live object. Required null control/output pointers return DS_INVALID in checked status-returning functions. Caller-forged structs and arbitrary invalid pointers are outside this contract; C cannot validate them generally.

No library supplies the assessed dispatch, closure, list, capability or reload mechanism. `dod.small_buffer` is a real dependency for erased capture storage; include and link the learner's accepted `labs/dod` implementation, preserving its byte-only contract. Host harnesses may use allocation/stdio; dynamic loading is a declared Linux library dependency and has no freestanding analogue in this exercise. Other mechanisms require separate compiler-helper and link audits before a strict freestanding claim.

## Interface views and dictionary calls

DSView borrows an object plus a DSOps pointer. DSOps.measure must interpret that concrete object type; a nonnull table cannot establish that association. ds_measure invokes once and preserves output on failure. DSBox embeds the table pointer before its concrete int value; ds_embedded adapts the address of that int to the same view contract. No vtable ABI is promised by ISO C. A const view does not freeze unrelated mutable aliases.

ds_fold admits0 through 16 ints and a nonnull DSCombine callback, folds left from initial, and publishes only after all callbacks succeed. A null input is valid only for length0. Callback effects are retained if a later callback fails; output preservation is not effect rollback. ds_sum is a direct specialized checked-add alternative under the same prefix-overflow policy. Values that overflow an intermediate prefix reject even if the final mathematical sum would fit. Operand order matters for a noncommutative dictionary.

## Explicit closure modes

DSClosure has an int capture, active bit and mode shared/mutable/once. Init admits exactly the three named modes. Call calculates checked capture+argument. Shared does not change capture; mutable stores the successful result. Once becomes inactive on the first admitted call attempt, including arithmetic rejection; invalid control/output arguments do not consume it. Inactive calls return DS_EMPTY. Output stays unchanged on rejection. These are runtime protocols, not a C borrow checker, Fn trait implementation or automatic move-only type. A copied struct can bypass active tracking; demonstrate that limitation without pretending the wrapper enforces linearity globally.

## Owning erased byte captures

DSErased owns a DSmall byte capture and an optional externally owned resource described by destroy/resource. The call thunk receives borrowed counted capture bytes, argument and a temporary output. Assign requires an inactive initialized owner; successful byte storage construction transfers the external resource obligation. Failure transfers nothing and leaves the owner empty. Capture representation and thunk must agree; no arbitrary struct pointer may be cast into the character buffer. A thunk may decode into an actual typed local. The provided anchor uses one unsigned byte plus an int argument in[-100,100]; other encodings need explicit contracts.

Move requires a distinct inactive initialized destination and active source. It transfers storage/call/resource ownership, then leaves source inactive with empty storage. Drop is idempotent, invokes the no-fail destroy callback exactly once when active and clears storage. No implicit copy operation is defined. External resources and captured references may have different lifetimes; a destroy callback does not extend a borrowed referent. Reentrancy is excluded. The callable is not safe to byte-copy merely because its capture fits inline.

## Counted int32 captures and typed alternative

`capture.h` extends the original erased anchor with 1 through 8 signed 32-bit values. This hosted codec requires CHAR_BIT=8 and 32-bit int, checked by translation-time assertions. Encode each value in four little-endian octets, interpreting negative values modulo 2^32. Decode each into an actual int32_t local: words above INT32_MAX become `-1 - (int32_t)(UINT32_MAX-word)`. Never cast capture storage to int32_t*. Call adds all decoded values and the argument in int64_t, checks the final sum against INT32_MIN/MAX and publishes once; intermediate int32 overflow is irrelevant to this specified final-sum policy. Rejection leaves output unchanged. Length0 or above8 rejects; unsupported over-aligned, self-referential or arbitrary C object captures have no implicit codec. The safe self-reference counterexample copies a live pointer whose referent remains the original object's member, not the copy's member.

`ds_capture_i32` borrows its input only during construction. Successful construction owns encoded copies and transfers the optional resource destructor obligation; failure leaves the resource with the caller. Captures of1 through 4 values use DSmall's inline16 bytes;5 through 8 values spill. `ds_borrowed_i32` performs the same final-sum calculation without copying, owning or retaining input. Its caller keeps the complete input alive and unchanged for the synchronous call. It makes no asynchronous lifetime guarantee.

The explicit `ds_counted_bytes` adapter imports **the actual T66 OwAllocator**, not another allocator with similar counters. A nonnull, zero-initialized/live OwAllocator stays at one address through all allocations/releases. The adapter admits1 through 64 bytes, requests ceil(n/sizeof(int32_t)) elements from its actual V4Allocator and returns a character pointer to the complete allocated int32_t[16] row object. Byte use is access to that complete array's representation, not a conversion creating arbitrary typed objects. Zero/above64 requests return null before upstream allocation and are outside this adapter's capacity; the general DSmall128-byte limit does not enlarge it. Eight live rows exhaust the actual pool; failed requests transfer no ownership. Release accepts only a currently owned row returned by the adapter. Host tooling and the supplied allocator are explicit dependencies; no target allocator or freestanding claim follows.

`struct ds_typed_capture` is the separately implemented concrete typed alternative: a union holds int32_t local[4] or an actual allocator-provided int32_t array, with count, spill tag and V4Allocator. Init/drop require live nonnull storage; assign requires empty initialized destination and1 through 8 disjoint input values. Missing allocation callbacks reject a spill; allocation failure leaves destination empty. Move requires distinct empty destination and nonempty source, transfers the allocator/representation and clears source; drop releases one spilled allocation and is idempotent. Copying owning structs is outside the ownership contract. Neither alternative automatically demotes a spill on a call. DSmall truncation retains capacity until explicit compact, so a separate threshold oscillation experiment must distinguish retention from compact-every-time churn.

Both construction paths copy O(n) values/bytes and invocation decodes/sums O(n); move copies at most inline capacity or transfers one pointer; drop releases at most one allocation plus the optional destructor. `dispatch.capture_codec` exercises allocation injection, exhaustion, exactly-once release, signed endpoints and unsupported lengths. `dispatch.typed_capture` compares value/result/allocation behavior and reports actual object footprints. The independent oracle compares48 C traces including exact bytes, output preservation, requests, live rows and releases. These bounded measurements do not establish relative elapsed-time performance, another ABI or learner independence.

## Multiple dispatch and cache epochs

DSMethods has a3x3 ordered method table, initially empty at epoch1. Each admitted set, including removal via null method, increments epoch; epoch3 rejects further mutation with DS_FULL rather than wrapping. Cache initially has epoch0. Lookup is keyed by table identity (caller binds one cache to one table), current epoch, left type and right type. Opposite type order is distinct unless symmetry is explicitly registered. Missing methods return DS_UNSUPPORTED. Cached misses must invalidate too. Failed method calls preserve output but may update the cache and callback effects. No global coherence, overload resolution, inheritance fallback or megamorphic policy is implied.

## Intrusive objects and opaque modules

DSItem contains DSLink as its first member and then an int. A DSList is a circular sentinel plus count, maximum4 live items. Items and list stay at stable addresses while linked. A link belongs to at most one list. Push appends; remove unlinks both neighbors, clears owner/links and permits later insertion. List sum performs checked left-to-right addition and preserves output on rejection. Sentinel links are never converted to DSItem. Conversion of a genuine first-member link to its enclosing live DSItem uses the inspected first-member rule; arbitrary-offset container_of is a separate compiler/platform investigation, not an ISO portability claim.

DSModule is opaque in the public header and uses2 internal typed instances. Open borrows a DSService callback/context, publishes a module on success and rejects exhaustion without modifying output. Twice performs two ordered reads and checked addition; it never rolls back external read effects. Close ends the module lease and clears its borrowed service. All aliases become invalid at close; reuse of that address is not an identity check. No allocator is needed for this bounded pool. The public incomplete type hides fields from ordinary clients, not malicious same-address-space memory access.

## Attenuation and revocation model

DSAuthority owns a synthetic counter, rights bits0(read)/1(add) and epoch1 through 3, then retired4. Issue admits only a subset of current rights. Attenuation can only remove rights from the given capability, preserving owner/epoch. Add requires matching live owner/epoch and add rights in both root and capability, then performs checked int addition. Revoke increments epoch, including retirement at4; later issue denies and further revoke returns DS_FULL. Old capabilities reject with DS_STALE. This is an original local cooperative API threat model: attacker chooses requests and receives legitimately issued values, but cannot fabricate pointers/struct fields, mutate the authority or read arbitrary process memory. C data alone is not an unforgeable hardware or language capability. The exercise separately requires this counterexample and a boundary design that can enforce it.

## Plugin ABI and reload

Only original DSOs built from tests/plugin.c may be loaded by this laboratory; paths contain an explicit local build-directory separator. Export `const struct ds_plugin dispatch_plugin` as a real complete object. abi must1, size must exactly sizeof(DSPlugin) for this shared compiler/ABI, and apply/migrate must be nonnull. Bad-version fixtures still export a complete typed object; size checking cannot safely discover an arbitrarily malformed foreign object's actual extent. No function/object pointer cast is required: dlsym resolves the object, whose function-pointer fields have actual declared types. dlerror is cleared/read around lookup. Loader constructors run before API validation, so version checks and RTLD_LOCAL do not sandbox code.

DSLoaded initially empty; open failure leaves it empty. Acquire returns a borrowed API pointer and increments a bounded borrow count0 through 4. Release decrements a matching held borrow; the caller clears every derived callback pointer before release. Close rejects with DS_BUSY while borrows remain; after successful dlclose it clears handle/API. An unloaded function pointer must never be called. Logical invalidation is required even when a loader keeps an image mapped. A failed close retains ownership for explicit recovery.

DSHost has current and one retired module, state and non-wrapping generation0 through 3. Reload requires a live unborrowed candidate, quiescent current, empty retired slot and generation<3. Candidate migrate receives old schema (0 for no current) and old integer state. Only after successful migration does host transfer current to retired, candidate to current, clear candidate and publish state/generation. Failure retains all ownership/state. Collect closes retired separately; failed collection retains it and blocks another reload. The original plugin1 preserves state and plugin2 doubles it, accepting source schema0 through 2 and state0 through 500. apply admits state0 through 1000 and amount0 through 100, then returns their sum. No asynchronous callbacks, thread-local plugin state, external registration or resource pointers survive quiescence in this model. Their addition is a required changed-condition protocol, not covered by the smoke test.

## Assessment and evidence

Every exercise incorporates this contract and modern-cost-contract.md. Public anchors execute bounded mechanisms; two compiled semantic mutants per anchor must fail the named DS_CHECK assertion with exit1. A crash is rejected as insufficient mutant evidence. Written compile-fail probes, alternative representations, all-target costs, independent oracle traces and delayed transfer remain separately assessed. Source-bound author campaigns are not independent review or learner mastery.


## Nominal receivers and partial progress

`dispatch_extensions.h` supplies two distinct named receiver types and two distinct named interface wrapper types. `DS_RECEIVER(p)` accepts exactly listed const/nonconst receiver pointer types through C17 `_Generic`, evaluates its argument once, and selects a typed constructor. It is a closed registration list, not inference of semantic conformance or language extension-method lookup. Incompatible named wrappers, unlisted receivers and incompatible callback signatures require diagnostics in the supplied strict translations; a signature-correct false implementation still compiles. `struct ds_int_receiver` construction borrows the actual int referent; the temporary receiver descriptor need not survive. `struct ds_bytes_receiver` construction borrows the descriptor as well as its byte span, so both must survive. A null byte pointer is legal only for length0. Length above INT_MAX rejects without publishing. Static operation tables outlive every returned view. A view of the second int field of a live parent requires no pointer subtraction or container conversion; const views do not prevent updates through other aliases.

`ds_read_progress` is a separate result protocol, not the ordinary failure-output-unchanged callback contract. A `struct ds_reader` callback receives a staging array with capacity0..16, must write only within capacity, initialize every reported byte and report a count no greater than capacity. The callback is invoked once even at capacity0. DS_OK may report a short prefix; DS_FAILED may report a completed prefix; DS_EMPTY denotes end of stream and requires count0. Return `{status,count}` and publish exactly that prefix for valid results, preserving all output suffix bytes. Invalid arguments reject before invocation; a malformed callback result returns DS_INVALID/count0 without copying, but cannot undo the callback's external effects. The callback is trusted not to overrun the staging array. This wrapper is not a sandbox or proof that an arbitrary callback obeys its contract. Work and storage are O(capacity), with at most16 staged bytes.

## Borrowed closures and recoverable owned results

`struct ds_borrow_closure` borrows1..8 actual int32 values for its entire active lifetime and invokes the existing final-sum checked `ds_borrowed_i32` adapter. The caller keeps the slice live and unchanged during invocation. Once mode consumes on every admitted attempt, including range failure; invalid arguments do not consume. A shared borrowed closure remains active. Neither stores a lifetime token or proves the referent's lifetime; raw C struct copies can duplicate a local consumed flag.

`struct ds_owned_closure` owns exactly one nonnull object/drop pair. Init requires fresh storage or an inactive owner; initializing over a live obligation is prohibited. Call requires an initialized result with empty retry pair and a trusted exactly typed body. Before invoking the body, clear the closure's resource and active flag: the body now uniquely owns the obligation. The body must either dispose it exactly once or return that still-live obligation in `struct ds_owned_result.retry`; it must not retain or duplicate another alias claiming ownership. A nonempty retry always has a nonnull no-fail drop callback. The wrapper returns DS_OK for admission, while `result.status` records the body's success or failure. Thus failure after admission can return an owned retry object even though the original closure remains consumed. The caller explicitly transfers retry into another empty owner and clears the result before reuse. Drop of an active closure disposes once; inactive drop is idempotent. Callback reentry, retaining borrowed arguments and concurrent mutation are excluded. This is runtime bookkeeping plus programmer ownership discipline, not ISO C linear typing. Invocation and drop are O(1) excluding body/destructor work.

## Cloning two owned capture resources

`struct ds_clone_capture` embeds the actual DSErased/DSmall owner, two distinct owned resource pointers and a clone/drop dictionary. It is address-stable because the erased destructor context points back to the enclosing owner; generic byte copy or `ds_erased_move` is forbidden for this type. Init requires fresh/unowned storage, live allocator and callback context. Adopt requires an empty initialized owner, two distinct nonnull resources and an agreed byte schema/thunk. It transfers both resources only after actual capture construction succeeds; failure transfers neither. Clone requires distinct owners, an active source and an empty destination. Clone resource0, then resource1, then construct destination capture through its own allocator. A failing resource callback transfers nothing; successful clone returns a distinct live resource governed by the same dictionary. On failure at resource1, drop the completed resource0. On subsequent spilled capture allocation failure, drop1 then0. Restore destination dictionary, keep it empty and preserve source. Attempts/callback side effects are observable and are not rolled back.

Success leaves two independent resource pairs and independent capture storage. Drop releases resource1 then0 and the actual capture allocation, and is idempotent. The no-fail callbacks must not reenter either owner. Trusted callbacks must satisfy distinctness and failure-transfer rules; arbitrary hostile callbacks are outside this contract. Clone costs O(capture bytes) plus two resource-clone costs and at most one byte allocation; rollback executes at most two destructors. Tests exercise inline8 and spilled20-byte capture with cuts at both resource acquisitions and the actual T66 allocation. The independent oracle checks42 acquisition-prefix/partial-progress traces. Sanitizers and eight semantic mutants test these bounded contracts, not general lifetime safety or asynchronous cancellation.


## Two-entry subtype resolution and module leases

`dispatch_resolution.h` adds a separate4-type graph without changing the original3x3 DSMethods contract:0 is the root,1 and2 directly derive from0, and3 derives from both1 and2. A method at(A,B) applies to(L,R) iff L is a subtype of A and R a subtype of B. A candidate dominates another iff it is at least as specific in both positions and strictly more specific in one. Return DS_UNSUPPORTED if none apply, DS_AMBIGUOUS if multiple incomparable maximal candidates remain, and invoke only the unique maximal method. Equal sums of inheritance distances are not the rule. In particular(1,2) and(2,1) are ambiguous for(3,3), and adding exact(3,3) resolves it. Dispatch never sorts the ordered pair. Failure and ambiguity preserve output; callback effects remain.

DSResolution starts empty at epoch1. Registration/removal increments epoch and rejects unchanged at31; terminal clear removes all registrations and sets32, invalidating even a cache populated at31. Clear is idempotent. Reinitializing the same address is prohibited while any cache still references that incarnation. Registration accepts either a static DSCombine or a DSLoaded module, never both. Module registration acquires an actual DSLoaded borrow and uses that API's apply callback. Replacement acquires the new borrow before dropping the old, so failure preserves the old registration. Explicit removal uses null method/module. Every referenced table and module owner stays live and address-stable through clear; moving a DSLoaded while registered or cached is prohibited.

DSResolutionCache stores exactly two entries and a next-victim bit. Key includes table address, epoch and ordered pair. Hits do not change eviction order; each admitted miss replaces the next slot and flips the bit (FIFO of fills, not least-recent use). Cache positive, unsupported and ambiguous decisions. Hits/misses saturate at UINT_MAX and are observations, not semantic identity. Miss resolution is O(16 squared) with this simple candidate-dominance scan; an entry hit compares at most two keys then invokes once. Uncached resolution repeats the dominance work. No hardware branch/latency equivalence follows from these bounds.

A positive cache entry originating in a plugin owns a distinct actual DSLoaded borrow in addition to the table registration's borrow. Acquire it before evicting the old entry; failure returns DS_FULL without replacing the cache or publishing output, although the miss counter increments. This conservative ordering can reject when releasing a victim first could make room; it preserves failure atomicity. Cache clear first removes each callable pointer and then releases that borrow. Table updates invalidate cache decisions by epoch but do not erase their outstanding ownership obligations. Thus removing a registration can still leave unload blocked by stale entries until cache clear/eviction. Cached misses hold no code pointer or lease, but must still check epoch after a plugin is registered. `ds_loaded_close` returns DS_BUSY until every registration, cache and explicit external borrow has gone.

The actual DSO test uses only the original locally built teaching module. Owner-thread serialization and no callback reentry ensure no invocation is active at clear; this is a synchronous quiescence protocol, not an asynchronous worker cancellation mechanism. Queueing callbacks requires the separate hot-reload extension. No pointer into unloaded code is ever invoked in a negative test. Four compiled semantic controls reject stale epochs, arbitrary ambiguity selection, missing cache leases and stale cached misses. The Python set-relation oracle checks all16 subsets of four registrations against all16 queries, comparing both actual cached and uncached implementations. Additional ABI and latency evidence remain distinct.


## Ordered dictionaries and saturating regrouping

The dictionary history support enumerates every sequence of lengths0..5 over{-2,0,3}, initial1, under checked addition, checked subtraction and rejection of a negative right operand followed by checked addition. The trace records every left/right operand and status, stops exactly at the first failure, and compares both calls and final output with an independent mathematical model. There are1092 complete traces. INT_MAX,1,-1 from initial0 rejects on the second operation even though the mathematical final sum would fit: prefix overflow is the contract. Failed callbacks may write their temporary output; the wrapper must not publish it. External callback effects remain.

`dispatch_dictionary.h` adds signed saturating addition using pre-add bounds so no signed overflow is evaluated. Saturation clamps to INT_MIN/MAX. `ds_sat_left` folds from0 in input order; `ds_sat_tree` recursively splits at n/2 and combines child results, with empty identity0. Both admit0..16 ints, null input only at0, live disjoint output, and preserve output on invalid input. For `[INT_MAX,1,-1]`, left fold returns INT_MAX-1 but the tree returns INT_MAX. The implementation does not claim the two functions have equivalent contracts. The actual pthread test runs the two root subtrees on two workers, joins both and combines them, obtaining the tree result. Worker input stays alive through join and each worker has a disjoint result object. No thread creation timing or scheduler determinism is claimed. Retain left order for general signed saturation; a restricted domain such as nonnegative operands admits a separately proved associative saturated sum.

The same test supplies a safe erased-map counterexample: two exactly typed C callbacks return a tagged value, but one changes the promised constructor identity. This compiles and is detected only by a runtime law check. It is not Haskell kind checking over an abstract type constructor. The actual higher_order family demonstrates two explicitly generated concrete carriers; adding another carrier requires more C declarations/operations and does not quantify over every type constructor. ISO C diagnostics validate the stated types, while relations erased into tags or void pointers require checks or discipline. Two reduction mutants clamp to the wrong endpoint or silently replace the specified tree by the left fold; both must fail a named semantic assertion.

## Two-hook membership and relocatable index continuation

`dispatch_intrusive.h` defines two independent circular physical hook chains. Initialize each list with a kind in0..1 and initialize each live item exactly once while unlinked. The sentinel has no parent; never recover a payload from it. Every non-sentinel hook has its genuine live parent and matching kind. Parent, both lists and all linked items stay at stable addresses. A list owns linkage only, not item storage or an external payload resource. `ds_dual_fold` invokes a separately supplied typed operation in list order; callback outputs are staged and error returns preserve caller output. The callback cannot mutate/reenter the active graph or retain the temporary result pointer. Numeric values and callback effects are not transactionally rolled back. Null required arguments reject; init functions require live nonnull typed storage and a valid kind.

The index continuation has four slots, two independent chains, a stable `struct ds_index_owner`, generations1..3 and permanent retirement at4. Handle validation checks the live owner pointer, index, liveness and generation; it cannot validate an arbitrary or dangling owner address. `ds_index_release` returns DS_BUSY until both memberships are removed. `ds_index_relocate` requires a disjoint live typed array of four slots, or the identical current pointer for a no-op; partially overlapping arrays are prohibited. End every raw storage borrow before moving. It copies all four slot representations then publishes the new backing pointer, without changing generations or handle identities. The former array remains a C object but is no longer authoritative protocol storage. No callback, allocator or hidden heap allocation occurs during movement. Outputs/control/member storage do not overlap. Generation retirement is finite resource exhaustion, not wraparound permission. No concurrent access or reentry is supported.

Physical push/remove use bounded constant link work. Index acquisition scans at most four controls; lookup and each hook operation use constant checks; movement copies four typed slots; fold visits at most four elements plus unknown callback work. Destruction requires unlinking both hooks, then releasing the logical slot; caller-owned payload resources need their own destructor protocol. Raw structure copying of a physical item does not repair its hook addresses or parent fields. The moved-array variant avoids those embedded addresses but adds owner/generation validation. Neither representation establishes language-level borrowing, an object hierarchy, or memory safety for forged raw pointers.

## Opaque implementation and shared service continuation

`module_alternative.c` supplies the same DSModule functions in a separate executable, using only the common public declaration for its client. It is never linked together with the original definitions. Both implementations have two static typed slots, two ordered read calls, checked sum and unchanged output on any error. A closed raw pointer must be discarded even though the static backing object still exists.

`dispatch_modules.h` provides a generation-checked lease wrapper and two correlated typed maps. Initialize a live registry once with all entries empty/generation1. All registry, output, service context and control-block pointers designate the stated live types and do not overlap except that read_owner and io_owner may be the identical valid OwShared object. The same owner then gains/releases two references. Context ownership and callback/code lifetime are supplied by the caller; the wrapper cannot infer them from void pointers. The read owner covers the read callback context; the IO owner covers clock and sink contexts. Callback code remains loaded through final release. Services and cleanup callbacks cannot reenter, mutate the registry, escape nonlocally or fail during release. No concurrent access is supported. Keep registry identity alive and stable until all lease copies are discarded; never reinitialize a control while stale tokens exist.

Factory acquisition is read retain, IO retain, opaque module open. OwScope is the actual imported conditional reverse cleanup component, not a copied replacement. Four explicit fault cuts return DS_FAILED without publishing, release the acquired prefix, and preserve every existing entry. The two-slot registry and underlying factory can independently exhaust; both preserve the output token. Real retain errors (dead count or overflow) become DS_FAILED and roll back previous ownership. `ds_lease_twice` uses only the read module; `ds_lease_stamp` passes only DSClock to its leaf; `ds_lease_emit` passes only DSSink. Output staging applies to reads and clocks, not to external sink effects. `ds_lease_close` consumes the live generation before cleanup, closes the module, then drops IO/read references. Counts1..3 are issued,4 is retired; double close and stale tokens reject. Invalid output pointers are rejected before callbacks. These runtime tokens add checked logical identity, not unforgeable security or compiler-enforced borrowing.

The user/metric facades have four actual typed value slots each, live bits and named key/value types. In-range replacement cannot fail and owns no external resources. Missing reads and out-of-range calls preserve outputs. Typed translation diagnostics enforce the finite association at direct calls; no runtime type-name check or incompatible function-pointer call is used.

## Server-held grant protocol and local process boundary

The original broker state has two uint32 counters in0..1000, two object epochs1..4, two holder sequence cursors1..65536 and four grant slots. Epoch4 and sequence65536 are retired states and never wrap. Initialize live storage once. Initial grant IDs1/2 belong to holder0,target0 with rights3/1; ID3 belongs to holder1,target1 with rights3. The fourth slot may hold one attenuated grant, never reused. No pointer is serialized. The `holder` argument is supplied by the authenticated endpoint controller, never request bytes. A private helper accepting an arbitrary caller-supplied holder would violate this boundary.

The request layout is16 eight-bit octets: version1, operation1..4, target0..1, rights0..3, big-endian grant ID32, big-endian sequence16, epoch8, reserved0, big-endian amount32. Read/derive amounts must be0; rights must be0 except derive. Syntax rejection leaves sequence unchanged. Correctly formed exact-next requests consume sequence before authorization; wrong/replayed/exhausted sequence rejects without consumption. Validate live grant, holder, exact target, grant/current epoch and rights before changing a counter. Read requires bit1; add and deputy-add bit2. Addition rejects above1000. Derive requires a rights subset and empty slot4; it publishes a server record and returns ID4. Trusted administrative revoke advances the target epoch through permanent retirement; no unprivileged administrative opcode is supplied.

`ds_broker_apply` returns DS_OK when it has produced a complete reply, including a denial reply; required null arguments or holder>=2 return DS_INVALID without publishing. Reply octets contain version/status/target/granted-rights, grant ID (or newly issued4), echoed sequence, current target epoch, reserved0 and authorized result. Failed requests publish result0 and rights0. A reply may overlap the request because every request field is decoded before publication; neither overlaps state. All pointers designate live objects of the required extent. The encoder simply narrows fields to wire widths; it neither authenticates nor repairs an invalid request. Exact-byte fixtures control the codec independently of matching encoder/decoder calls.

The Linux harness provisions one unnamed local stream pair and forks before application threads. The server initializes its state after the split, closes the unused child endpoint, and assigns holder0 to the retained connection. The child closes the server endpoint and may forge every request byte. No pathname, listener, external service or credential is involved. Server reads at most3 octets at a time, applies only a complete16-octet frame, and sends at most2 octets at a time. EOF with a partial frame discards it without applying. Per-call nonblocking send/recv, poll waits and the32-frame harness limit are explicit OS behavior, not ISO C. Only the known original child is waited for. No claim is made about an absolute request deadline, unbounded client fairness or process hardening.

This boundary permits protocol attacks but excludes arbitrary same-user debugging of the server, kernel compromise, privileged inherited descriptors and unmodeled descriptor transfer. Opaque IDs are connection-scoped server lookup keys; they are not globally secret bearer tokens. Guessed IDs still need holder/target/epoch/rights admission. Server effects may precede a failed response write; the requester then has an unknown outcome. Sequence rejection prevents duplicate execution in this live connection but neither reconstructs lost replies nor supplies durable restart identity. Extending the transport requires explicit authentication, session lifetime, persistence and replay policy. Do not describe this original teaching service as a production sandbox.
# Extensible descriptor contract (X-T69-010)

`plugin_base.h` is the frozen older client declaration. `struct ds_plugin_base` contains
major/minor, extent metadata, required/provided feature masks and a typed apply
function. Major2 is required. The original complete object exported as
`dispatch_plugin_extended` is either this exact type, a `struct ds_plugin_optional` whose
first member has this type, or the first such prefix of the documented Future
fixture. The loader and client agree on the Linux x86-64 ABI, alignment, integer
widths and calling convention. No packed, foreign-language or hostile object
extent is inferred from a size field. Minor versions never alter existing fields.

`ds_feature_acquire` takes a stable open `struct ds_loaded`, known mask0 or1, and an
inactive output initialized to zero. It rejects a short base or wrong major;
required features must be a subset of both provided and known features.
Unsupported required features return `DS_UNSUPPORTED`; unknown optional bits
are ignored. Known bit1 requires extent at least `sizeof(struct ds_plugin_optional)` and
a nonnull describe callback. Only then read the optional member. Successful
acquisition increments the actual module borrow count; four simultaneous
borrows are allowed, the fifth returns `DS_FULL`. Rejection preserves the output
and borrow count. A live feature must not be copied or outlive its stable owner.

Apply stages a scalar and publishes only success. Describe stages up to32 bytes,
publishes exactly the returned count only on success, and rejects a reported
count larger than capacity. Callbacks are trusted not to write outside their
supplied extent; checking a returned count cannot repair an earlier overwrite.
Output bytes, length and control objects are distinct storage. Release clears
the feature then releases its one module borrow; repeated release is invalid.
Callbacks cannot retain transient output storage or reenter owner mutation.
No mechanism allocation occurs; the dynamic loader may allocate internally.

The ten original fixture variants and two independently compiled client views
are in `fixtures/dispatch/plugin_compat.json`. Constructor callbacks increment
only a synthetic host counter. They execute even for rejected missing-symbol
and incompatible-size fixtures, demonstrating why admission checks do not
sandbox same-process code. No stale pointer is called or invalid real loader
handle submitted. The optional description is two bytes `v2`, without a null
terminator. It needs capacity2; capacity1 fails without changing caller outputs.

Run both `dispatch.plugin` and `dispatch.plugin_compat`, plus their compiled
mutants. The cross-compiler probe separately builds GCC/Clang hosts and DSOs,
checks dynamic symbols/dependencies, and retains size, disassembly and stack
artifacts. This tests one compatible ABI; it does not authorize arbitrary
foreign libraries or prove general cross-toolchain binary compatibility.

## Asynchronous registrations and owned reload (X-T69-010-DELAYED, X-T69-011)

Use `dispatch_async.h` with the actual T70 DSmall and T66 counted-byte adapter.
All public registry, loader and reload calls belong to one serialized controller
thread. Only the supplied workers execute callbacks concurrently. Registries,
loader control objects and reload hosts stay at fixed addresses until all jobs
are joined and every registration removed. No memcpy, assignment, relocation or
reinitialization of a live pthread object/owner is permitted. Initialize fresh
storage once; destroy only after successful cleanup. Token copies are checked
identities, never independent owners. No callback may call controller APIs,
retain transient arguments, mutate registry state, throw across C, call
pthread_exit/pthread_cancel, or use signals to escape the protocol. Callbacks
receive copied integers, not pointers into the owned serialized state.

A registry has four typed entries with generations1..3; removal advances to4
and permanently retires that entry. At most12 registration lifetimes fit in one
registry. Each registration acquires one actual DSLoaded code borrow. Queueing
one job acquires a second borrow before pthread_create; a thread-creation failure
releases that new borrow. There is at most one unjoined job per entry. Two
registrations plus their two jobs therefore reach the existing module bound4.
A full registry/borrow budget leaves the output token unchanged. Registration
attempt counters do not wrap; fail_at selects a controlled pre-acquisition failure.
The registration/acknowledgement counters cannot exceed12 in an admitted lifetime.

Workers wait under the registry's default non-robust mutex for go or cancel.
The predicate is rechecked after every wake. Starting a canceled or already
started job is busy. Removal marks cancel and wakes workers, but returns BUSY
while a job remains unjoined, including a completed job. Both registration and
queue borrows remain owned at this point. A queued canceled job never calls the
plugin; an already running call finishes cooperatively. The controller drains
only after done, joins that exact worker once, suppresses canceled results,
releases the queue borrow, then retries removal to acknowledge the registration
and release its final borrow. A canceled drain returns STALE and preserves its
output. Done/notification alone is not removal acknowledgement. No real code is
unloaded while a lease remains. In this Linux port, valid default mutex lock,
unlock and condition operations are assumed to succeed; robust mutexes,
recursive locking, owner death and asynchronous signal use are outside contract.

DSReload owns two stable DSLoaded controls, two DSmall states and two registration
identities per slot. `current==2` and `retired==2` mean absent. A reload stages in
the inactive slot; indices change at commit, owner addresses never move. The
initial load accepts0..8 int32 values. Later changes require initial=NULL/count0
and read the current host-owned snapshot. Values are encoded as four little-endian
octets arithmetically, not by dereferencing int pointers into byte storage.
Initial schema1 admits -1000..1000; schema2 migration admits0..500 and doubles
values. Migration failure at any element preserves all current bytes and identity.
Lengths0..4 use DSmall's16 local bytes; lengths5..8 allocate through the actual
T66 allocator and release through the same adapter. These fixtures exercise
host-owned state; no foreign allocator or plugin-owned destructor is implied.

Preparation opens an original explicit local DSO, migrates all elements into a
32-octet staging array, constructs the candidate DSmall, and acquires two dormant
registrations. They are unobservable to queued work until this serialized call
commits. Failure of either registration unwinds the acquired prefix exactly once,
resets candidate bytes and closes its module. A genuine close error retains that
module in the retired slot for retry; no invalid loader handle is fabricated.
The current service and generation remain unchanged on every precommit failure.
After both registrations exist, publish the new current index and increment
nonwrapping generation0..3; then request cancellation of the former registrations.
One retired slot bounds retained generations and blocks another reload.

Collection retries both old removals and returns BUSY while any acknowledgement
is outstanding. Once quiescent it invokes the supplied collection gate. A gate
failure occurs before real dlclose, retaining the module and state bytes; prior
registration acknowledgements remain committed. Success closes the actual module,
resets its DSmall through its allocator, and frees the retired slot. Repeated
collection of an empty slot is success without calling the gate. Stop first
publishes no current service, then follows the same retirement protocol; BUSY or
failure can leave resources to drain/collect, never an active service to resume.
A gate must not reenter the host, mutate ownership or retain its transient inputs.
All output/control ranges are disjoint and all pointer arguments refer to live
objects. These are checked application protocols, not compiler-enforced linearity.

`async.c` holds a real call with a host mutex/condition barrier and keeps a second
job queued across commit. The original plugin returns from the host barrier into
its DSO before worker completion; this is why a host-stack pause still needs a
code lease. Tests never forcefully terminate that call or close it prematurely.
Four semantic mutants respectively release a registration early, publish a
canceled result, omit precommit prefix rollback, or discard state on failed
collection. Assertions terminate the test process directly, so a deliberately
failed control does not return through a stack still borrowed by a worker.
The63 independent transaction rows and repeated timing cells are separate from
these actual interleavings. No finite tests establish all scheduler histories,
lock freedom, bounded callback latency, transparent replacement or learner mastery.

## Cross-method laws, subobject views and capture admission

`struct ds_copy_view` has length and copy operations over one stable counted source. Both
callbacks have correct C signatures and must preserve the source bytes. Instrumented
call counters may mutate their original non-const fixture; the erased const view
does not authorize mutation of an actually const object. A successful copy must
produce exactly the promised length and preserve the byte sequence, including
embedded zero octets. The wrapper calls length once, rejects lengths above16 or
caller capacity before invoking copy, calls copy once into16 private bytes, and
publishes only on success with an equal reported count. Empty copies still call
both operations. Output bytes/length/control/source are disjoint. A lying count
is rejected; wrong bytes with the right count require an independent semantic
oracle. Trusted callbacks never write beyond supplied capacity. Staging cannot
repair an out-of-bounds callback write or roll back arbitrary external effects.
The original zero-terminated-length/count-preserving-copy fixture is type-correct
but returns FULL for the counted source [1,0,2]; type compatibility did not prove
the cross-method relation. Two compiled controls omit the count relation or
publish a byte before failure validation.

`struct ds_multi` owns two distinct borrowed subobjects: a DSBytesReceiver descriptor and
an embedded DSBox integer. Its two views reference `&owner.bytes` and
`&owner.scalar.value` respectively, using the actual prior typed constructor for
the former. The owner and pointed-to byte array remain live and unmoved. Updating
length/value is visible through the old views; replacing the ops pointer requires
constructing a new view because table selection was captured by value. No whole
object cast, covariance or runtime lifetime proof is inferred. Mutants substitute
a fully valid wrong interface or a stable unrelated int, so their failures are
semantic associations rather than invalid typed accesses.

`ds_capture_checked` admits only the declared int32 encoding schema, required
alignment1 through `_Alignof(int32_t)`, and a scalar result. Unknown/self-referential
or arbitrary typed-object kinds, over-aligned storage requests and borrowed-result
requests return UNSUPPORTED before allocation or ownership transfer. Alignment0
or a non-Boolean result flag is INVALID. This is an explicit admission policy,
not a verifier of arbitrary memory behind a pointer. The caller still supplies a
live aligned int32 array. Accepted construction delegates to actual capture.c,
DSmall and T66 allocation. A typed self-reference copy is demonstrated while
both objects are alive: its pointer still addresses the original; a deliberate
typed relocation repairs it. No copied character buffer is accessed as that
object. A genuine32-byte-aligned typed object supplies the alignment comparison.
A separate diagnostic control proves a pointer-returning callback is not the
scalar DSThunk signature. Returning borrowed objects would require another
interface with an owner lifetime protocol; it is not silently supported here.

## Retained closure environment contract (X-T69-004-R4)

`struct ds_deferred` is stationary fresh storage with one controller and at most one
pthread worker. Bind retains an actual T66 OwShared environment, records a typed
body and issues a generation token. It fails without publication for a dead/full
shared owner. The binding owns the environment through submission, execution and
join/drain. The original environment owner may drop its reference immediately
after bind; the closure's reference keeps its context alive. Controller calls
are serialized, and callbacks never call them or mutate reference counts. A
callback may mutate its own environment only while no other actor accesses those
fields; the test inspects them after join. No robust mutex, signal handler,
forced thread cancellation or arbitrary asynchronous destruction is admitted.

Submit creates a joinable waiting worker. Failure retains the binding for retry.
Start wakes it; cancellation before submission consumes the binding immediately.
Cancellation with an outstanding job returns BUSY and keeps the reference until
join/drain. Queued cancellation invokes no body; running cancellation waits for
the body to return and suppresses its result. Completion consumes the binding
before dropping its last environment reference, so destruction/release happens
after every callback access. Body failure also consumes the once binding and
preserves caller output. An external body effect is not rolled back. Normal
completion, queued cancellation, running cancellation, failed body and pre-submit
cancellation each destroy/release the sole environment exactly once in the fixture.
A copied token refers to the same central generation and cannot call twice;
copying the pthread control itself remains prohibited by contract. Generations1..3
then retire4 without reuse. This is runtime protocol enforcement, not C linearity.

The scalar DSClosure history control separately executes every three-call word
with arguments-4..4, every initial value-4..4 and all three modes:59,049 transitions.
INT_MIN/INT_MAX failed attempts and copied once-state counterexamples are separate.
Both copied plain once closures can produce a value because ISO C allows struct
copying; the retained token protocol checks shared central state instead.
