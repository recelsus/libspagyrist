#include "spagyrist/cli.hpp"

#include "spagyrist/selector/builtin.hpp"
#include "spagyrist/selector/fzf.hpp"
#include "spagyrist/selector/number.hpp"

namespace spagyrist {

std::optional<selector_kind> parse_selector_kind(std::string_view value) noexcept
{
    if (value == "builtin") {
        return selector_kind::builtin;
    }
    if (value == "fzf") {
        return selector_kind::fzf;
    }
    if (value == "number") {
        return selector_kind::number;
    }
    return std::nullopt;
}

std::optional<format> parse_format(std::string_view value) noexcept
{
    if (value == "terminal") {
        return format::terminal;
    }
    if (value == "markdown") {
        return format::markdown;
    }
    if (value == "plain") {
        return format::plain;
    }
    return std::nullopt;
}

std::optional<output_target> parse_output_target(std::string_view value) noexcept
{
    if (value == "stdout") {
        return output_target::standard_output;
    }
    if (value == "editor") {
        return output_target::editor;
    }
    return std::nullopt;
}

bool is_common_value_option(std::string_view arg) noexcept
{
    return arg == "--select" || arg == "-s"
        || arg == "--format" || arg == "-f"
        || arg == "--output" || arg == "-o";
}

bool is_common_flag_option(std::string_view arg) noexcept
{
    return arg == "--version" || arg == "--info";
}

cli_parse_result parse_common_args(std::span<const std::string> args)
{
    cli_parse_result result;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        if (arg == "--version") {
            result.options.version = true;
            continue;
        }
        if (arg == "--info") {
            result.options.info = true;
            continue;
        }
        if (!is_common_value_option(arg)) {
            result.remaining_args.push_back(arg);
            continue;
        }
        if (i + 1 >= args.size()) {
            result.error = "missing value for " + arg;
            return result;
        }

        const auto& value = args[++i];
        if (arg == "--select" || arg == "-s") {
            const auto parsed = parse_selector_kind(value);
            if (!parsed) {
                result.error = "invalid selector: " + value;
                return result;
            }
            result.options.selector = *parsed;
        } else if (arg == "--format" || arg == "-f") {
            const auto parsed = parse_format(value);
            if (!parsed) {
                result.error = "invalid format: " + value;
                return result;
            }
            result.options.output_format = *parsed;
        } else if (arg == "--output" || arg == "-o") {
            const auto parsed = parse_output_target(value);
            if (!parsed) {
                result.error = "invalid output target: " + value;
                return result;
            }
            result.options.output = *parsed;
        }
    }
    return result;
}

candidate_selection_result
select_candidate(selector_kind kind, std::span<const candidate> candidates)
{
    switch (kind) {
    case selector_kind::builtin: {
        builtin_selector primary;
        number_selector fallback;
        return select_candidate_with_fallback_result(primary, fallback, candidates);
    }
    case selector_kind::fzf: {
        fzf_selector primary;
        number_selector fallback;
        return select_candidate_with_fallback_result(primary, fallback, candidates);
    }
    case selector_kind::number: {
        number_selector selector;
        return select_candidate_result(selector, candidates);
    }
    }

    return candidate_selection_result{
        .status = selector_status::error,
        .selected = std::nullopt,
        .message = "unknown selector",
    };
}

bool write_output(output_target target, std::string_view content, std::ostream& fallback_stream)
{
    if (target == output_target::editor) {
        return write_editor(content, fallback_stream);
    }
    write_stdout(fallback_stream, content);
    return true;
}

} // namespace spagyrist
