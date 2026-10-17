# II. Control Flow, Error Handling & Vocabulary Types

This category distinguishes absence, recoverable failure, partial progress, cancellation, contract violation, and non-returning execution. Collapsing all of them into a null pointer, exception, or boolean discards information that downstream code needs.

<a id="c01"></a>
## C01. Explicit absence with optional values

**1. Modern approach & concrete entities.** Rust `Option<T>`; C++ `std::optional<T>`; Swift `T?`; Zig `?T`; V option types. Optionality belongs to the type rather than to an undocumented subset of payload values. [RUST-OPTION] [CPP-OPTIONAL] [V-OPTIONS]

**2. Classical antecedent / historical anti-pattern.** `-1` for a missing index, zero for an absent measurement, an empty string for “not provided,” and a separate validity flag that can disagree with its value.

**3. Underlying mechanism & memory layout.** Semantically, `Option<T>` is `None | Some(T)`. A conventional representation stores payload space and an engagement tag. A niche can encode absence using a representation unavailable to valid `T` values. Rust documents specific null-pointer/nonzero optimizations; the guarantee is not universal for every `T`. C++ optional holds its value within its own storage rather than requiring a separate allocation. [RUST-OPTION] [CPP-OPTIONAL]

**4. Engineering trade-offs & footprint.** Optional payloads can increase size and alignment. The abstraction may still use a physical sentinel internally; what improves is its encapsulated, type-checked meaning. Nested optionals can represent different absence reasons but often deserve named alternatives instead. An optional reference does not own the referent. A present pointer payload can itself be null when the type permits it. Unchecked unwrapping merely relocates the failure risk; absence must be eliminated through a branch, default, propagation, or a proved invariant.

<a id="c02"></a>
## C02. Disjoint success/failure results

**1. Modern approach & concrete entities.** Rust `Result<T,E>`; C++23 `std::expected<T,E>`; Swift `Result<Success,Failure>`; Zig `E!T`; V `!T`; Roc `Try(T,E)` in its current new-compiler tutorial. C3 `T?` represents an optional result carrying a fault rather than simply a nullable payload. [RUST-RESULT] [CPP-EXPECTED] [ROC-TUTORIAL] [C3-OPTIONALS]

**2. Classical antecedent / historical anti-pattern.** Magical status integers, `errno` disconnected from the returned value, output parameters usable after failure, and partially initialized “result” structs without a defined state model.

**3. Underlying mechanism & memory layout.** A disjoint result contains either the success payload or error payload. It normally uses inline union-like storage with a discriminant or available niche. Branching on the outcome selects the valid alternative. Error payloads can be compact enums or richer structures; those structures may themselves own allocations or references.

**4. Engineering trade-offs & footprint.** A large error type enlarges the result even on successful paths. Returning the value may require copying/moving, hidden return storage, or additional registers depending on ABI. Converting lower-level errors can lose detail or allocate context. An explicit result does not prevent unrelated panics, exceptions, or process termination. C++ `expected` does not have `variant`'s `valueless_by_exception` state, but operations can still throw according to the contained types and operation contracts. [CPP-EXPECTED]

<a id="c03"></a>
## C03. Product returns for partial progress and independent outcomes

**1. Modern approach & concrete entities.** Go `(T,error)` and `io.Reader.Read` returning `(n int, err error)`; named structs such as `ReadOutcome { bytes, status }`; Rust or C++ domain structs returning partial data alongside a terminal condition. [GO-IO]

**2. Classical antecedent / historical anti-pattern.** Assuming every operation is all-or-nothing, discarding bytes obtained before EOF, and using one integer both as a byte count and a negative error code without a precise contract.

**3. Underlying mechanism & memory layout.** A product stores both components simultaneously rather than choosing between them. This can intentionally represent `n > 0` together with an error. The return convention may place fields in registers or caller-provided storage. Unlike a disjoint union, a plain product does not exclude meaningless combinations unless construction and validation establish that restriction.

**4. Engineering trade-offs & footprint.** The caller must process the success component before interpreting the terminal condition when the API requires it. Go's typed-nil interface behavior can surprise error-producing code: an interface containing a typed nil pointer is not necessarily a nil interface. Product results can be clearer for streaming and incremental operations, whereas disjoint results better express truly exclusive outcomes. Selecting `Result` merely because it looks more modern can make a partial-progress protocol less accurate. Model actual outcome cardinality before choosing representation. [GO-FAQ]

<a id="c04"></a>
## C04. Structured early-return propagation

**1. Modern approach & concrete entities.** Rust `?`; Zig `try` and `catch`; V result propagation with `!`; C3 postfix `!`; Roc `?`. These are language-specific propagation mechanisms, not identical operators with identical error models. [RUST-TRY] [ZIG] [V-OPTIONS] [C3-OPTIONALS] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Repeated `if (status != OK) return status;`, deeply nested success branches, and forgetting to release resources on one newly introduced error path.

