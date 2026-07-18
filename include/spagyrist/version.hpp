#pragma once

#include <string>
#include <string_view>

namespace spagyrist {

[[nodiscard]] std::string_view version() noexcept;
[[nodiscard]] std::string version_text();

} // namespace spagyrist
