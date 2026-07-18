#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
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

class unavailable_selector final : public spagyrist::selector {
public:
    bool is_available() const override
    {
        return false;
    }

    std::optional<std::size_t>
    select(std::span<const spagyrist::candidate>) override
    {
        selected = true;
        return 0;
    }

    bool selected{};
};

class throwing_selector final : public spagyrist::selector {
public:
    std::optional<std::size_t>
    select(std::span<const spagyrist::candidate>) override
    {
        throw std::runtime_error("selector failed for test");
    }
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

std::filesystem::path write_executable_script(std::string name, std::string content)
{
    const auto script = std::filesystem::temp_directory_path() / std::move(name);
    {
        std::ofstream file(script);
        file << content;
    }
    chmod(script.c_str(), 0700);
    return script;
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

void selector_compat_api_uses_detailed_error_path()
{
    auto values = candidates();
    throwing_selector selector;

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(!selected.has_value());
}

void selector_result_distinguishes_cancel_and_invalid_selection()
{
    auto values = candidates();
    fixed_selector cancel_selector{std::nullopt};
    fixed_selector invalid_selector{9};

    const auto cancelled = spagyrist::select_candidate_result(cancel_selector, values);
    const auto invalid = spagyrist::select_candidate_result(invalid_selector, values);

    SPAGYRIST_CHECK(cancelled.status == spagyrist::selector_status::cancelled);
    SPAGYRIST_CHECK(!cancelled.selected.has_value());
    SPAGYRIST_CHECK(invalid.status == spagyrist::selector_status::invalid_selection);
    SPAGYRIST_CHECK(!invalid.selected.has_value());
}

void selector_result_reports_empty_candidates_as_no_selection()
{
    std::vector<spagyrist::candidate> values;
    fixed_selector selector{0};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::no_selection);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    SPAGYRIST_CHECK(selected.message == "No candidates found.");
    SPAGYRIST_CHECK(selector.observed_size == 0);
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

void number_selector_empty_input_retries()
{
    auto values = candidates();
    std::istringstream input{"\n1\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 0);
    SPAGYRIST_CHECK(output.str().find("Enter a number between 1 and 2.") != std::string::npos);
}

void number_selector_invalid_input_retries()
{
    auto values = candidates();
    std::istringstream input{"abc\n1\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 0);
    SPAGYRIST_CHECK(output.str().find("Enter a number between 1 and 2.") != std::string::npos);
}

void number_selector_out_of_range_retries_and_shows_maximum()
{
    auto values = candidates();
    std::istringstream input{"9\n2\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 1);
    SPAGYRIST_CHECK(output.str().find("Selection out of range. Maximum is 2.") != std::string::npos);
}

void number_selector_eof_cancels()
{
    auto values = candidates();
    std::istringstream input;
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate(selector, values);

    SPAGYRIST_CHECK(!selected.has_value());
}

void number_selector_result_reports_selection()
{
    auto values = candidates();
    std::istringstream input{"2\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::selected);
    SPAGYRIST_CHECK(selected.selected.has_value());
    SPAGYRIST_CHECK(selected.selected->index == 1);
}

void number_selector_result_reports_eof_as_cancelled()
{
    auto values = candidates();
    std::istringstream input;
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::cancelled);
    SPAGYRIST_CHECK(!selected.selected.has_value());
}

void number_selector_result_retries_invalid_and_out_of_range_input()
{
    auto values = candidates();
    std::istringstream input{"abc\n9\n1\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::selected);
    SPAGYRIST_CHECK(selected.selected.has_value());
    SPAGYRIST_CHECK(selected.selected->index == 0);
    SPAGYRIST_CHECK(output.str().find("Enter a number between 1 and 2.") != std::string::npos);
    SPAGYRIST_CHECK(output.str().find("Selection out of range. Maximum is 2.") != std::string::npos);
}

void number_selector_result_reports_empty_candidates_as_no_selection()
{
    std::vector<spagyrist::candidate> values;
    std::istringstream input{"1\n"};
    std::ostringstream output;
    spagyrist::number_selector selector{{.input = &input, .output = &output}};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::no_selection);
    SPAGYRIST_CHECK(!selected.selected.has_value());
}

void fzf_selector_uses_external_process_even_for_one_candidate()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf.sh",
        "#!/bin/sh\n"
        "cat >/tmp/spagyrist-fake-fzf-input\n"
        "head -n 1 /tmp/spagyrist-fake-fzf-input\n");

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

void fzf_selector_passes_preview_when_available()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-preview.sh",
        "#!/bin/sh\n"
        "printf '%s\\n' \"$@\" >/tmp/spagyrist-fake-fzf-preview-args\n"
        "cat >/tmp/spagyrist-fake-fzf-preview-input\n"
        "head -n 1 /tmp/spagyrist-fake-fzf-preview-input\n");

    std::vector<spagyrist::candidate> values;
    spagyrist::candidate only;
    only.id = "Only";
    only.title = "Only";
    only.preview = "Line 1\nLine 2\tTabbed";
    values.push_back(only);

    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};
    const auto selected = spagyrist::select_candidate(selector, values);

    std::ifstream args{"/tmp/spagyrist-fake-fzf-preview-args"};
    const std::string arg_text{
        std::istreambuf_iterator<char>{args},
        std::istreambuf_iterator<char>{}};
    std::ifstream input{"/tmp/spagyrist-fake-fzf-preview-input"};
    const std::string input_text{
        std::istreambuf_iterator<char>{input},
        std::istreambuf_iterator<char>{}};

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(arg_text.find("--preview") != std::string::npos);
    SPAGYRIST_CHECK(arg_text.find("printf '%b' {3}") != std::string::npos);
    SPAGYRIST_CHECK(arg_text.find("--nth\n1\n") != std::string::npos);
    SPAGYRIST_CHECK(arg_text.find("--accept-nth\n2\n") != std::string::npos);
    SPAGYRIST_CHECK(input_text.find("Line 1\\nLine 2\\tTabbed") != std::string::npos);
    std::filesystem::remove(script);
    std::filesystem::remove("/tmp/spagyrist-fake-fzf-preview-args");
    std::filesystem::remove("/tmp/spagyrist-fake-fzf-preview-input");
}

void fzf_selector_does_not_add_preview_when_absent()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-no-preview.sh",
        "#!/bin/sh\n"
        "printf '%s\\n' \"$@\" >/tmp/spagyrist-fake-fzf-no-preview-args\n"
        "cat >/dev/null\n"
        "printf 'Only\\t0\\t\\n'\n");

    std::vector<spagyrist::candidate> values;
    spagyrist::candidate only;
    only.id = "Only";
    only.title = "Only";
    values.push_back(only);

    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};
    const auto selected = spagyrist::select_candidate(selector, values);

    std::ifstream args{"/tmp/spagyrist-fake-fzf-no-preview-args"};
    const std::string arg_text{
        std::istreambuf_iterator<char>{args},
        std::istreambuf_iterator<char>{}};

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(arg_text.find("--preview") == std::string::npos);
    SPAGYRIST_CHECK(arg_text.find("--nth\n1\n") != std::string::npos);
    SPAGYRIST_CHECK(arg_text.find("--accept-nth\n2\n") != std::string::npos);
    std::filesystem::remove(script);
    std::filesystem::remove("/tmp/spagyrist-fake-fzf-no-preview-args");
}