**3. Underlying mechanism & memory layout.** Propagation commonly lowers to examining an outcome, extracting its success value, or returning the error through the enclosing function's return convention. Language-specific conversions may transform the error. Live local owners are destroyed according to the language's scope rules. No extra persistent result object is inherently required beyond the intermediate values that survive optimization.

**4. Engineering trade-offs & footprint.** Propagation is not recovery, logging, retry, rollback, or compensation. A convenient operator can make a function exit at many points, which matters for state mutation and resource transactions. Error conversion can discard category information. Rust's underlying generalized propagation traits have stability constraints distinct from everyday stable `?` usage; do not assume every customization is portable stable Rust. C++ `expected` supplies a result representation but does not itself add a language-level `?` operator. Make ownership and partially performed effects visible around propagation sites.

<a id="c05"></a>
## C05. Typed throwing and explicit exception boundaries

**1. Modern approach & concrete entities.** Swift `throws(E)` and `throws(Never)`; C++ `noexcept` and explicit exception-handling boundaries; result-returning adapters at FFI or subsystem edges. Typed throwing constrains the error effect rather than requiring the caller to manipulate a result object. [SWIFT-THROWS] [CPP-EXCEPT]

**2. Classical antecedent / historical anti-pattern.** Undocumented exceptions escaping arbitrary layers, treating every failure as an untyped string, and allowing foreign callers to encounter unwinding they cannot handle.

**3. Underlying mechanism & memory layout.** A typed effect records which error type a function can throw. Lowering and ABI are language-specific: Swift error propagation is not mechanically equivalent to C++ exception unwinding. Typical C++ table-driven exception ABIs use unwind metadata and cold-path runtime machinery, but the language does not prescribe a universal implementation. `noexcept` restricts propagation behavior; it is not proof that the body contains no throwing operation.

**4. Engineering trade-offs & footprint.** Unwind tables, exception objects, cleanup paths, and optimizer constraints can affect code size and latency. A failure to respect a non-throwing boundary can terminate execution instead of recovering. Typed errors make handling more precise but can couple layers to large error taxonomies. Exceptions can remain reasonable for rare non-local failures; explicit results can suit expected operational failures. Compare equivalent failure behavior rather than contrasting checked results with unchecked integer returns and calling the extra branch “overhead.”

<a id="c06"></a>
## C06. Monadic mapping, flattening, and error transformation

**1. Modern approach & concrete entities.** Rust `map`, `map_err`, `and_then`, `or_else`, and `transpose`; C++23 optional/expected monadic operations such as `and_then`, `transform`, and `transform_error`; Swift `map` and `flatMap`. [RUST-RESULT-METHODS] [CPP-EXPECTED-MONADIC] [CPP-OPTIONAL-MONADIC]

**2. Classical antecedent / historical anti-pattern.** Repeated unwrap/check/rewrap code, nested `Result<Result<T,E>,E>` values, and branching that hides the actual data transformation.

**3. Underlying mechanism & memory layout.** `map` changes a success value using `T -> U`; `and_then` composes `T -> Result<U,E>` and flattens one result layer. Error-mapping operates on the other alternative. A compiler can inline these short functions into the same branch sequence as explicit control flow. `transpose` can exchange nesting such as `Option<Result<T,E>>` and `Result<Option<T>,E>` without erasing their distinct semantics.

**4. Engineering trade-offs & footprint.** Closures can capture owners, borrow data, allocate, or invoke indirect calls. Fluent syntax does not establish purity or costlessness. Large chains increase type complexity and may make debugger stepping less clear. Short-circuit composition evaluates dependent stages sequentially and normally stops at the first failure; it is not error accumulation. Name intermediate domain operations when a chain hides important side effects or ownership transitions. “Monadic” describes a compositional shape and laws, not necessarily a full higher-kinded type system.

<a id="c07"></a>
## C07. Applicative validation and error accumulation

**1. Modern approach & concrete entities.** Scala Cats `Validated`/`ValidatedNel`, applicative combination of independent validations, or explicit `Validation<T, Errors>` libraries in other languages. This is a library-level technique, not a built-in feature shared by every surveyed language. [CATS-VALIDATED]

**2. Classical antecedent / historical anti-pattern.** Rejecting a configuration after only its first bad field, mutating one shared error list from every validator, or chaining dependent results when the checks are actually independent.

**3. Underlying mechanism & memory layout.** Independent computations each produce a valid value or errors. A combination operation constructs the aggregate when all succeed, otherwise combines errors using a defined associative operation. Unlike monadic sequencing, later validation does not require the successful value of an earlier stage. The error collection can be a fixed-capacity buffer, linked non-empty collection, vector, or another representation.

