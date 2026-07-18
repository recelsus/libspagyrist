#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

namespace spagyrist {

enum class match_case {
    sensitive,
    insensitive,
    smart,
};

struct match_options {
    match_case case_mode{match_case::smart};
};

struct match_result {
    bool matched{};
    double score{};
    std::vector<std::size_t> positions;
};

[[nodiscard]] match_result
fuzzy_match(
    std::string_view query,
    std::string_view candidate,
    const match_options& options = {});

} // namespace spagyrist