void fzf_selector_reports_missing_executable()
{
    spagyrist::fzf_selector_options options;
    options.executable = "/tmp/spagyrist-missing-fzf-for-test";
    spagyrist::fzf_selector selector{options};

    SPAGYRIST_CHECK(!selector.is_available());
}

void fzf_selector_result_reports_missing_executable_as_unavailable()
{
    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = "/tmp/spagyrist-missing-fzf-for-test";
    spagyrist::fzf_selector selector{options};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::unavailable);
    SPAGYRIST_CHECK(!selected.selected.has_value());
}

void fzf_selector_result_reports_out_of_range_index()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-out-of-range.sh",
        "#!/bin/sh\n"
        "cat >/dev/null\n"
        "printf '99\\tmissing\\n'\n");

    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::invalid_selection);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    std::filesystem::remove(script);
}

void fzf_selector_result_reports_invalid_output_as_error()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-invalid-output.sh",
        "#!/bin/sh\n"
        "cat >/dev/null\n"
        "printf 'not-an-index\\n'\n");

    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::error);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    std::filesystem::remove(script);
}

void fzf_selector_result_reports_cancelled_exit()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-cancel.sh",
        "#!/bin/sh\n"
        "cat >/dev/null\n"
        "exit 130\n");

    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::cancelled);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    std::filesystem::remove(script);
}

void fzf_selector_result_reports_abnormal_exit_as_error()
{
    const auto script = write_executable_script(
        "spagyrist-fake-fzf-error.sh",
        "#!/bin/sh\n"
        "cat >/dev/null\n"
        "printf 'boom\\n' >&2\n"
        "exit 2\n");

    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = script.string();
    spagyrist::fzf_selector selector{options};

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::error);
    SPAGYRIST_CHECK(selected.message.find("boom") != std::string::npos);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    std::filesystem::remove(script);
}