**4. Engineering trade-offs & footprint.** All relevant checks run rather than stopping early, and error accumulation consumes storage. Validation dependencies limit parallel or applicative composition. Ordering and duplicate handling must be specified. Bounded systems should define what happens when the diagnostic capacity is exhausted. Accumulation is appropriate for forms, schemas, and configurations; it is often inappropriate when a failing operation mutates a device or invalidates assumptions needed by subsequent checks. Keep validation separate from committing external effects.

<a id="c08"></a>
## C08. Unit, bottom, and impossible outcome types

**1. Modern approach & concrete entities.** Rust `()`, `!`, and `Infallible`; Swift `Void` and `Never`; Zig `void` and `noreturn`; C++ `expected<void,E>` and `std::monostate`. [RUST-INFALLIBLE] [RUST-NEVER] [CPP-EXPECTED]

**2. Classical antecedent / historical anti-pattern.** Dummy success integers, fake payload fields in commands, and keeping an error branch in an API even when it cannot be constructed through valid code.

**3. Underlying mechanism & memory layout.** A unit type has one logical value; an uninhabited type has none. `Result<T,Infallible>` cannot produce its error alternative through valid safe construction. A diverging expression can fit control-flow branches without supplying a normal result. Compilers can remove impossible alternatives and their dead code where the language guarantees permit it.

**4. Engineering trade-offs & footprint.** An erased unit value in Rust does not imply every empty C++ object has size zero. Availability of the Rust never type in all possible type positions must be distinguished from established non-returning return-type usage. Marking a branch unreachable is not a substitute for establishing that it cannot occur. Foreign input, unsafe values, or invalid discriminants can violate the assumptions behind the proof. Use impossible types for genuinely impossible outcomes, not failures that are merely rare or currently unimplemented.

<a id="c09"></a>
## C09. Lexically registered cleanup and error-only cleanup

**1. Modern approach & concrete entities.** Go, Swift, Odin, and C3 `defer`; Zig `defer` and `errdefer`; C++ scope-guard libraries or an explicitly implemented guard. Do not assume `std::scope_exit` is universally part of the standard library merely because similarly named experimental/library facilities exist. [GO-DEFER] [ZIG] [SCOPEGUARD]

**2. Classical antecedent / historical anti-pattern.** Scattered cleanup labels, acquisition far from release logic, and duplicated rollback calls on every early return.

**3. Underlying mechanism & memory layout.** A cleanup action is registered and invoked at its defined exit point. Compilers can lower fixed lexical cleanup to direct calls; dynamic registrations may require bookkeeping. Go evaluates deferred call arguments when registering the call and executes defers at function exit. Zig `errdefer` applies to error exit from its scope, not ordinary success.

**4. Engineering trade-offs & footprint.** Scope rules vary across languages. A Go defer inside a long loop can retain resources until the enclosing function returns. Captured variables or pointer references can refer to changed or expired state if the API permits escape. Deferred cleanup is not ownership checking; double cleanup is still possible in a permissive language. Error-only cleanup must be disabled or transferred correctly when ownership is handed off. Check release ordering, nested scopes, and whether cleanup itself can fail or block.

<a id="c10"></a>
## C10. Transactional updates and commit/rollback guards

**1. Modern approach & concrete entities.** Build-then-commit, copy-and-swap where applicable, RAII rollback guards, consuming transaction handles, and domain-specific `commit() -> Result`. C++ exception-safety guarantees provide useful vocabulary: no-throw, strong, and basic guarantees. [CPP-GUIDELINES] [SCOPEGUARD]

**2. Classical antecedent / historical anti-pattern.** Mutating shared state incrementally and leaving it half-updated when allocation, validation, or I/O fails; assuming destructor execution automatically undoes prior effects.

**3. Underlying mechanism & memory layout.** Work occurs in temporary storage or under a rollback record. A successful final step publishes the new state, often through a non-throwing swap or pointer replacement. Before commit, failure destroys provisional resources and preserves the previous visible state. An undo log stores inverse operations when a full copy is unsuitable.

**4. Engineering trade-offs & footprint.** Temporary copies, logs, and retained old state increase peak memory. A commit is only atomic relative to an explicitly defined observer/concurrency model. External effects may not be reversible: sending a message, starting DMA, or writing a register cannot generally be undone by restoring a C++ object. Filesystem or database transactions need their own guarantees. Strong exception safety is therefore a local contract to establish, not a consequence of using `Result`, exceptions, or RAII alone.

<a id="c11"></a>
## C11. Explicit arithmetic and conversion failure policy

**1. Modern approach & concrete entities.** Rust `checked_add`, `wrapping_add`, `saturating_add`, `overflowing_add`, and `TryFrom`; Zig wrapping/saturating arithmetic; domain wrappers distinguishing representable, bounded, and scaled values. [RUST-U32] [RUST-TRYFROM] [ZIG]

