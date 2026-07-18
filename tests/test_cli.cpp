#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace {

void cli_parses_common_options_and_keeps_remaining_args()
{
    const std::vector<std::string> args{
        "-s",
        "number",
        "-f",
        "markdown",
        "-o",
        "stdout",
        "--client-option",
        "query",
    };

    const auto parsed = spagyrist::parse_common_args(args);

    SPAGYRIST_CHECK(!parsed.error.has_value());
    SPAGYRIST_CHECK(parsed.options.selector == spagyrist::selector_kind::number);
    SPAGYRIST_CHECK(parsed.options.output_format == spagyrist::format::markdown);
    SPAGYRIST_CHECK(parsed.options.output == spagyrist::output_target::standard_output);
    SPAGYRIST_CHECK(parsed.remaining_args.size() == 2);
    SPAGYRIST_CHECK(parsed.remaining_args[0] == "--client-option");
    SPAGYRIST_CHECK(parsed.remaining_args[1] == "query");
}

void cli_parses_version_and_info_flags()
{
    const std::vector<std::string> args{
        "--version",
        "--info",
    };

    const auto parsed = spagyrist::parse_common_args(args);

    SPAGYRIST_CHECK(!parsed.error.has_value());
    SPAGYRIST_CHECK(parsed.options.version);
    SPAGYRIST_CHECK(parsed.options.info);
    SPAGYRIST_CHECK(parsed.remaining_args.empty());
}

void cli_reports_invalid_common_option_values()
{
    const std::vector<std::string> args{
        "--select",
        "missing",
    };

    const auto parsed = spagyrist::parse_common_args(args);

    SPAGYRIST_CHECK(parsed.error == "invalid selector: missing");
}

void cli_writes_output_to_stdout_target()
{
    std::ostringstream output;

    const auto wrote = spagyrist::write_output(
        spagyrist::output_target::standard_output,
        "rendered\n",
        output);

    SPAGYRIST_CHECK(wrote);
    SPAGYRIST_CHECK(output.str() == "rendered\n");
}

} // namespace

void run_cli_tests()
{
    cli_parses_common_options_and_keeps_remaining_args();
    cli_parses_version_and_info_flags();
    cli_reports_invalid_common_option_values();
    cli_writes_output_to_stdout_target();
}
