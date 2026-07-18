#pragma once

#include "builtin_state.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace spagyrist::detail {

struct builtin_selector_view_options {
    std::size_t width{80};
    bool clear_screen{true};
    bool use_color{false};
};

[[nodiscard]] std::string render_builtin_selector_screen(
    const builtin_selector_state& state,
    const std::vector<candidate_text>& candidates,
    const builtin_selector_view_options& options = {});

} // namespace spagyrist::detail
