# Validation record

**Generated and checked:** 2026-10-13.

## Document checks

- 100 unique technique identifiers: 26 memory, 24 control flow, 26 polymorphism, and 24 data-modeling entries.
- Exactly one occurrence of each of the four required analytical fields in every technique.
- All 188 used source-reference labels resolve to registered links.
- All local Markdown file links and explicit-anchor links resolve within the package.
- Fenced code blocks are balanced.
- The complete and modular editions are assembled from the same substantive text.
- No unresolved placeholders or tool-only citation tokens remain in the Markdown.

The reading kernel and eight substantive modules contain approximately **26,999 words**, including tables and code tokens, excluding the generated index and source registry. Word counts depend on tokenizer conventions.

## Executed examples

### C++23

```text
g++ (Debian 14.2.0-19) 14.2.0
x86_64

Command: g++ -std=c++23 -O2 -Wall -Wextra -Wpedantic -Werror examples/packet_view.cpp
Behavioral checks: passed
sizeof(void*)=8
sizeof(span<const byte>)=16
sizeof(span<const byte,16>)=8
sizeof(unique_ptr<int>)=8
sizeof(shared_ptr<int>)=16
sizeof(optional<int>)=8
sizeof(Frame)=24
sizeof(expected<Frame,ParseError>)=32
```

### Go

```text
go version go1.23.2 linux/amd64

Command: go run examples/go_representation.go
pointer is nil: true
interface is nil: false
slice descriptor bytes: 24
write through slice changes backing value: 99
```

## Limits

No performance benchmark, assembly-equivalence proof, sanitizer run, cross-target ABI test, or all-language compilation test was performed. Rust/device protocol fragments are signatures or pseudocode, not validated executable implementations. Primary-source URLs were researched, but this artifact's structural validator checks reference resolution rather than continuously monitoring external link availability. Source documents are live unless their URLs identify fixed versions.
