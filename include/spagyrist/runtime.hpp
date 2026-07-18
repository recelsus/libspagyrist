#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace spagyrist {

struct runtime_status {
    std::string name;
    bool available{};
    std::string detail;
};

[[nodiscard]] std::vector<runtime_status> selector_runtime_statuses();
[[nodiscard]] std::vector<runtime_status> editor_runtime_statuses();

[[nodiscard]] std::string format_runtime_statuses(
    std::string_view title,
    const std::vector<runtime_status>& statuses);

[[nodiscard]] std::string runtime_info_text();

} // namespace spagyrist
