#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace spagyrist::detail::fzf {

struct process_result {
    int exit_code{};
    std::string output;
    std::string error;
};

[[nodiscard]] process_result run_process(
    const std::string& executable,
    const std::vector<std::string>& arguments,
    std::string_view input);

} // namespace spagyrist::detail::fzf
