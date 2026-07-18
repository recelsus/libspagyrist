#include "executable.hpp"

#include <cstdlib>
#include <string_view>
#include <unistd.h>

namespace spagyrist::detail::fzf {
namespace {

bool is_executable_file(const std::string& path)
{
    return !path.empty() && access(path.c_str(), X_OK) == 0;
}

} // namespace

bool executable_exists(const std::string& executable)
{
    if (executable.find('/') != std::string::npos) {
        return is_executable_file(executable);
    }

    const auto* path_value = std::getenv("PATH");
    if (path_value == nullptr) {
        return false;
    }

    const auto path = std::string_view{path_value};
    std::size_t pos = 0;
    while (pos <= path.size()) {
        const auto colon = path.find(':', pos);
        const auto entry = path.substr(pos, colon == std::string_view::npos ? path.size() - pos : colon - pos);
        const auto directory = entry.empty() ? "." : std::string{entry};
        auto candidate_path = directory;
        if (!candidate_path.empty() && candidate_path.back() != '/') {
            candidate_path += '/';
        }
        candidate_path += executable;
        if (is_executable_file(candidate_path)) {
            return true;
        }
        if (colon == std::string_view::npos) {
            break;
        }
        pos = colon + 1;
    }
    return false;
}

} // namespace spagyrist::detail::fzf
