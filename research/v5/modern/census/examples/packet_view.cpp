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