**2. Classical antecedent / historical anti-pattern.** Assuming signed overflow wraps, silent narrowing casts, conflating arithmetic overflow with sensor saturation, and performing size multiplication before checking it.

**3. Underlying mechanism & memory layout.** Checked arithmetic returns an optional/result or overflow flag; saturating arithmetic clamps; wrapping arithmetic computes modulo the integer range. Lowering can use carry/overflow flags, comparisons, wide intermediates, or dedicated instructions. Fallible conversion establishes representability in the destination type.

**4. Engineering trade-offs & footprint.** Checks may introduce branches or additional instructions, while saturation may hide invalid upstream calculations. A representable conversion does not validate units or physical meaning. Multiplication/addition order matters for overflow checks, especially for buffer sizes and offsets. Floating-point operations require a different model involving rounding, NaNs, infinities, and precision rather than integer overflow alone. Select the policy per operation; globally choosing wrap or saturate is rarely an adequate domain specification. Compiler build modes must not silently change a critical numeric contract.

<a id="c12"></a>
## C12. Expression-oriented control flow and guarded bindings

**1. Modern approach & concrete entities.** Rust `if`/`match` expressions, `if let`, and `let ... else`; Swift `guard let`; Zig conditional and switch expressions; Nix `if ... then ... else`. [RUST-IF] [RUST-LET] [SWIFT-CONTROL] [NIX-LANGUAGE]

**2. Classical antecedent / historical anti-pattern.** Declaring an uninitialized variable and assigning it in branches, keeping invalid values in scope, and deeply nesting normal execution under defensive checks.

**3. Underlying mechanism & memory layout.** A branch expression computes one value and joins through a compiler-level control-flow merge, commonly represented by an SSA phi or equivalent. A guarded binding narrows the usable state: the success value exists only after the pattern has been established, while the failure branch exits or handles the alternative.

**4. Engineering trade-offs & footprint.** Expression syntax does not remove branch instructions. All producing branches need compatible types, and moving values into one branch can constrain later use. A guard can improve readability but may hide too many early exits in a long function. Dynamically typed Nix performs its checks during evaluation; expression orientation alone is not compile-time type safety. Avoid default-initializing a semantically invalid value merely to satisfy syntax when a value-producing branch can express the actual construction directly.

<a id="c13"></a>
## C13. Lazy ranges, iterator pipelines, and loop fusion

**1. Modern approach & concrete entities.** C++20 ranges/views; Rust `Iterator`; Swift lazy sequences; Go `iter.Seq`/`Seq2` and range-over-function iteration. [CPP-RANGES] [RUST-ITERATOR] [GO-ITER]

**2. Classical antecedent / historical anti-pattern.** Allocating an intermediate collection for every filter/map, duplicated traversal code, and hand-maintained indices that obscure the algorithm's data transformation.

**3. Underlying mechanism & memory layout.** Adaptors contain traversal state and callable environments. Pull iterators produce the next element on demand; Go's sequence interface uses a yield callback. Specialization and inlining can fuse several stages into one loop. A lazy view need not materialize its output, although a terminal `collect` or equivalent does.

**4. Engineering trade-offs & footprint.** Laziness defers work and failure, and repeated traversal may repeat computation. Captured references can dangle in languages without appropriate lifetime checking. Not every C++ view is non-owning, and `borrowed_range` is not a borrow checker. Iterator-category requirements affect algorithm complexity. Closure state and nested adaptor types can increase compilation time and code size. Fusion is an optimization opportunity, not a contractual guarantee that every pipeline beats a direct loop. Preserve a measurable relationship to the expected access pattern.

<a id="c14"></a>
## C14. Typed early termination in higher-order algorithms

**1. Modern approach & concrete entities.** Rust `ControlFlow<Break,Continue>`, `Iterator::try_fold`, and `try_for_each`; explicit stop/continue enums in callback-driven traversals. [RUST-CONTROLFLOW] [RUST-ITERATOR]

**2. Classical antecedent / historical anti-pattern.** Returning `false` to mean either success, failure, or “stop searching”; throwing an exception solely to escape a callback; and storing a discovered value in an outer mutable variable.

**3. Underlying mechanism & memory layout.** The callback returns a typed control result, optionally carrying the break value. The traversal inspects the discriminant and either continues or returns the payload. Static generic callbacks can inline this into the same control flow as an ordinary loop with `break value` behavior.

**4. Engineering trade-offs & footprint.** The traversal and callback must agree whether stopping is normal completion, a found result, cancellation, or failure. A break payload can enlarge temporary values. Short-circuit behavior changes which side effects execute and how much input is consumed, which is important for streams and parsers. This technique avoids conflating algorithm control with exceptional failure, but it does not establish that a partially consumed iterator can safely be retried or reused without an explicit contract.

