#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

namespace {

class fixed_selector final : public spagyrist::selector {
public:
    explicit fixed_selector(std::optional<std::size_t> index)
        : index_(index)
    {
    }

    std::optional<std::size_t>
    select(std::span<const spagyrist::candidate> candidates) override
    {
        observed_size = candidates.size();
        return index_;
    }

    std::size_t observed_size{};

private:
    std::optional<std::size_t> index_;
};

std::vector<spagyrist::candidate> candidates()
{
    spagyrist::candidate linux_candidate;
    linux_candidate.id = "Linux";
    linux_candidate.title = "Linux";

    spagyrist::candidate kernel;
    kernel.id = "Linux kernel";
    kernel.title = "Linux kernel";

    return {linux_candidate, kernel};
}

void selector_result_maps_index_to_candidate()
{
    auto values = candidates();
    fixed_selector selector{1};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 1);
    SPAGYRIST_CHECK(selected->item.id == "Linux kernel");
    SPAGYRIST_CHECK(selector.observed_size == values.size());
}

void selector_cancel_returns_empty_selection()
{
    auto values = candidates();
    fixed_selector selector{std::nullopt};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(!selected.has_value());
}

void selector_out_of_range_returns_empty_selection()
{
    auto values = candidates();
    fixed_selector selector{9};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(!selected.has_value());
}

} // namespace

void run_selector_tests()
{
    selector_result_maps_index_to_candidate();
    selector_cancel_returns_empty_selection();
    selector_out_of_range_returns_empty_selection();
}
