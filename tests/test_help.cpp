#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <string>

namespace {

void help_exposes_selector_options()
{
    const auto options = spagyrist::selector_help_options();

    SPAGYRIST_CHECK(options.size() == 3);
    SPAGYRIST_CHECK(options[0].value == "builtin");
    SPAGYRIST_CHECK(options[1].value == "fzf");
    SPAGYRIST_CHECK(options[2].value == "number");
}

void help_exposes_format_options()
{
    const auto options = spagyrist::format_help_options();

    SPAGYRIST_CHECK(options.size() == 3);
    SPAGYRIST_CHECK(options[0].value == "terminal");
    SPAGYRIST_CHECK(options[1].value == "markdown");
    SPAGYRIST_CHECK(options[2].value == "plain");
}

void help_exposes_output_options()
{
    const auto options = spagyrist::output_help_options();

    SPAGYRIST_CHECK(options.size() == 2);
    SPAGYRIST_CHECK(options[0].value == "stdout");
    SPAGYRIST_CHECK(options[1].value == "editor");
}

void help_formats_options_with_option_name()
{
    const auto text = spagyrist::format_help_options("--output", spagyrist::output_help_options());

    SPAGYRIST_CHECK(text.find("-o, --output <stdout|editor>") != std::string::npos);
    SPAGYRIST_CHECK(text.find("Default: stdout") != std::string::npos);
}

void help_exposes_cli_option_aliases()
{
    const auto selector = spagyrist::selector_cli_help_option();
    const auto format = spagyrist::format_cli_help_option();
    const auto output = spagyrist::output_cli_help_option();
    const auto version = spagyrist::version_cli_help_option();
    const auto info = spagyrist::info_cli_help_option();

    SPAGYRIST_CHECK(selector.long_name == "--select");
    SPAGYRIST_CHECK(selector.short_name == "-s");
    SPAGYRIST_CHECK(format.long_name == "--format");
    SPAGYRIST_CHECK(format.short_name == "-f");
    SPAGYRIST_CHECK(output.long_name == "--output");
    SPAGYRIST_CHECK(output.short_name == "-o");
    SPAGYRIST_CHECK(version.long_name == "--version");
    SPAGYRIST_CHECK(version.short_name.empty());
    SPAGYRIST_CHECK(info.long_name == "--info");
    SPAGYRIST_CHECK(info.short_name.empty());
}

void help_builds_common_cli_text()
{
    const auto text = spagyrist::cli_help_text();

    SPAGYRIST_CHECK(text.find("Spagyrist options:") != std::string::npos);
    SPAGYRIST_CHECK(text.find("--select auto") == std::string::npos);
    SPAGYRIST_CHECK(text.find("-s, --select <builtin|fzf|number>") != std::string::npos);
    SPAGYRIST_CHECK(text.find("-f, --format <terminal|markdown|plain>") != std::string::npos);
    SPAGYRIST_CHECK(text.find("-o, --output <stdout|editor>") != std::string::npos);
    SPAGYRIST_CHECK(text.find("--version") != std::string::npos);
    SPAGYRIST_CHECK(text.find("--info") != std::string::npos);
}

} // namespace

void run_help_tests()
{
    help_exposes_selector_options();
    help_exposes_format_options();
    help_exposes_output_options();
    help_formats_options_with_option_name();
    help_exposes_cli_option_aliases();
    help_builds_common_cli_text();
}
