#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace spagyrist::detail {

// Checks byte-sequence shape only; this is not full Unicode scalar validation.
[[nodiscard]] bool is_structural_utf8_byte_sequence(std::string_view value) noexcept;
[[nodiscard]] std::size_t utf8_code_point_length(unsigned char lead) noexcept;
[[nodiscard]] std::size_t utf8_safe_prefix_size(std::string_view value, std::size_t max_bytes) noexcept;
void pop_back_utf8_code_point(std::string& value);

} // namespace spagyrist::detail
