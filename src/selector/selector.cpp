#include "spagyrist/selector.hpp"

#include "spagyrist/selector/fzf.hpp"

#include <stdexcept>

namespace spagyrist {

std::optional<selection>
select_candidate(selector& selector, std::span<const candidate> candidates)
{
    const auto selected_index = selector.select(candidates);
    if (!selected_index || *selected_index >= candidates.size()) {
        return std::nullopt;
    }

    return selection{
        .index = *selected_index,
        .item = candidates[*selected_index],
    };
}

std::optional<selection>
select_candidate_with_fallback(
    selector& primary,
    selector& fallback,
    std::span<const candidate> candidates)
{
    if (auto* fzf = dynamic_cast<fzf_selector*>(&primary); fzf != nullptr && !fzf->is_available()) {
        return select_candidate(fallback, candidates);
    }

    try {
        return select_candidate(primary, candidates);
    } catch (const std::exception&) {
        return select_candidate(fallback, candidates);
    }
}

} // namespace spagyrist
