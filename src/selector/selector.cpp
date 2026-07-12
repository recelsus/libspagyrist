#include "spagyrist/selector.hpp"

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

} // namespace spagyrist
