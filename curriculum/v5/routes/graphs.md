# Graphs: dependency processing gives the proof a purpose

A build or compiler has dependencies before it has a graph algorithm. Define
vertices, edge direction, identity and whether duplicate edges are meaningful.
Select adjacency arrays, lists or bitmaps using mutation and traversal needs.
The representation's valid lifetime and index bounds remain C obligations.

PR-T15-05 begins with finite disconnected graphs and explicit work storage.
For BFS, discovery assigns a vertex once; prove that layer order gives shortest
unweighted distance. A valid returned path alone is not a shortest-path proof.
For iterative DFS, record whether a work item means entry, edge continuation or
exit. Replacing recursion with a stack without preserving that state changes
finish-time meaning and can break topological ordering.

The dependency consumer requires either a topological order or a cycle witness.
Check every edge against an accepted order independently. A cycle is a contract
result, not an unrelated crash. Weighted paths introduce nonnegative edge
assumptions and checked distance addition. Explain the settled-set invariant of
Dijkstra and exhibit a negative-edge counterexample even when no negative cycle
exists. Bellman-Ford has a different failure and cost contract.

Compiler analyses revisit graphs as worklists and fixed points. State the lattice,
monotonic transfer and termination bound; a traversal that visits each node once
is generally insufficient. The semantic signal graph changes identity and
invalidation obligations under incremental edits. Spatial scene graphs add
transform composition and cycle prevention, not shortest-path work.

Transfer supplies dependency updates and a requirement to identify all affected
outputs without prescribing DFS, a worklist or a persistent graph. Require a
correctness argument, rejected alternative, bounded resource policy and revised
choice after cycles become legal and fixed-point semantics replace rejection.
