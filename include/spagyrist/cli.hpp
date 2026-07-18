#pragma once

#include "spagyrist/output.hpp"
#include "spagyrist/renderer.hpp"
#include "spagyrist/selector.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spagyrist {

enum class selector_kind {
    builtin,
    fzf,
    number,
};

struct common_options {
    selector_kind selector{selector_kind::builtin};
    format output_format{format::terminal};
    output_target output{output_target::standard_output};
    bool version{false};
    bool info{false};
};

struct cli_parse_result {
    common_options options;
    std::vector<std::string> remaining_args;
    std::optional<std::string> error;
};

[[nodiscard]] std::optional<selector_kind> parse_selector_kind(std::string_view value) noexcept;
[[nodiscard]] std::optional<format> parse_format(std::string_view value) noexcept;
[[nodiscard]] std::optional<output_target> parse_output_target(std::string_view value) noexcept;

[[nodiscard]] bool is_common_value_option(std::string_view arg) noexcept;
[[nodiscard]] bool is_common_flag_option(std::string_view arg) noexcept;

[[nodiscard]] cli_parse_result parse_common_args(std::span<const std::string> args);
[[nodiscard]] candidate_selection_result
select_candidate(selector_kind kind, std::span<const candidate> candidates);
bool write_output(output_target target, std::string_view content, std::ostream& fallback_stream);

} // namespace spagyrist
