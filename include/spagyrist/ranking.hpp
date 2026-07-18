#pragma once

#include "spagyrist/candidate_text.hpp"
#include "spagyrist/matcher.hpp"

#include <cstddef>
#include <span>
#include <string_view>
#include <vector>

namespace spagyrist {

enum class ranking_order {
    relevance,
    input,
};

struct ranking_options {
    ranking_order order{ranking_order::relevance};
    match_options matcher;
};

struct ranked_candidate {
    std::size_t index{};
    double score{};
    // Byte offsets matched in candidate_text::search.
    std::vector<std::size_t> search_positions;
    // Byte offsets matched in candidate_text::display.
    std::vector<std::size_t> display_positions;
};

[[nodiscard]] std::vector<ranked_candidate>
rank_candidates(
    std::string_view query,
    std::span<const candidate_text> candidates,
    const ranking_options& options = {});

} // namespace spagyrist