<a id="c15"></a>
## C15. Type-checked formatting and output construction

**1. Modern approach & concrete entities.** C++ `std::format_string<Args...>`, `format`, and `format_to`; Rust `format_args!`, `write!`, and formatting traits. [CPP-FORMAT] [RUST-FORMAT]

**2. Classical antecedent / historical anti-pattern.** `printf` argument/format mismatches, buffer overflow through unbounded formatting, and converting every intermediate value to a temporary string before writing it.

**3. Underlying mechanism & memory layout.** Static format strings can be checked against argument types during translation. A formatting argument package can borrow values and route them to a sink, avoiding construction of one combined intermediate string. The formatter still executes numeric conversion, padding, escaping, or user-defined formatting logic at runtime.

**4. Engineering trade-offs & footprint.** Producing an owning result string can allocate. `format_to` is not automatically bounded: the output iterator/sink must enforce capacity when necessary. Runtime format strings need the API's runtime-validation path. User-defined formatters can perform expensive work or fail. Borrowed argument objects have lifetime restrictions. Binary size may grow from instantiated formatters or Unicode/locale support. Type checking solves argument consistency; bounded memory use, log truncation policy, and blocking I/O remain separate requirements.

<a id="c16"></a>
## C16. Must-use results and deliberate discard

**1. Modern approach & concrete entities.** Rust `#[must_use]`, the must-use behavior of result/future types, and C++ `[[nodiscard]]` with diagnostic messages. [RUST-DIAGNOSTICS] [CPP-NODISCARD]

**2. Classical antecedent / historical anti-pattern.** Ignoring return codes accidentally, constructing a lazy future and never polling it, and discarding a guard immediately while believing it protects a later region.

**3. Underlying mechanism & memory layout.** The compiler diagnoses certain discarded expressions according to language and diagnostic rules. No per-value runtime tracking is necessary. Projects can promote relevant warnings to errors while allowing explicit, reviewable discard where appropriate.

**4. Engineering trade-offs & footprint.** Diagnostics are not a proof of meaningful handling: assigning to an unused variable or explicitly discarding can satisfy the mechanism. Warning policies differ, and not all ways of losing a value are diagnosed. Excessive annotation can create noise that users suppress indiscriminately. The goal is to make accidental loss conspicuous, especially for failure-bearing values and lazy operations. Pair this with useful error types and review rules; a must-use integer still lacks a clear domain contract, and a nodiscard view can still dangle.

<a id="c17"></a>
## C17. Coroutines and compiler-generated asynchronous state machines

**1. Modern approach & concrete entities.** C++20 `co_await`, `co_yield`, promise types, and `coroutine_handle`; Rust `async`, `Future`, and `Poll`; Swift `async`/`await`. [CPP-COROUTINE] [RUST-FUTURE] [SWIFT-CONCURRENCY]

**2. Classical antecedent / historical anti-pattern.** Callback pyramids, manually numbered resume states, and allocating an OS thread for every pending I/O operation.

**3. Underlying mechanism & memory layout.** Suspension points become states, and locals that remain live across suspension become fields in a frame/future. Resumption dispatches from that state. Nested Rust futures can be stored inline; some address-sensitive futures need pinning before polling. C++ supplies frame allocation machinery and permits allocation elision in specified circumstances. Executors, not the `await` keyword alone, determine scheduling.

**4. Engineering trade-offs & footprint.** Large live locals enlarge the frame; spawning or boxing may allocate. A borrowed object must remain valid across suspension. Awaiting does not necessarily yield, and blocking inside asynchronous code can stall its executor. Destroying a suspended computation must follow its language/library lifecycle rules. Cancellation is not rollback, and a dropped future may have already initiated external work. State-machine syntax simplifies control structure while making frame size, capture ownership, and suspension boundaries central design concerns.

<a id="c18"></a>
## C18. Structured concurrency and scoped task lifetimes

**1. Modern approach & concrete entities.** Swift `async let` and task groups; Rust `std::thread::scope` for scoped threads; libraries that bind child tasks to a parent scope. [SWIFT-CONCURRENCY] [RUST-SCOPE]

**2. Classical antecedent / historical anti-pattern.** Detached workers outliving borrowed inputs, untracked background tasks, and shutdown implemented through sleeps and global flags.

**3. Underlying mechanism & memory layout.** The enclosing scope owns or coordinates child execution and does not finish in the normal structured fashion until required child completion is resolved. Scoped threads can borrow suitable local data because the scope bounds their lifetime. Task-group state tracks children, completions, and cancellation relationships; the exact representation depends on the runtime.

