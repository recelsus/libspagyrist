#include "spagyrist/output.hpp"

#include <ostream>

namespace spagyrist {

void write_stdout(std::ostream& stream, std::string_view content)
{
    stream << content;
}

} // namespace spagyrist

