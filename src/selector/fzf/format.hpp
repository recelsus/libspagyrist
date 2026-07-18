#pragma once

#include "spagyrist/candidate.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spagyrist::detail::fzf {

[[nodiscard]] std::vector<std::string> default_arguments();
[[nodiscard]] bool has_preview(std::span<const candidate> candidates);
[[nodiscard]] bool has_preview_argument(const std::vector<std::string>& arguments);
[[nodiscard]] std::string input_for_candidates(std::span<const candidate> candidates);
[[nodiscard]] std::optional<std::size_t> parse_selected_index(std::string_view output);

} // namespace spagyrist::detail::fzf
