#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>

namespace {

class scoped_env {
public:
    explicit scoped_env(const char* name)
        : name_(name)
    {
        if (const auto* value = std::getenv(name_); value != nullptr) {
            original_ = value;
        }
    }

    scoped_env(const scoped_env&) = delete;
    scoped_env& operator=(const scoped_env&) = delete;

    ~scoped_env()
    {
        if (original_) {
            setenv(name_, original_->c_str(), 1);
        } else {
            unsetenv(name_);
        }
    }

private:
    const char* name_;
    std::optional<std::string> original_;
};

void stdout_output_writes_content_to_stream()
{
    std::ostringstream stream;

    spagyrist::write_stdout(stream, "rendered content\n");

    SPAGYRIST_CHECK(stream.str() == "rendered content\n");
}

void editor_output_uses_visual_when_available()
{
    scoped_env visual{"VISUAL"};
    scoped_env editor{"EDITOR"};
    setenv("VISUAL", "true", 1);
    unsetenv("EDITOR");
    std::ostringstream stream;

    const auto opened = spagyrist::write_editor("rendered content\n", stream);

    SPAGYRIST_CHECK(opened);
    SPAGYRIST_CHECK(stream.str().empty());
}

void editor_output_falls_back_from_visual_to_editor()
{
    scoped_env visual{"VISUAL"};
    scoped_env editor{"EDITOR"};
    setenv("VISUAL", "false", 1);
    setenv("EDITOR", "true", 1);
    std::ostringstream stream;

    const auto opened = spagyrist::write_editor("rendered content\n", stream);

    SPAGYRIST_CHECK(opened);
    SPAGYRIST_CHECK(stream.str().empty());
}

void editor_output_falls_back_to_stream_when_no_editor_is_available()
{
    scoped_env visual{"VISUAL"};
    scoped_env editor{"EDITOR"};
    scoped_env path{"PATH"};
    unsetenv("VISUAL");
    unsetenv("EDITOR");
    setenv("PATH", "", 1);
    std::ostringstream stream;

    const auto opened = spagyrist::write_editor("rendered content\n", stream);

    SPAGYRIST_CHECK(!opened);
    SPAGYRIST_CHECK(stream.str() == "rendered content\n");
}

} // namespace

void run_output_tests()
{
    stdout_output_writes_content_to_stream();
    editor_output_uses_visual_when_available();
    editor_output_falls_back_from_visual_to_editor();
    editor_output_falls_back_to_stream_when_no_editor_is_available();
}
