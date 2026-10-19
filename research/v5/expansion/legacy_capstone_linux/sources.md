# Linux capstone source and depth audit

All four original Linux cards and project contracts were read against the actual
P3 parser, P5 worker, ownership, result and COAsync boundaries. The old integration
anchor combines a VM with a checked page; it does not implement this service.
The capstone's original concrete tasks therefore require a new service adapter.

The new portable component restores a temporary P3 parser from a retained candidate
and copies frames into separately owned queues. This keeps the stated252 bytes of
persistent wire backing while explicitly accounting for temporary parser/message
stack and metadata. The inspected P3 type contains both candidate and decoded
message storage; blindly embedding two full parsers would change that accounting.

Actual P5Bridge has32-byte capture, cancellation and a distinct join, so42-byte
frames use bounded chunks. Actual COAsync accepts at most8 bytes and requires
synchronous disarm. Those contracts cannot be interchanged by renaming an API.
Actual OwScope/OwShared and er_transfer bodies were read for their cleanup,
refcount and zero-progress restrictions. The adapter uses those exact producers;
it does not claim C enforces their state or ownership rules.

The primary readings are the registered epoll(7) LT/ET and descriptor identity
sections, epoll_ctl(2) registration/event/error sections, and write(2) prefix,
error and durability notes. They were read directly before capture. Captures are
private build artifacts; no copied full primary text is published here. Source
inspection is bounded to the exact locators, not whole-manual expertise.

Target execution uses the actual retained Cortex-M4 reset/vector and byte-helper
sources, inspected directly, with a new service driver and independent poison of
data/BSS before reset. This is MPS2 execution, not F439 UART/DMA or physical timing.
Kernel/library/compiler identity is recorded by the campaign. No host security
setting, external service, production device or real user data is involved.

The original24 task identities and delayed intervals are preserved. A source-bound
author run can support implementation and author-validation dimensions only.
Independent learner/review and physical observations remain distinct. The output
sink's durability row remains explicitly pending: the required exercise is to
identify suitable evidence and refuse a false substitution, not to relabel CRC
or file readback as a power-loss experiment.