**4. Engineering trade-offs & footprint.** Joining children can delay exit indefinitely if they do not terminate. Cancellation is commonly cooperative and must be observed by the work. A task scope establishes lifetime structure, not hard deadlines, transactional behavior, or deterministic scheduling. Scoped OS threads still have stacks and kernel/runtime costs. A cancellation token alone does not make detached task launching structured. Identify the scope that owns each task, its shutdown responsibility, and how errors from multiple children are collected or prioritized.

<a id="c19"></a>
## C19. Explicit cancellation, deadlines, and bounded backpressure

**1. Modern approach & concrete entities.** Go `context.Context`; cancellation-safe asynchronous operations; bounded channels such as Tokio `mpsc`; C++ stop-token facilities. [GO-CONTEXT] [TOKIO-CHANNELS] [TOKIO-SELECT] [CPP-STOP]

**2. Classical antecedent / historical anti-pattern.** Global “stop” booleans, infinite queues absorbing overload, timeouts checked only after work completes, and abandoning an operation while it still accesses freed memory.

**3. Underlying mechanism & memory layout.** Cancellation state is shared through a token/context and observed at defined points. Deadlines connect time to that state. A bounded queue limits in-flight messages; producers wait, fail, or apply an explicit drop policy when capacity is exhausted. Cancellation safety describes whether interrupting and retrying an operation loses or duplicates progress.

**4. Engineering trade-offs & footprint.** Tokens, callbacks, timers, waiters, and queues consume storage and synchronization work. A deadline notification does not prove the operation has ceased. Selecting between futures can drop losing branches, so their cancellation contracts matter. Backpressure can propagate latency or deadlock if a producer holds a resource the consumer needs. Distinguish queue capacity, task count, payload ownership, and external in-flight I/O; bounding only one does not bound the entire subsystem.

<a id="c20"></a>
## C20. Typed asynchronous composition with senders and receivers

**1. Modern approach & concrete entities.** C++26 `std::execution` sender/receiver facilities: `just`, `then`, `when_all`, `connect`, `start`, schedulers, and completion channels `set_value`, `set_error`, and `set_stopped`. [CPP-EXEC]

**2. Classical antecedent / historical anti-pattern.** Untyped callback/context registries, opaque scheduler placement, and one virtual interface or allocation per asynchronous composition stage.

**3. Underlying mechanism & memory layout.** A sender describes a computation. Connecting it to a receiver constructs an operation state; starting initiates execution. Completion signatures carry possible value/error/stopped outcomes. Template composition can embed child operation states directly and resolve many connections statically, rather than requiring every stage to be separately heap-allocated.

**4. Engineering trade-offs & footprint.** Runtime schedulers and particular adaptors can still allocate or dispatch dynamically. Operation-state lifetime must cover the asynchronous work that uses it. Large composed types increase compilation and diagnostic complexity. Stopped completion is not the same as an application error or successful rollback. Execution placement, cancellation forwarding, and completion ordering remain explicit design questions. This is a standardization direction represented in the C++26 draft; verify implementation support rather than assuming a C++26 language switch enables the complete library on every toolchain.

<a id="c21"></a>
## C21. Actor isolation and serialized access to mutable state

**1. Modern approach & concrete entities.** Swift `actor`, `@MainActor`, isolated parameters, and `Sendable` boundaries; actor-style services implemented with owned state plus typed mailboxes in other languages. [SWIFT-ACTORS] [SWIFT-SENDABLE] [TOKIO-CHANNELS]

**2. Classical antecedent / historical anti-pattern.** Shared mutable objects with locks added individually, arbitrary callbacks accessing a service's internals, and assuming that all methods of a class run on one thread.

**3. Underlying mechanism & memory layout.** Actor-isolated state is accessed through an execution domain that serializes relevant work. Cross-boundary interactions may enqueue jobs and suspend callers. State resides with the actor; messages/jobs carry values or permitted references. Isolation is a semantic restriction, not necessarily a permanent dedicated OS thread.

**4. Engineering trade-offs & footprint.** Mailboxes, scheduling, and synchronization introduce overhead. Swift actor methods can be reentrant across `await`: another operation may change state before the suspended method resumes. Serialization of individual segments is not atomicity of an entire multi-await protocol. Slow work can create queueing delays, and unbounded mailboxes can exhaust memory. Invariant checks often need repeating after suspension. Actors reduce accidental shared mutation but do not eliminate deadlock-like dependency cycles, starvation, or application-level races.

<a id="c22"></a>
## C22. Effect tracking and algebraic effect handlers

**1. Modern approach & concrete entities.** Koka effect rows and handlers; Roc's documented distinction between pure `->` and effectful `=>` function arrows. Roc's distinction should not be presented as equivalent to a general algebraic effect-handler implementation. [KOKA] [ROC-TUTORIAL]

**2. Classical antecedent / historical anti-pattern.** Signatures that conceal I/O, mutation, suspension, or failure; implicit global dependencies; and using exceptions as the only extensible non-local control mechanism.

