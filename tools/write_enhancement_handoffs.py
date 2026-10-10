"""Add bounded extension handoffs while preserving original lesson/project work."""
from common import ROOT, write

HANDOFFS = {
    'T08': 'Transfer the same object/disassembly/ABI dossier to {ref}`T36` handwritten kernels and {ref}`T37` instruction decoding. Identify the chosen machine ABI before comparing compiler output with original assembly.',
    'T14': 'Carry the existing typed container, ownership and invalidation contracts into {ref}`T36`. Derive the new invariant or representation alternative before changing the implementation; retain the original project evidence rather than rebuilding every container.',
    'T15': 'Extend the existing algorithm implementations in {ref}`T36` with first-principles certificates, handwritten assembly kernels and higher abstractions. Reuse parser/queue/timer mechanisms in {ref}`T40`–{ref}`T42` while adding protocol-specific state and resource obligations.',
    'T18': 'Use these ownership and compatibility contracts at the assembly/C boundary in {ref}`T36` and at every communication service in {ref}`T38`–{ref}`T42`. Specify whether an adjacent layer consumes, borrows or retains a span before implementing its adapter.',
    'T24': 'Reuse the event-loop, descriptor, backpressure and shutdown adapters in {ref}`T42`. The added HTTP work owns message grammar, framing, response order and deadlines. OS sockets continue to own the lower network stack for that family.',
    'T27': 'Use {ref}`T37` to deepen ISA, clocked-state, bus, memory and exception reasoning. Carry the actual startup/linker/map evidence into {ref}`T38`–{ref}`T41`; a host model cannot replace board reachability or physical observations.',
    'T28': 'Carry the MMIO and completion contracts into {ref}`T38` serial interfaces and {ref}`T39` Ethernet. Distinguish peripheral register behavior, electrical conversion, shared-medium ownership and higher protocol semantics before joining layers.',
    'T29': 'Transfer ISR/DMA ownership and ordering evidence to {ref}`T39` and {ref}`T41`. The selected F439 Ethernet descriptors and buffers must use DMA-reachable memory; publish complete descriptor/buffer state before OWN and inspect completion before reuse.',
    'T32': 'Apply the existing length, arithmetic, ownership and failure reasoning to {ref}`T40`–{ref}`T42`. Preserve malformed-input and resource-exhaustion cases at each layer; CRC success and a cooperative peer do not establish authentication or general protocol conformance.',
}

def main():
    marker='<!-- enhancement handoff -->'
    for key,paragraph in HANDOFFS.items():
        path=ROOT/f'curriculum/modules/{key}.md'
        text=path.read_text(encoding='utf-8')
        if marker in text:
            before,rest=text.split(marker,1)
            text=before+rest[rest.index('## Environment and limits'):]
        text=text.replace('## Environment and limits',marker+'\n\n'+paragraph+'\n\n## Environment and limits',1)
        write(path,text)
    print('Added nine bounded handoffs; original project briefs and taxonomy are unchanged.')

if __name__=='__main__': main()
