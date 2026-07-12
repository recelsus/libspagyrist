#pragma once

#include "spagyrist/document.hpp"

#include <string>

namespace spagyrist {

enum class format {
    markdown,
    plain,
    terminal,
};

[[nodiscard]] std::string render_markdown(const document& value);
[[nodiscard]] std::string render_plain(const document& value);
[[nodiscard]] std::string render_terminal(const document& value);
[[nodiscard]] std::string render(const document& value, format output_format);

} // namespace spagyrist

