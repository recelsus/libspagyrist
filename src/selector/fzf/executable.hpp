#pragma once

#include <string>

namespace spagyrist::detail::fzf {

[[nodiscard]] bool executable_exists(const std::string& executable);

} // namespace spagyrist::detail::fzf