**3. Underlying mechanism & memory layout.** The type system tracks which operations a computation may perform. Effect-polymorphic code can preserve the effects of a supplied computation. A handler provides an interpretation and may resume a continuation. Implementations can use evidence passing, continuation transformations, specialization, or runtime continuation objects; annotation erasure does not imply all handling operations erase.

**4. Engineering trade-offs & footprint.** Continuations may allocate or retain substantial state, and multi-shot handling complicates resource ownership. Effect inference and handler nesting can make diagnostics difficult. The tracked set is only as expressive as the language's effect model. A function can be effectful and still return an explicit `Result`: operational effects and failure representation are separate dimensions. Effect systems can expose hidden behavior, but they do not automatically prove termination, bounded resource usage, or correct interaction with foreign operations.

<a id="c23"></a>
## C23. Executable interface contracts

**1. Modern approach & concrete entities.** C++26 `pre`, `post`, and `contract_assert`; C3 contract annotations; runtime assertions tied to precise interface predicates. [CPP-CONTRACTS] [C3-CONTRACTS]

**2. Classical antecedent / historical anti-pattern.** Preconditions buried in comments, duplicated assertions with unclear responsibility, and implicit assumptions about valid indices, shapes, or initialized state.

**3. Underlying mechanism & memory layout.** Predicates are associated with calls, returns, or implementation points. Depending on the applicable evaluation semantics, execution can check them and invoke violation handling. Contracts need not add per-object storage; they add evaluation and failure-handling paths when enabled. They are distinct from refinement types and machine-checked proofs.

**4. Engineering trade-offs & footprint.** Predicates can be expensive and may run in different modes. An ignored or non-terminating check does not establish that the condition holds for subsequent execution. Keep predicates free from essential side effects. Malformed external input normally requires ordinary rejection, not reliance on a contract-violation mode. Postconditions involving external state can race or become stale. Contracts make obligations explicit and tool-visible; whether they enforce, diagnose, or merely document a condition must be resolved for the selected language and build configuration.

<a id="c24"></a>
## C24. One-time initialization and synchronized lazy values

**1. Modern approach & concrete entities.** Rust `OnceLock<T>` and `LazyLock<T>`; C++ function-local static initialization and `std::call_once`; Go `sync.Once` and related helpers. [RUST-ONCE] [CPP-ONCE] [GO-ONCE]

**2. Classical antecedent / historical anti-pattern.** Unsynchronized “if not initialized” flags, unsafe double-checked locking, global initialization-order coupling, and repeated expensive computation of an immutable shared value.

**3. Underlying mechanism & memory layout.** A state machine tracks uninitialized, initializing, and initialized conditions. Synchronization publishes the fully initialized value before other threads obtain it. Fast paths typically inspect completion state; losing initializers wait or observe the winner. Failure/retry and poisoning behavior differ among APIs.

**4. Engineering trade-offs & footprint.** First use can allocate, block, or perform expensive initialization, creating a latency spike. Recursive initialization may deadlock or otherwise fail according to the API. The first accessing thread can inherit unexpected work or affinity requirements. A cached value can retain memory for the process lifetime and may be unsuitable when configuration must change. One-time initialization establishes publication safety, not perpetual validity of external facts captured during initialization. Prefer explicit startup initialization where predictability matters more than deferred work.

