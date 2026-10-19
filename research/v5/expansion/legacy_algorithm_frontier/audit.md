# PR-T36-07 frontier composition audit

Read the original project, exact T36.09.a/b/c criteria and original V1 tasks,
fixture, actual T15 graph_dp.c graph validation/BFS/Dijkstra functions, and the
actual T14 typed CSR/ring/heap operations and public contracts. The prior T15
uses an array queue and scan minimum; the older matrix16/UINT32_MAX baseline is
not the requested sparse8/24/65534 adapter. Neither name alone proves composition.

The new generator extracts the exact actual validator, empty traversal, BFS and
Dijkstra bodies and makes explicit single-occurrence substitutions at queue,
selection, decrease-key and trace boundaries. It preserves the original discovery
and relaxation logic. Generated provenance hashes each selected original body;
the unchanged originals are also linked and compared. T14 CDRing, CDHeap and
CDCSR implementations are called directly, without rewriting or changing them.
Distance×8+vertex adapts the prior insertion-order heap ties to the required
vertex-ID tie rule; its maximum524279 is checked against the selected int model.

Actual frontier snapshots, changes, result paths and costs are independently
checked against simple paths and replay. Parallel edges retain CSR source order,
receive distinct relaxation identities, and update one owned heap item per vertex.
Range failure preserves public result/trace while caller workspace stays scratch.
The signed-negative changed task has an executed independent certificate policy
and bounded path/counterexample investigation; it is not mislabeled a full new
negative-edge target solver. The original task asks for revised witness/arithmetic
derivation, which remains a separate learner explanation obligation.

Actually inspected all three scanned primary pages269–271 of Dijkstra1959 via
extracted images. Web PDF text and local pypdf extraction were empty; unavailable
WSL pdftotext/pymupdf and Windows pymupdf did not establish reading. Pillow was
provisioned privately under ignored build/source-render-tools solely to extract
the scanned page images, which were then viewed. Page269 states the connected
presentation and Problem1; page270 has Problem2's settled/frontier partition,
relaxation and minimum selection plus directed-length remark; page271 completes
the stored-distance distinction. No referenced Ford/Berge/Kruskal work was read
from those bibliography entries. The original source's comparison of work is not
promoted to a modern benchmark or complexity result.

Final C17/C23 published text remains uninspected. The current implementation
baseline remains C17. External review, delayed independent learner work, physical
observation, PR-T36-08 and T37 are separate required dimensions/obligations.
