#include "spagyrist/ranking.hpp"

#include <algorithm>
#include <utility>

namespace spagyrist {

std::vector<ranked_candidate>
rank_candidates(
    std::string_view query,
    std::span<const candidate_text> candidates,
    const ranking_options& options)
{
    std::vector<ranked_candidate> output;
    output.reserve(candidates.size());

    for (const auto& candidate : candidates) {
        auto matched = fuzzy_match(query, candidate.search, options.matcher);
        if (!matched.matched) {
            continue;
        }
        auto display_matched = fuzzy_match(query, candidate.display, options.matcher);
        output.push_back(ranked_candidate{
            .index = candidate.index,
            .score = matched.score,
            .search_positions = std::move(matched.positions),
            .display_positions = display_matched.matched
                ? std::move(display_matched.positions)
                : std::vector<std::size_t>{},
        });
    }

    if (options.order == ranking_order::relevance) {
        std::stable_sort(
            output.begin(),
            output.end(),
            [](const ranked_candidate& lhs, const ranked_candidate& rhs) {
                if (lhs.score == rhs.score) {
                    return lhs.index < rhs.index;
                }
                return lhs.score > rhs.score;
            });
    }

    return output;
}

} // namespace spagyrist
