#include "spagyrist/runtime.hpp"

#include "selector/fzf/executable.hpp"
#include "spagyrist/version.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <string>
#include <string_view>
#include <unistd.h>

namespace spagyrist {
namespace {

bool is_executable(std::string_view path)
{
    return !path.empty() && access(std::string{path}.c_str(), X_OK) == 0;
}

bool command_exists(std::string_view command)
{
    if (command.empty()) {
        return false;
    }
    if (command.find('/') != std::string_view::npos) {
        return is_executable(command);
    }

    const auto* path_env = std::getenv("PATH");
    if (path_env == nullptr || std::string_view{path_env}.empty()) {
        return false;
    }

    std::string_view path{path_env};
    while (true) {
        const auto separator = path.find(':');
        const auto directory = path.substr(0, separator);
        const auto candidate = std::string{directory.empty() ? "." : std::string{directory}}
            + "/" + std::string{command};
        if (is_executable(candidate)) {
            return true;
        }
        if (separator == std::string_view::npos) {
            break;
        }
        path.remove_prefix(separator + 1);
    }
    return false;
}

std::string env_detail(const char* name)
{
    if (const auto* value = std::getenv(name); value != nullptr && value[0] != '\0') {
        std::string output{name};
        output += '=';
        output += value;
        return output;
    }
    return "not set";
}

std::size_t max_name_width(const std::vector<runtime_status>& statuses) noexcept
{
    std::size_t width = 0;
    for (const auto& status : statuses) {
        width = std::max(width, status.name.size());
    }
    return width;
}

void append_status(std::string& output, const runtime_status& status, std::size_t name_width)
{
    output += "  ";
    output += status.available ? "[x] " : "[ ] ";
    output += status.name;
    output.append(name_width - status.name.size(), ' ');
    if (!status.detail.empty()) {
        output += "  ";
        output += status.detail;
    }
    output += '\n';
}

} // namespace

std::vector<runtime_status> selector_runtime_statuses()
{
    return {
        runtime_status{"builtin", true, "built in"},
        runtime_status{"number", true, "built in"},
        runtime_status{
            "fzf",
            detail::fzf::executable_exists("fzf"),
            "external command: fzf"},
    };
}

std::vector<runtime_status> editor_runtime_statuses()
{
    std::vector<runtime_status> output;
    output.push_back(runtime_status{
        "VISUAL",
        std::getenv("VISUAL") != nullptr && std::getenv("VISUAL")[0] != '\0',
        env_detail("VISUAL"),
    });
    output.push_back(runtime_status{
        "EDITOR",
        std::getenv("EDITOR") != nullptr && std::getenv("EDITOR")[0] != '\0',
        env_detail("EDITOR"),
    });

    constexpr std::array<std::string_view, 4> fallback_editors{
        "nvim",
        "vim",
        "vi",
        "nano",
    };
    for (const auto editor : fallback_editors) {
        output.push_back(runtime_status{
            std::string{editor},
            command_exists(editor),
            "fallback editor",
        });
    }
    return output;
}

std::string format_runtime_statuses(
    std::string_view title,
    const std::vector<runtime_status>& statuses)
{
    std::string output;
    output += title;
    output += ":\n";
    const auto name_width = max_name_width(statuses);
    for (const auto& status : statuses) {
        append_status(output, status, name_width);
    }
    return output;
}

std::string runtime_info_text()
{
    std::string output;
    output += version_text();
    output += "\n\n";
    output += format_runtime_statuses("Selectors", selector_runtime_statuses());
    output += '\n';
    output += format_runtime_statuses("Editors", editor_runtime_statuses());
    return output;
}

} // namespace spagyrist
