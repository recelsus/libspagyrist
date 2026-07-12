#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

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

void number_selector_maps_number_to_index()
{
    auto values = candidates();
    std::istringstream input{"2\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 1);
    SPAGYRIST_CHECK(selected->item.title == "Linux kernel");
    SPAGYRIST_CHECK(output.str().find("1. Linux") != std::string::npos);
}

void number_selector_empty_input_cancels()
{
    auto values = candidates();
    std::istringstream input{"\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(!selected.has_value());
}

void fzf_selector_uses_external_process_even_for_one_candidate()
{
    const auto script = std::filesystem::temp_directory_path() / "spagyrist-fake-fzf.sh";
    {
        std::ofstream file(script);
        file << "#!/bin/sh\n";
        file << "cat >/tmp/spagyrist-fake-fzf-input\n";
        file << "head -n 1 /tmp/spagyrist-fake-fzf-input\n";
    }
    chmod(script.c_str(), 0700);

    std::vector<spagyrist::candidate> values;
    spagyrist::candidate only;
    only.id = "Only";
    only.title = "Only";
    values.push_back(only);

    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};
    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 0);
    std::filesystem::remove(script);
    std::filesystem::remove("/tmp/spagyrist-fake-fzf-input");
}

} // namespace

void run_selector_tests()
{
    selector_result_maps_index_to_candidate();
    selector_cancel_returns_empty_selection();
    selector_out_of_range_returns_empty_selection();
    number_selector_maps_number_to_index();
    number_selector_empty_input_cancels();
    fzf_selector_uses_external_process_even_for_one_candidate();
}
