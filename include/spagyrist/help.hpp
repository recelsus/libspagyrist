#pragma once

#include <span>
#include <string>
#include <string_view>

namespace spagyrist {

struct help_option {
    std::string_view value;
    std::string_view description;
};

struct cli_help_option {
    std::string_view long_name;
    std::string_view short_name;
    std::string_view description;
    std::string_view default_value;
    std::span<const help_option> values;
};

[[nodiscard]] std::span<const help_option> selector_help_options() noexcept;
[[nodiscard]] std::span<const help_option> format_help_options() noexcept;
[[nodiscard]] std::span<const help_option> output_help_options() noexcept;

[[nodiscard]] cli_help_option selector_cli_help_option() noexcept;
[[nodiscard]] cli_help_option format_cli_help_option() noexcept;
[[nodiscard]] cli_help_option output_cli_help_option() noexcept;
[[nodiscard]] cli_help_option version_cli_help_option() noexcept;
[[nodiscard]] cli_help_option info_cli_help_option() noexcept;

[[nodiscard]] std::string format_cli_help_option(const cli_help_option& option);
[[nodiscard]] std::string format_help_options(
    std::string_view option_name,
    std::span<const help_option> options);

[[nodiscard]] std::string selector_help_text();
[[nodiscard]] std::string format_help_text();
[[nodiscard]] std::string output_help_text();
[[nodiscard]] std::string cli_help_text();

} // namespace spagyrist
