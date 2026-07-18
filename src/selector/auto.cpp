#include "spagyrist/selector/auto.hpp"

namespace spagyrist {

auto_selector::auto_selector(auto_selector_options options)
    : options_(options)
{
}

bool auto_selector::is_available() const
{
    return true;
}

std::optional<std::size_t>
auto_selector::select(std::span<const candidate> candidates)
{
    const auto result = select_result(candidates);
    if (result.status == selector_status::selected) {
        return result.index;
    }
    return std::nullopt;
}

selector_result
auto_selector::select_result(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return selector_result::no_selection();
    }

    builtin_selector builtin{options_.builtin};
    auto result = builtin.select_result(candidates);
    if (result.status != selector_status::unavailable
        && result.status != selector_status::error) {
        return result;
    }

    number_selector number{options_.number};
    return number.select_result(candidates);
}

} // namespace spagyrist
