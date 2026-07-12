#include "spagyrist/renderer.hpp"

namespace spagyrist {

std::string render(const document& value, format output_format)
{
    switch (output_format) {
    case format::markdown:
        return render_markdown(value);
    case format::plain:
        return render_plain(value);
    case format::terminal:
        return render_terminal(value);
    }

    return {};
}

} // namespace spagyrist

