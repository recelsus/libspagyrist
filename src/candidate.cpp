#include "spagyrist/candidate.hpp"

namespace spagyrist {

bool is_valid(const candidate& value) noexcept
{
    return !value.id.empty() && !value.title.empty();
}

} // namespace spagyrist

