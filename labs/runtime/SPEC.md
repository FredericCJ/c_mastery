# CVM1: original educational contract

The wire image consists of ASCII `CVM1`, one function-count byte, that many six-byte descriptors, then the code region. A descriptor contains little-endian unsigned 16-bit entry offset, unsigned 16-bit byte length, unsigned 8-bit arity and unsigned 8-bit local-slot count. Functions partition the code region in descriptor order without gaps. Entry function 0 takes no arguments. There are 1–8 functions, 1–1024 code bytes, and 0–8 locals per function with arity at most local count. No host structure is serialized.

The raw `cvm_encode`/`cvm_decode` boundary checks container bounds and contiguous function regions, but does not inspect instructions or require zero entry arity. It can transport an invalid instruction stream. `cvm_save`/`cvm_load` additionally verify instructions and the zero-argument entry; `cvm_run` verifies again before dispatch. All codec/wrapper failures preserve destination bytes/program and written length. This separation permits codec work before verification exists.

| Opcode | Number | Following bytes | Stack action |
|---|---|---|---|
| INT | 1 | signed int32 as four little-endian two's-complement bytes | push integer |
| TRUE / FALSE / NIL | 2 / 3 / 4 | none | push value |
| ADD / SUB / MUL / DIV | 5 / 6 / 7 / 8 | none | two int32 operands become one checked int32 |
| EQ / LT | 9 / 10 | none | two values become Boolean; LT requires integers |
| LOAD / STORE | 11 / 12 | local slot byte | LOAD pushes initialized local; STORE pops into local |
| POP | 13 | none | discard top value |
| JMP / JFALSE | 14 / 15 | unsigned absolute code offset, two little-endian bytes | jump; conditional also pops a Boolean |
| CALL | 16 | function ID byte, argument-count byte | consume arguments, return one result |
| RET | 17 | none | return exactly one frame-relative operand |
| PAIR | 18 | none | consume left/right values and push an object reference |
| FIRST / SECOND | 19 / 20 | none | consume a valid pair reference and push its field |
| GC | 21 | none | collect without changing operand height |
| HOST | 22 | service byte | consume int32 and push returned int32, or fail |

Binary operands evaluate left first. Arithmetic rejects overflow, zero divisor and INT32_MIN/-1. Division truncates toward zero. EQ compares tag and value; references compare slot and generation. Reference equality does not dereference the object. All object access validates occupancy and generation. The language has no pointer arithmetic, user-selected native address or native function pointer.

The verifier decodes every instruction and checks descriptor bounds, opcodes, operands, local indices, direct-call arity, every branch target and function boundaries. A worklist assigns relative operand heights to reachable instructions, rejects inconsistent joins and underflow/overflow, and requires one value at RET. It rejects fallthrough past a function. It does not prove tags, initialized reads, termination, object liveness, total cross-frame stack space or callback correctness; execution checks those relevant dynamic conditions.

Runtime limits are 64 total operand slots, 16 live frames, 16 pair objects and 8 explicit roots. Frames own instruction position, stack base and initialized local state. Calls never recurse through the C interpreter. Generation overflow retires the slot. VM reset is a new lifetime and all old external references must be discarded; generation checks are scoped to one initialized VM lifetime.

Arena mode never reclaims individual objects. Collection mode marks the operand stack, initialized locals in live frames and explicit roots, then iteratively traces pair fields. The marked set prevents cycles from revisiting objects. Collection has a separate budget: each examined root or field costs one unit, and sweep costs 16. Sweep starts only after complete marking and sufficient sweep budget. Incomplete marking may change mark bits but frees nothing; the next collection clears marks before restarting. `cvm_pair` roots both C-temporary operands across collection. A host-held result needs an explicit root before a later allocating operation. The collector is nonmoving and stop-the-world.

Fuel decreases once per guest dispatch. Host budget decreases before each callback. Neither budget implies elapsed-time bounds. The callback must have an independent finite I/O/deadline policy and cannot mutate or reenter the VM. A failed run preserves diagnostic VM state but the output result is unchanged; initialize again for an independent replay. Loading/compilation failures preserve the output program. The program and byte image remain immutable throughout a run.

## Source language

The compiler accepts at most 4096 source bytes and 64 nested expression levels. Identifiers use ASCII letters/underscore followed by letters/digits/underscore, at most 31 bytes. Whitespace is space, tab, LF or CR; there are no comments or strings. Each source is one or more `(fn name (parameters...) expression)` definitions. The first is zero-argument `main`. Function definitions are unique; parameters within a function are unique. Function signatures are collected before bodies, so direct forward calls and recursion work. There are no closures or first-class functions.

Expressions are signed int32 literals, `true`, `false`, `nil`, a bound name, or these prefix forms:

```text
(+ a b) (- a b) (* a b) (/ a b) (== a b) (< a b)
(let name initializer body) (set name value)
(if boolean_condition yes no) (while boolean_condition body)
(do first second) (call function arguments...)
(pair left right) (first pair_value) (second pair_value)
(gc) (host service_integer argument)
```

The let initializer sees the surrounding scope; its body sees the new binding. Shadowing is permitted. Assignment requires an existing binding and returns the assigned value. A while body result is discarded and a normally terminating while returns nil. do discards its first result. if executes one branch. gc returns nil. Every expression leaves one value. Host services are an explicit embedding policy rather than a native FFI.

The source grammar, bytecode, capacities and checks are original course specifications. They are neither C conformance nor Lox compatibility, and do not establish a security sandbox for hostile native callbacks.
