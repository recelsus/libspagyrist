#include "spagyrist/selector.hpp"

#include <stdexcept>
#include <utility>

namespace spagyrist {

selector_result selector_result::selected(std::size_t index)
{
    return selector_result{
        .status = selector_status::selected,
        .index = index,
        .message = {},
    };
}

selector_result selector_result::no_selection()
{
    return selector_result{
        .status = selector_status::no_selection,
        .index = std::nullopt,
        .message = {},
    };
}

selector_result selector_result::cancelled()
{
    return selector_result{
        .status = selector_status::cancelled,
        .index = std::nullopt,
        .message = {},
    };
}

selector_result selector_result::unavailable(std::string message)
{
    return selector_result{
        .status = selector_status::unavailable,
        .index = std::nullopt,
        .message = std::move(message),
    };
}

selector_result selector_result::error(std::string message)
{
    return selector_result{
        .status = selector_status::error,
        .index = std::nullopt,
        .message = std::move(message),
    };
}

selector_result selector_result::invalid_selection(std::size_t index)
{
    return selector_result{
        .status = selector_status::invalid_selection,
        .index = index,
        .message = {},
    };
}

bool selector::is_available() const
{
    return true;
}

selector_result
selector::select_result(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return selector_result::no_selection();
    }
    if (!is_available()) {
        return selector_result::unavailable();
    }

    try {
        const auto selected_index = select(candidates);
        if (!selected_index) {
            return selector_result::cancelled();
        }
        if (*selected_index >= candidates.size()) {
            return selector_result::invalid_selection(*selected_index);
        }
        return selector_result::selected(*selected_index);
    } catch (const std::exception& error) {
        return selector_result::error(error.what());
    } catch (...) {
        return selector_result::error("selector failed");
    }
}

candidate_selection_result
select_candidate_result(selector& selector, std::span<const candidate> candidates)
{
    const auto result = selector.select_result(candidates);
    if (result.status != selector_status::selected || !result.index) {
        return candidate_selection_result{
            .status = result.status,
            .selected = std::nullopt,
            .message = result.message,
        };
    }

    if (*result.index >= candidates.size()) {
        return candidate_selection_result{
            .status = selector_status::invalid_selection,
            .selected = std::nullopt,
            .message = "selector returned out-of-range index",
        };
    }

    return candidate_selection_result{
        .status = selector_status::selected,
        .selected = selection{
            .index = *result.index,
            .item = candidates[*result.index],
        },
        .message = result.message,
    };
}

std::optional<selection>
select_candidate(selector& selector, std::span<const candidate> candidates)
{
    return select_candidate_result(selector, candidates).selected;
}

candidate_selection_result
select_candidate_with_fallback_result(
    selector& primary,
    selector& fallback,
    std::span<const candidate> candidates)
{
    const auto primary_result = select_candidate_result(primary, candidates);
    if (primary_result.status == selector_status::unavailable
        || primary_result.status == selector_status::error) {
        return select_candidate_result(fallback, candidates);
    }

    return primary_result;
}

std::optional<selection>
select_candidate_with_fallback(
    selector& primary,
    selector& fallback,
    std::span<const candidate> candidates)
{
    return select_candidate_with_fallback_result(primary, fallback, candidates).selected;
}

} // namespace spagyrist
