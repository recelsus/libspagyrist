#include "spagyrist/renderer.hpp"

namespace spagyrist {

std::string render_terminal(const document& value)
{
    return render_plain(value);
}

} // namespace spagyrist

