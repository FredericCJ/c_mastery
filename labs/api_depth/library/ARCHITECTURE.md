# Course library decisions

The installed `lc.h` owns operation requirements; generated references reproduce
that header. This note owns design alternatives and does not duplicate numeric
admission limits. The original `ApiBuffer`/`api_append` implementation remains the
mechanism producer. The packaging adapter owns opaque allocation, ABI admission
and release identity. `ApiBuffer` is private: exposing it would couple consumers
to both its layout and its capacity. The alternative inline public representation
would remove one allocation but impose that compatibility obligation.

`lc_view` borrows; `lc_copy` spends O(length) byte stores to decouple the caller's
lifetime. Python must retain the original opaque address for library release;
converting an owned pointer directly to Python bytes does not release it. Native
argument names document units; `LCBytes` and `LCElements` add incompatible struct
types, but explicit casts or an incorrectly constructed wrapper bypass intent.

The host and teaching register-model backends have separate state and different
operations. Their operation table does not mean every slot has the same behavior.
Replacing the host implementation with the model changes the selected result
rule and this decision record, not `lc_append`'s installed contract. The model
is not peripheral execution. No vendor header enters the public ABI.
The CMake `LC_BACKEND` choice selects HOST, MODEL, BOTH or NONE explicitly;
the auxiliary investigation target is distinct from the installed byte library.
Its compilation database is evidence of selected source dependencies, while the
two-context lifecycle test is separate execution evidence.

Constructor stages acquire resources1,2(optional),3 and publish only after all
selected acquisitions. Reverse release is contractual in this example. Nested,
linear and helper-based implementations are compared by complete event traces.
Moving predicates into helpers lowers a local branch count while retaining the
same acquired-prefix states and callback obligations. Metric reports name this
limited token-count definition; they are not maintainability scores.

Synchronous completion permits context reclamation after return. Deferred
completion retains it through poll or cancellation acknowledgment. The transition
to terminal state occurs before invoking the callback, but reentry is prohibited;
the operation itself remains owned until explicit release. Requests alone cannot
authorize context destruction. Serialized scheduling here is not a race detector.

Consumers use `find_package(Course 1 CONFIG)` and `Course::course`. Build and install
interfaces differ; neither a generated private header nor source-tree include
path belongs in the installed interface. Static consumers require all referenced
objects, including the optional private helper; shared deployment additionally
needs runtime lookup. The installed ABI version remains1 across the two semantic
releases. Release notes describe changed admission independently of ABI layout.
