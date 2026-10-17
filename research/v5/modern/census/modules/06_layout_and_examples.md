# VI. Layout models and worked examples

## A. Representation models: guarantees versus useful approximations

For conventional inline storage, a useful first approximation for a sum is the size of its largest alternative plus discriminator and padding. This is not a universal formula: niches can remove a separate discriminator, over-alignment changes padding, and implementation strategies vary. For a product, every field needs representation unless an applicable optimization or language rule permits elision. [RUST-LAYOUT] [CPP-VARIANT]

```text
Illustrative contiguous view:
    descriptor -> first element
    length     = number of accessible elements
    owner      = elsewhere; not encoded by the descriptor alone

Illustrative erased borrowed interface:
    data path       -> concrete object
    operation table -> operations for its hidden concrete type
    lifetime        = a separate contract or static relationship

Illustrative owning resizable sequence:
    data pointer + initialized length + allocation capacity
    element storage and allocator metadata are additional memory

Illustrative closed sum:
    active alternative + suitably aligned alternative storage
    the tag may instead be encoded in an unused payload representation
```

The word “fat pointer” describes pointer-plus-metadata representations, not a single cross-language ABI. Slice length and trait-object dispatch metadata serve different purposes. Go slices additionally expose capacity. Rust's reference documentation distinguishes guaranteed layout facts from current observations about dynamically sized pointees. [GO-SLICES] [RUST-LAYOUT]

An optimized `Option` can still use a sentinel physically: the improvement is that absence is explicit in the type and ordinary clients cannot confuse it with a valid payload. Rust documents no-extra-storage cases for selected payload types. General C++ `optional<T>` and arbitrary Rust `Option<T>` must not be assigned the underlying type's size without checking their specific contracts. [RUST-OPTION] [CPP-OPTIONAL]

## B. C++23: parse bytes into a borrowed algebraic value

The example below uses a deliberately small protocol: one kind byte, a big-endian two-byte payload length, then exactly that many payload bytes. Kind `1` carries data; kind `2` is an acknowledgment with no payload. This is an illustrative protocol designed for the example, not an existing transport specification.

The parser combines M05, C02, D02, D06, and D20. It checks envelope structure before constructing a stronger representation. It performs no payload copy, and its result types require no dynamic storage for these particular alternatives. It does not validate any application-specific meaning inside the payload. Its returned span still borrows the caller's bytes; C++ does not verify that relationship at every later use. [CPP-SPAN] [CPP-EXPECTED] [CPP-VARIANT]

