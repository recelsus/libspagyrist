#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <cstdlib>
#include <optional>
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

void version_reports_project_version()
{
    SPAGYRIST_CHECK(spagyrist::version() == "0.1.0");
    SPAGYRIST_CHECK(spagyrist::version_text() == "libspagyrist 0.1.0");
}

void runtime_reports_builtin_selectors()
{
    const auto selectors = spagyrist::selector_runtime_statuses();

    SPAGYRIST_CHECK(selectors.size() == 3);
    SPAGYRIST_CHECK(selectors[0].name == "builtin");
    SPAGYRIST_CHECK(selectors[0].available);
    SPAGYRIST_CHECK(selectors[1].name == "number");
    SPAGYRIST_CHECK(selectors[1].available);
    SPAGYRIST_CHECK(selectors[2].name == "fzf");
}

void runtime_reports_editor_environment()
{
    scoped_env visual{"VISUAL"};
    scoped_env editor{"EDITOR"};
    setenv("VISUAL", "test-visual", 1);
    unsetenv("EDITOR");

    const auto editors = spagyrist::editor_runtime_statuses();

    SPAGYRIST_CHECK(editors.size() == 6);
    SPAGYRIST_CHECK(editors[0].name == "VISUAL");
    SPAGYRIST_CHECK(editors[0].available);
    SPAGYRIST_CHECK(editors[0].detail == "VISUAL=test-visual");
    SPAGYRIST_CHECK(editors[1].name == "EDITOR");
    SPAGYRIST_CHECK(!editors[1].available);
}

void runtime_formats_statuses()
{
    const auto text = spagyrist::runtime_info_text();

    SPAGYRIST_CHECK(text.find("libspagyrist 0.1.0") != std::string::npos);
    SPAGYRIST_CHECK(text.find("Selectors:") != std::string::npos);
    SPAGYRIST_CHECK(text.find("Editors:") != std::string::npos);
}

} // namespace

void run_runtime_tests()
{
    version_reports_project_version();
    runtime_reports_builtin_selectors();
    runtime_reports_editor_environment();
    runtime_formats_statuses();
}
