# Registry and generic dispatch key

T06.06.a: canonical ordering is by ID while ID/name/handler remain one row. Aliases add reverse names only; forward ID lookup remains canonical. Duplicate IDs are never the alias syntax. The sorted baseline is1/start,4/stop,7/status with input10 results11/14/17.

T06.06.b: generation covers every view plus manifest. Clean reordering yields identical bytes because both canonical rows and aliases have explicit order. A stale hand-edited name can compile; the independent full-name comparison and regeneration check catch it. Declared build dependencies include the source registry and the generator itself.

T06.06.c: validate all names,IDs,handlers and counts before writes. Target lookup validates input then uses lower-bound binary search; a sparse maximum ID does not become an array offset. Three command entries differ radically from2147483648 pointer slots. Include actual pointer width and metadata overhead in costs; no universal lookup-speed theorem follows.

T06.07.a: unary plus imposes arithmetic conversion before generic selection,which avoids relying on implicit qualifier treatment alone. Typedef int is compatible with int; const/volatile lvalues become arithmetic values; arrays do not satisfy unary-plus constraints. Explicit element selection is a different admitted expression. Address selection instead preserves qualifier and array extent through pointer types and requires a live addressable object.

T06.07.b: generic controlling expressions are unevaluated; the selected function receives its ordinary argument once. With -fno-inline, the named wrapper paths contain direct/unconditional transitions to the same typed function. GCC can still fold identical wrapper bodies and route one wrapper through another; this is not a runtime generic branch. Inspect the entire bounded alias path,not merely whether the typed callee name occurs in the first block.

T06.07.c: an invalid member expression is semantically checked even when its association is unselected. Valid function designators in every association repair the expression problem. A distinct long association extends the accepted interface; an int typedef association duplicates a compatible type and must be rejected.

The strict image observes only the selected x86-64 component and original Linux-exit harness. A different data model,ABI,FPU or target may introduce helpers or change numeric observations and requires a fresh audit. Source drafts,actual diagnostics and independent assessment are separate evidence dimensions.