```cpp
/** @file packet_view.cpp
 *  @brief Checked decoding into a borrowed sum type; C++23 example.
 *  @details The frame does not own its input. Its backing storage must outlive
 *           every use of the returned view and remain valid and suitably stable.
 */
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <variant>

/** @brief Recoverable protocol-validation failures. */
enum class ParseError { truncated_header, unknown_kind, length_mismatch,
                        acknowledgment_has_payload };
/** @brief Borrowed bytes from a validated data-frame envelope. */
struct DataFrame { std::span<const std::byte> payload; };
/** @brief An acknowledgment has no payload in this example protocol. */
struct Acknowledgment {};
/** @brief The two admissible frame alternatives. */
using Frame = std::variant<DataFrame, Acknowledgment>;

/** @brief Parse [kind:u8][length:u16 big-endian][payload:length bytes].
 *  @param input A readable byte range; returned data views borrow this range.
 *  @return One frame, or an explicit validation failure.
 *  @details Exactly one complete frame is accepted; trailing data is rejected.
 */
[[nodiscard]] std::expected<Frame, ParseError>
parse_frame(std::span<const std::byte> input) noexcept {
    constexpr std::size_t header_size = 3;
    if (input.size() < header_size) {
        return std::unexpected(ParseError::truncated_header);
    }
    const unsigned int kind = std::to_integer<unsigned int>(input[0]);
    if (kind != 1U && kind != 2U) {
        return std::unexpected(ParseError::unknown_kind);
    }
    const std::size_t length =
        (std::to_integer<std::size_t>(input[1]) << 8U) |
         std::to_integer<std::size_t>(input[2]);
    if (length != input.size() - header_size) {
        return std::unexpected(ParseError::length_mismatch);
    }
    if (kind == 2U) {
        if (length != 0U) {
            return std::unexpected(ParseError::acknowledgment_has_payload);
        }
        return Frame{Acknowledgment{}};
    }
    return Frame{DataFrame{input.subspan(header_size, length)}};
}

/** @brief Run focused behavioral checks and print local ABI observations. */
int main() {
    constexpr std::array data{std::byte{1}, std::byte{0}, std::byte{2},
                              std::byte{0xAA}, std::byte{0xBB}};
    constexpr std::array ack{std::byte{2}, std::byte{0}, std::byte{0}};
    constexpr std::array unknown{std::byte{3}, std::byte{0}, std::byte{0}};
    constexpr std::array bad_length{std::byte{1}, std::byte{0}, std::byte{2}};
    constexpr std::array bad_ack{std::byte{2}, std::byte{0}, std::byte{1},
                                std::byte{0}};
    const auto decoded = parse_frame(data);
    assert(decoded.has_value());
    assert(std::get<DataFrame>(*decoded).payload.data() == data.data() + 3);
    assert(std::get<DataFrame>(*decoded).payload.size() == 2);
    assert(std::holds_alternative<Acknowledgment>(*parse_frame(ack)));
    assert(parse_frame({}).error() == ParseError::truncated_header);
    assert(parse_frame(unknown).error() == ParseError::unknown_kind);
    assert(parse_frame(bad_length).error() == ParseError::length_mismatch);
    assert(parse_frame(bad_ack).error() == ParseError::acknowledgment_has_payload);
    std::cout << "Behavioral checks: passed\n"
              << "sizeof(void*)=" << sizeof(void*) << '\n'
              << "sizeof(span<const byte>)=" << sizeof(std::span<const std::byte>) << '\n'
              << "sizeof(span<const byte,16>)=" << sizeof(std::span<const std::byte,16>) << '\n'
              << "sizeof(unique_ptr<int>)=" << sizeof(std::unique_ptr<int>) << '\n'
              << "sizeof(shared_ptr<int>)=" << sizeof(std::shared_ptr<int>) << '\n'
              << "sizeof(optional<int>)=" << sizeof(std::optional<int>) << '\n'
              << "sizeof(Frame)=" << sizeof(Frame) << '\n'
              << "sizeof(expected<Frame,ParseError>)="
              << sizeof(std::expected<Frame, ParseError>) << '\n';
}
```

The length check subtracts the already-validated header size before comparison rather than adding an untrusted length to a pointer. Envelope validation rejects trailing bytes, making the contract “one exact frame” rather than silently accepting an arbitrary prefix. The `Acknowledgment` alternative cannot carry bytes, so that invariant need not be rechecked by every consumer.

Local compilation and execution produced:

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

These sizes apply only to the compiler, standard library, target, and options shown. `sizeof(shared_ptr<int>)` excludes the managed allocation and control block. `sizeof(span)` excludes the viewed bytes. `sizeof(Frame)` includes space for the largest alternative and implementation bookkeeping. The test establishes the listed behavior and local observations; it does not establish equivalent assembly to a hand-written baseline, sanitizer coverage, all edge cases, or behavior on an embedded ABI.

## C. Go: a descriptor is not ownership, and typed nil is not a nil interface

The next program demonstrates two distinct issues: a slice shares backing storage, and an interface containing a typed nil pointer still has a dynamic type. The use of `unsafe.Sizeof` is observational; the code does not reinterpret or dereference invalid memory. [GO-SLICES] [GO-FAQ]

```go
// Package main demonstrates descriptor size and typed-nil interface semantics.
package main

import (
    "fmt"
    "unsafe"
)

// Fault is an example concrete error type with a pointer receiver.
type Fault struct{}

// Error implements error, including a receiver that may be nil.
func (*Fault) Error() string { return "example fault" }

func main() {
    var ptr *Fault
    var err error = ptr
    var values []byte
    fmt.Printf("pointer is nil: %t\n", ptr == nil)
    fmt.Printf("interface is nil: %t\n", err == nil)
    fmt.Printf("slice descriptor bytes: %d\n", unsafe.Sizeof(values))
    backing := []int{10, 20, 30}
    view := backing[:2]
    view[0] = 99
    if backing[0] != 99 || err == nil {
        panic("representation check failed")
    }
    fmt.Printf("write through slice changes backing value: %d\n", backing[0])
}
```

