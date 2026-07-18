#include "spagyrist/version.hpp"

#ifndef SPAGYRIST_VERSION
#define SPAGYRIST_VERSION "0.0.0"
#endif

namespace spagyrist {

std::string_view version() noexcept
{
    return SPAGYRIST_VERSION;
}

std::string version_text()
{
    std::string output{"libspagyrist "};
    output += version();
    return output;
}

} // namespace spagyrist
