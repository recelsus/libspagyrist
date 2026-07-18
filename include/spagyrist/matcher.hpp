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
    bool reject_scattered_long_matches{false};
};

struct match_result {
    bool matched{};
    double score{};
    // Byte offsets in the candidate string. The current matcher is greedy and byte-based.
    std::vector<std::size_t> positions;
};

// Greedy subsequence matcher. This is not fzy-compatible and does not search
// for the globally optimal alignment when repeated characters exist.
[[nodiscard]] match_result
fuzzy_match(
    std::string_view query,
    std::string_view candidate,
    const match_options& options = {});

} // namespace spagyrist
