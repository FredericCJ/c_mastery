# Consume index assessor analysis

For the admitted immutable execution, all five function results are fully
defined; tests cannot distinguish ordering strength by observing their values.
The direct access has the pointer as the member-address operand. The value-index
case extends that chain through the node field load, local scalar copy and
array subscript. The null test precedes that use but does not replace its value.
The branch case uses the loaded pointer only to choose one of two constant
assignments. Neither constant value is computed from the loaded pointer as an
operand. The killed case has a chain to the argument but not through the macro's
result. Its identical numeric output is the required counterexample to a
value-only test of synchronization.

In the delayed writer, assume no other synchronization: writer initializes
values[1], initializes a node, then release-stores its address. Reader's consume
load reads that store and chooses constant index1. There is no dependency chain
to values[1], so the ordinary writer/read pair lacks the required ordering; a
portable data-race-free proof fails. Do not assign a golden stale-value output
or execute that flawed ordinary-memory program. The acquire repair needs the
matching release read; the data-index repair additionally needs node.index=1
and the full pointer/field/scalar/subscript chain. Neither grants reclamation.

The graph oracle uses recursive path enumeration independently of the C forward
propagation loop. It proves only equality on the729 supplied finite DAGs; the
graph labels themselves must be justified from the C source. Mutations replace
the value-derived subscript with a constant, invert the branch-selected index,
or admit control edges as dependencies. Each is compiled and rejected separately.
The LLVM/GCC strengthening observed in a selected object is not substituted for
the source-language argument. A target object without helpers is not a target
execution, latency measurement or interrupt-safety result.

The ordinary exact-final-edition comparison and LRUN-16 source closure remain
partial until the actual inaccessible texts can be inspected. This companion
completes the previously missing changed source specimen and its bounded author
analysis; no historical learner acceptance or reviewer independence is altered.