void selector_fallback_uses_fallback_when_fzf_is_missing()
{
    auto values = candidates();
    spagyrist::fzf_selector_options options;
    options.executable = "/tmp/spagyrist-missing-fzf-for-test";
    spagyrist::fzf_selector primary{options};
    fixed_selector fallback{1};

    const auto selected = spagyrist::select_candidate_with_fallback(primary, fallback, values);

    SPAGYRIST_CHECK(selected.has_value());
    SPAGYRIST_CHECK(selected->index == 1);
    SPAGYRIST_CHECK(fallback.observed_size == values.size());
}

void selector_fallback_uses_fallback_when_primary_is_unavailable()
{
    auto values = candidates();
    unavailable_selector primary;
    fixed_selector fallback{1};

    const auto selected = spagyrist::select_candidate_with_fallback_result(primary, fallback, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::selected);
    SPAGYRIST_CHECK(selected.selected.has_value());
    SPAGYRIST_CHECK(selected.selected->index == 1);
    SPAGYRIST_CHECK(!primary.selected);
    SPAGYRIST_CHECK(fallback.observed_size == values.size());
}

void selector_fallback_uses_fallback_when_primary_throws()
{
    auto values = candidates();
    throwing_selector primary;
    fixed_selector fallback{1};

    const auto selected = spagyrist::select_candidate_with_fallback_result(primary, fallback, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::selected);
    SPAGYRIST_CHECK(selected.selected.has_value());
    SPAGYRIST_CHECK(selected.selected->index == 1);
    SPAGYRIST_CHECK(fallback.observed_size == values.size());
}

void selector_fallback_does_not_fallback_when_primary_cancels()
{
    auto values = candidates();
    fixed_selector primary{std::nullopt};
    fixed_selector fallback{1};

    const auto selected = spagyrist::select_candidate_with_fallback_result(primary, fallback, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::cancelled);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    SPAGYRIST_CHECK(fallback.observed_size == 0);
}

void selector_fallback_does_not_fallback_when_primary_returns_invalid_index()
{
    auto values = candidates();
    fixed_selector primary{9};
    fixed_selector fallback{1};

    const auto selected = spagyrist::select_candidate_with_fallback_result(primary, fallback, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::invalid_selection);
    SPAGYRIST_CHECK(!selected.selected.has_value());
    SPAGYRIST_CHECK(fallback.observed_size == 0);
}

void builtin_selector_reports_unavailable_without_tty()
{
    auto values = candidates();
    spagyrist::builtin_selector selector;

    if (selector.is_available()) {
        return;
    }

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::unavailable);
    SPAGYRIST_CHECK(!selected.selected.has_value());
}

void builtin_selector_result_reports_empty_candidates_as_no_selection()
{
    std::vector<spagyrist::candidate> values;
    spagyrist::builtin_selector selector;

    const auto selected = spagyrist::select_candidate_result(selector, values);

    SPAGYRIST_CHECK(selected.status == spagyrist::selector_status::no_selection);
    SPAGYRIST_CHECK(!selected.selected.has_value());
}

} // namespace

void run_selector_tests()
{
    selector_result_maps_index_to_candidate();
    selector_cancel_returns_empty_selection();
    selector_out_of_range_returns_empty_selection();
    selector_compat_api_uses_detailed_error_path();
    selector_result_distinguishes_cancel_and_invalid_selection();
    selector_result_reports_empty_candidates_as_no_selection();
    number_selector_maps_number_to_index();
    number_selector_empty_input_retries();
    number_selector_invalid_input_retries();
    number_selector_out_of_range_retries_and_shows_maximum();
    number_selector_eof_cancels();
    number_selector_result_reports_selection();
    number_selector_result_reports_eof_as_cancelled();
    number_selector_result_retries_invalid_and_out_of_range_input();
    number_selector_result_reports_empty_candidates_as_no_selection();
    fzf_selector_uses_external_process_even_for_one_candidate();
    fzf_selector_passes_preview_when_available();
    fzf_selector_does_not_add_preview_when_absent();
    fzf_selector_reports_missing_executable();
    fzf_selector_result_reports_missing_executable_as_unavailable();
    fzf_selector_result_reports_out_of_range_index();
    fzf_selector_result_reports_invalid_output_as_error();
    fzf_selector_result_reports_cancelled_exit();
    fzf_selector_result_reports_abnormal_exit_as_error();
    selector_fallback_uses_fallback_when_fzf_is_missing();
    selector_fallback_uses_fallback_when_primary_is_unavailable();
    selector_fallback_uses_fallback_when_primary_throws();
    selector_fallback_does_not_fallback_when_primary_cancels();
    selector_fallback_does_not_fallback_when_primary_returns_invalid_index();
    builtin_selector_reports_unavailable_without_tty();
    builtin_selector_result_reports_empty_candidates_as_no_selection();
}