<!-- REFERENCE DEFINITIONS -->
[C3-CONTRACTS]: https://c3-lang.org/language-common/contracts/ "C3 documentation: contracts"
[C3-OPTIONALS]: https://c3-lang.org/language-common/optionals-essential/ "C3 documentation: fallible optionals"
[CATS-VALIDATED]: https://typelevel.org/cats/datatypes/validated.html "Cats: Validated and accumulated validation"
[CPP-CONTRACTS]: https://eel.is/c++draft/basic.contract "C++ working draft: contracts"
[CPP-COROUTINE]: https://eel.is/c++draft/dcl.fct.def.coroutine "C++ working draft: coroutines"
[CPP-EXCEPT]: https://eel.is/c++draft/except "C++ working draft: exception handling"
[CPP-EXEC]: https://eel.is/c++draft/exec "C++ working draft: execution control"
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-EXPECTED-MONADIC]: https://eel.is/c++draft/expected.object.monadic "C++ working draft: expected monadic operations"
[CPP-FORMAT]: https://eel.is/c++draft/format "C++ working draft: formatting"
[CPP-GUIDELINES]: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines "C++ Core Guidelines"
[CPP-NODISCARD]: https://eel.is/c++draft/dcl.attr.nodiscard "C++ working draft: nodiscard"
[CPP-ONCE]: https://eel.is/c++draft/thread.once "C++ working draft: call_once"
[CPP-OPTIONAL]: https://eel.is/c++draft/optional "C++ working draft: optional"
[CPP-OPTIONAL-MONADIC]: https://eel.is/c++draft/optional.monadic "C++ working draft: optional monadic operations"
[CPP-RANGES]: https://eel.is/c++draft/ranges "C++ working draft: ranges"
[CPP-STOP]: https://eel.is/c++draft/thread.stoptoken "C++ working draft: stop tokens"
[GO-CONTEXT]: https://pkg.go.dev/context "Go standard library: context"
[GO-DEFER]: https://go.dev/blog/defer-panic-and-recover "Go: defer, panic, and recover"
[GO-FAQ]: https://go.dev/doc/faq "Go frequently asked questions"
[GO-IO]: https://pkg.go.dev/io "Go standard library: io"
[GO-ITER]: https://pkg.go.dev/iter "Go standard library: iter"
[GO-ONCE]: https://pkg.go.dev/sync "Go standard library: synchronization and once"
[KOKA]: https://koka-lang.github.io/koka/doc/book.html "Koka language book"
[NIX-LANGUAGE]: https://nix.dev/manual/nix/2.34/language/ "Nix 2.34 reference manual: language"
[ROC-TUTORIAL]: https://raw.githubusercontent.com/roc-lang/roc/main/docs/mini-tutorial-new-compiler.md "Roc official new-compiler tutorial"
[RUST-CONTROLFLOW]: https://doc.rust-lang.org/std/ops/enum.ControlFlow.html "Rust standard library: ControlFlow"
[RUST-DIAGNOSTICS]: https://doc.rust-lang.org/reference/attributes/diagnostics.html "Rust Reference: diagnostic attributes"
[RUST-FORMAT]: https://doc.rust-lang.org/std/fmt/index.html "Rust standard library: formatting"
[RUST-FUTURE]: https://doc.rust-lang.org/std/future/trait.Future.html "Rust standard library: Future"
[RUST-IF]: https://doc.rust-lang.org/reference/expressions/if-expr.html "Rust Reference: if expressions"
[RUST-INFALLIBLE]: https://doc.rust-lang.org/std/convert/enum.Infallible.html "Rust standard library: Infallible"
[RUST-ITERATOR]: https://doc.rust-lang.org/std/iter/trait.Iterator.html "Rust standard library: Iterator"
[RUST-LET]: https://doc.rust-lang.org/reference/statements.html "Rust Reference: statements and let-else"
[RUST-NEVER]: https://doc.rust-lang.org/reference/types/never.html "Rust Reference: never type"
[RUST-ONCE]: https://doc.rust-lang.org/std/sync/struct.OnceLock.html "Rust standard library: OnceLock"
[RUST-OPTION]: https://doc.rust-lang.org/std/option/index.html "Rust standard library: Option and representation guarantees"
[RUST-RESULT]: https://doc.rust-lang.org/std/result/index.html "Rust standard library: Result"
[RUST-RESULT-METHODS]: https://doc.rust-lang.org/std/result/enum.Result.html "Rust standard library: Result methods"
[RUST-SCOPE]: https://doc.rust-lang.org/std/thread/fn.scope.html "Rust standard library: scoped threads"
[RUST-TRY]: https://doc.rust-lang.org/reference/expressions/operator-expr.html "Rust Reference: operator expressions and propagation"
[RUST-TRYFROM]: https://doc.rust-lang.org/std/convert/trait.TryFrom.html "Rust standard library: TryFrom"
[RUST-U32]: https://doc.rust-lang.org/std/primitive.u32.html "Rust standard library: integer arithmetic policies"
[SCOPEGUARD]: https://docs.rs/scopeguard/latest/scopeguard/ "scopeguard crate documentation"
[SWIFT-ACTORS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0306-actors.md "Swift Evolution SE-0306: actors"
[SWIFT-CONCURRENCY]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/Concurrency.md "The Swift Programming Language: concurrency"
[SWIFT-CONTROL]: https://raw.githubusercontent.com/swiftlang/swift-book/main/TSPL.docc/LanguageGuide/ControlFlow.md "The Swift Programming Language: control flow"
[SWIFT-SENDABLE]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0302-concurrent-value-and-concurrent-closures.md "Swift Evolution SE-0302: Sendable"
[SWIFT-THROWS]: https://raw.githubusercontent.com/swiftlang/swift-evolution/main/proposals/0413-typed-throws.md "Swift Evolution SE-0413: typed throws"
[TOKIO-CHANNELS]: https://tokio.rs/tokio/tutorial/channels "Tokio tutorial: channels"
[TOKIO-SELECT]: https://docs.rs/tokio/latest/tokio/macro.select.html "Tokio: select and cancellation safety"
[V-OPTIONS]: https://docs.vlang.io/type-declarations.html "V documentation: options, results, sum types, and generics"
[ZIG]: https://ziglang.org/documentation/master/ "Zig language reference, master snapshot"
