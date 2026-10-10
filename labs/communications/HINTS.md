# Staged hints

Read one row at a time. Predict the result and draw ownership before revealing
the discriminating experiment.

| Boundary | First hint | If still stuck | Discriminating drill |
|---|---|---|---|
| HTTP line | Count octets rather than C-string length | CR is a pending state, not a complete line | Split every CR/LF pair and put NUL in the body |
| Framing | Derive one body owner before consuming any body | TE and CL are competing boundaries | Append a second request after a rejected ambiguous first request |
| Chunk | Separate hexadecimal size, extensions, data and trailing CRLF | A zero size still has a trailer section | Omit only the last LF and signal EOF |
| Continue | Header completion can publish an interim response | Do not wait for body before100 | A raw client waits for100 before sending its first body byte |
| Output | Offset is a count of locally accepted bytes | EINTR and EAGAIN preserve it | Return2, EINTR, EAGAIN, then1 repeatedly |
| Deadline | Total age differs from last-progress age | Progress cannot renew the total deadline | Deliver one byte just before every idle deadline |
| Link | Validate the complete unescaped body before publication | CRC covers the header and payload | Flip one source-address bit without fixing CRC |
| Router | Three namespaces have different lifetimes | Preserve node IDs but replace link IDs | Poison the borrowed ingress frame after return |
| Transport | ACK identity and credit are separate facts | Final duplicate needs a retained receipt | Drop the final ACK and retransmit identical bytes |
| Session | Token transfer can be pending | Losing completion must not create two senders | Try DATA before token completion and replay a checkpoint with changed bytes |
| Presentation | Check all field lengths before publication | Signed wire integers do not use host struct layout | Encode a negative temperature and overlong UTF-8 |
| Network | Ethernet padding ends outside IP total length | Checksum pseudo-header includes protocol and length | Change pad bytes, then a TCP byte |
| TCP | Keep sequence intervals below half the number space | Unsigned wrap is not ordinary less-than | Start an exchange ten bytes before2^32 and duplicate a prefix |
| RTO | Update variance from the old mean | Retransmission destroys this RTT sample | Acknowledge a retransmitted SYN and inspect the next data RTO |
| Reset | Read status after requesting mode | Request bits are not operation status | Hold OPMOD unchanged until the deadline |
| DMA | CPU and device cannot own writable bytes together | Publish OWN only after the declared barrier | Attempt preparation while OWN remains set |

Hardware recovery starts with exact board/part/clock, physical measurement and a
smallest failing transaction. The generic models cannot reveal wrong voltage,
wiring, termination, pin routing, controller clock or DMA memory placement.
