#include "spagyrist/help.hpp"

#include <array>
#include <string>

namespace spagyrist {
namespace {

constexpr std::array selector_options{
    help_option{"builtin", "Use the built-in interactive selector."},
    help_option{"fzf", "Use fzf as an external selector."},
    help_option{"number", "Use numbered stdin selection."},
};

constexpr std::array format_options{
    help_option{"terminal", "Render for terminal display."},
    help_option{"markdown", "Render as Markdown."},
    help_option{"plain", "Render as plain text."},
};

constexpr std::array output_options{
    help_option{"stdout", "Write rendered content to standard output."},
    help_option{"editor", "Open rendered content with VISUAL, EDITOR, or a fallback editor."},
};

void append_line(std::string& output, std::string_view line)
{
    output += line;
    output += '\n';
}

std::string option_values_text(std::span<const help_option> options)
{
    if (options.empty()) {
        return {};
    }
    std::string output{"<"};
    for (std::size_t i = 0; i < options.size(); ++i) {
        if (i != 0) {
            output += '|';
        }
        output += options[i].value;
    }
    output += '>';
    return output;
}

std::string option_usage(const cli_help_option& option)
{
    std::string output;
    if (!option.short_name.empty()) {
        output += option.short_name;
        output += ", ";
    }
    output += option.long_name;
    const auto values = option_values_text(option.values);
    if (!values.empty()) {
        output += ' ';
        output += values;
    }
    return output;
}

std::string option_summary(const cli_help_option& option)
{
    std::string output{option.description};
    if (!option.default_value.empty()) {
        output += ". Default: ";
        output += option.default_value;
    }
    return output;
}

std::size_t usage_width(const cli_help_option& option)
{
    return option_usage(option).size();
}

} // namespace

std::span<const help_option> selector_help_options() noexcept
{
    return selector_options;
}

std::span<const help_option> format_help_options() noexcept
{
    return format_options;
}

std::span<const help_option> output_help_options() noexcept
{
    return output_options;
}

cli_help_option selector_cli_help_option() noexcept
{
    return cli_help_option{
        .long_name = "--select",
        .short_name = "-s",
        .description = "Selector",
        .default_value = "builtin",
        .values = selector_help_options(),
    };
}

cli_help_option format_cli_help_option() noexcept
{
    return cli_help_option{
        .long_name = "--format",
        .short_name = "-f",
        .description = "Output format",
        .default_value = "terminal",
        .values = format_help_options(),
    };
}

cli_help_option output_cli_help_option() noexcept
{
    return cli_help_option{
        .long_name = "--output",
        .short_name = "-o",
        .description = "Output target",
        .default_value = "stdout",
        .values = output_help_options(),
    };
}

cli_help_option version_cli_help_option() noexcept
{
    return cli_help_option{
        .long_name = "--version",
        .short_name = {},
        .description = "Show version information",
        .default_value = {},
        .values = {},
    };
}

cli_help_option info_cli_help_option() noexcept
{
    return cli_help_option{
        .long_name = "--info",
        .short_name = {},
        .description = "Show Spagyrist runtime information",
        .default_value = {},
        .values = {},
    };
}

std::string format_cli_help_option(const cli_help_option& option)
{
    std::string output;
    const auto usage = option_usage(option);
    output += "  ";
    output += usage;
    constexpr std::size_t description_column = 40;
    const auto used_width = usage_width(option);
    if (used_width < description_column) {
        output.append(description_column - used_width, ' ');
    }
    output += "  ";
    output += option_summary(option);
    output += '\n';
    return output;
}

std::string format_help_options(
    std::string_view option_name,
    std::span<const help_option> options)
{
    cli_help_option option{
        .long_name = option_name,
        .short_name = {},
        .description = {},
        .default_value = {},
        .values = options,
    };
    if (option_name == "--select") {
        option = selector_cli_help_option();
    } else if (option_name == "--format") {
        option = format_cli_help_option();
    } else if (option_name == "--output") {
        option = output_cli_help_option();
    }
    return format_cli_help_option(option);
}

std::string selector_help_text()
{
    return format_cli_help_option(selector_cli_help_option());
}

std::string format_help_text()
{
    return format_cli_help_option(format_cli_help_option());
}

std::string output_help_text()
{
    return format_cli_help_option(output_cli_help_option());
}

std::string cli_help_text()
{
    std::string output;
    append_line(output, "Spagyrist options:");
    output += selector_help_text();
    output += format_help_text();
    output += output_help_text();
    output += format_cli_help_option(version_cli_help_option());
    output += format_cli_help_option(info_cli_help_option());
    return output;
}

} // namespace spagyrist
