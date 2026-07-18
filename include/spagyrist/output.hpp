#pragma once

#include <iosfwd>
#include <string_view>

namespace spagyrist {

enum class output_target {
    standard_output,
    pager,
    editor,
};

// Opens content in VISUAL, EDITOR, or a known terminal editor. If no editor can
// be used, writes content to fallback_stream and returns false.
bool write_editor(std::string_view content, std::ostream& fallback_stream);
void write_stdout(std::ostream& stream, std::string_view content);

} // namespace spagyrist
