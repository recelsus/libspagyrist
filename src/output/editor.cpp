#include "spagyrist/output.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <ostream>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>

namespace spagyrist {
namespace {

std::string shell_quote(std::string_view value)
{
    std::string output{"'"};
    for (const auto ch : value) {
        if (ch == '\'') {
            output += "'\\''";
        } else {
            output += ch;
        }
    }
    output += '\'';
    return output;
}

bool is_executable(std::string_view path)
{
    return !path.empty() && access(std::string{path}.c_str(), X_OK) == 0;
}

bool path_contains_slash(std::string_view value)
{
    return value.find('/') != std::string_view::npos;
}

bool command_exists(std::string_view command)
{
    if (command.empty()) {
        return false;
    }
    if (path_contains_slash(command)) {
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

std::string make_temp_file(std::string_view content)
{
    std::array<char, 32> path{};
    std::strcpy(path.data(), "/tmp/spagyrist-editor-XXXXXX");

    const auto fd = mkstemp(path.data());
    if (fd < 0) {
        return {};
    }
    close(fd);

    std::ofstream file{path.data(), std::ios::binary | std::ios::trunc};
    if (!file) {
        unlink(path.data());
        return {};
    }
    file << content;
    if (!file) {
        unlink(path.data());
        return {};
    }

    return path.data();
}

bool run_shell_command(std::string_view command, std::string_view path)
{
    const auto full_command = std::string{command} + " " + shell_quote(path);
    const auto status = std::system(full_command.c_str());
    return status != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool try_editor_command(std::string_view command, std::string_view path)
{
    return !command.empty() && run_shell_command(command, path);
}

bool try_fallback_editor(std::string_view command, std::string_view path)
{
    if (!command_exists(command)) {
        return false;
    }
    return run_shell_command(shell_quote(command), path);
}

bool open_in_editor(std::string_view path)
{
    if (const auto* visual = std::getenv("VISUAL"); visual != nullptr && visual[0] != '\0') {
        if (try_editor_command(visual, path)) {
            return true;
        }
    }
    if (const auto* editor = std::getenv("EDITOR"); editor != nullptr && editor[0] != '\0') {
        if (try_editor_command(editor, path)) {
            return true;
        }
    }

    constexpr std::array<std::string_view, 4> fallback_editors{
        "nvim",
        "vim",
        "vi",
        "nano",
    };
    for (const auto editor : fallback_editors) {
        if (try_fallback_editor(editor, path)) {
            return true;
        }
    }
    return false;
}

} // namespace

bool write_editor(std::string_view content, std::ostream& fallback_stream)
{
    const auto path = make_temp_file(content);
    if (path.empty()) {
        fallback_stream << content;
        return false;
    }

    const auto opened = open_in_editor(path);
    unlink(path.c_str());
    if (!opened) {
        fallback_stream << content;
    }
    return opened;
}

} // namespace spagyrist
