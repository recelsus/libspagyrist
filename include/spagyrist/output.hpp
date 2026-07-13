#pragma once

#include <iosfwd>
#include <string_view>

namespace spagyrist {

enum class output_target {
    standard_output,
    pager,
    editor,
};

void write_stdout(std::ostream& stream, std::string_view content);

} // namespace spagyrist