Local compilation and execution produced:

```text
go version go1.23.2 linux/amd64

Command: go run examples/go_representation.go
pointer is nil: true
interface is nil: false
slice descriptor bytes: 24
write through slice changes backing value: 99
```

The 24-byte slice descriptor excludes backing-array storage and collector metadata. Slice assignment copies a descriptor, not an independent element array. Consequently a “value was copied” statement must identify whether it means the descriptor, owned contents, or the entire reachable object graph.

## D. Typestate for an external device: model uncertainty instead of hiding it

The following is type-signature pseudocode, not executable code:

```text
configure:
    Unconfigured × ValidatedConfiguration
    -> Result<Idle, (Unconfigured, ConfigurationError)>

start:
    Idle
    -> Result<Starting, (Idle, RejectedBeforeActivation)>

observe_start:
    Starting × HardwareObservation
    -> Running | Idle | RecoveryRequired

request_stop:
    Running
    -> Stopping

observe_stop:
    Stopping × HardwareObservation
    -> Idle | RecoveryRequired
```

The important design decision is not the spelling of the wrappers. An error that occurs before issuing an activation command can safely return `Idle`; an ambiguous observation after activation cannot. A `RecoveryRequired` state prevents ordinary operations until recovery resolves the uncertainty. This is architectural reasoning about the example, not a claim that a type checker independently proves hardware state.

A borrowed DMA buffer adds another dimension: it must remain allocated, sufficiently aligned, mapped, and unavailable for conflicting CPU access for the complete transfer interval. CPU-language pinning addresses selected movement invariants; it does not configure an IOMMU, maintain cache coherency, or lock physical pages. The low-level driver contract must establish those environmental obligations. [RUST-PIN]

## E. Closed sums versus open implementations

Consider either a `variant<Circle, Box>` processed by visitation or a borrowed `dyn Shape`/existential interface. A sum makes the alternative set explicit and can store values inline; an existential makes the operation set explicit and can accept implementations not known by the consumer. Allocation depends on how the values are owned, not only on which dispatch model is chosen. [CPP-VARIANT] [CPP-VARIANT-VISIT] [RUST-DYN]

A practical split is to keep a small closed representation in a hot data-processing core, while using a dynamic interface at a plugin or backend boundary. Converting once at the boundary can be less expensive and easier to verify than carrying erased polymorphism through every inner-loop operation. Conversely, generating a specialized inner loop for every combination of policy types can worsen instruction-cache behavior. Measure both code size and hot-path execution rather than selecting dispatch based on terminology alone.

<!-- REFERENCE DEFINITIONS -->
[CPP-EXPECTED]: https://eel.is/c++draft/expected "C++ working draft: expected"
[CPP-OPTIONAL]: https://eel.is/c++draft/optional "C++ working draft: optional"
[CPP-SPAN]: https://eel.is/c++draft/views.span "C++ working draft: span"
[CPP-VARIANT]: https://eel.is/c++draft/variant "C++ working draft: variant"
[CPP-VARIANT-VISIT]: https://eel.is/c++draft/variant.visit "C++ working draft: variant visitation"
[GO-FAQ]: https://go.dev/doc/faq "Go frequently asked questions"
[GO-SLICES]: https://go.dev/blog/slices-intro "Go slices: usage and internals"
[RUST-DYN]: https://doc.rust-lang.org/reference/types/trait-object.html "Rust Reference: trait objects"
[RUST-LAYOUT]: https://doc.rust-lang.org/reference/type-layout.html "Rust Reference: type layout"
[RUST-OPTION]: https://doc.rust-lang.org/std/option/index.html "Rust standard library: Option and representation guarantees"
[RUST-PIN]: https://doc.rust-lang.org/std/pin/index.html "Rust standard library: pinning"
