#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <sstream>

namespace {

void stdout_output_writes_content_to_stream()
{
    std::ostringstream stream;

    spagyrist::write_stdout(stream, "rendered content\n");

    SPAGYRIST_CHECK(stream.str() == "rendered content\n");
}

} // namespace

void run_output_tests()
{
    stdout_output_writes_content_to_stream();
}

