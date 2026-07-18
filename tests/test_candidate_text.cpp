#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <vector>

namespace {

spagyrist::candidate rich_candidate()
{
    spagyrist::candidate value;
    value.id = "linux";
    value.title = "Linux";
    value.subtitle = "Kernel";
    value.description = "Unix-like operating system";
    value.url = "https://example.test/linux";
    value.preview = "Preview text";
    value.metadata.emplace("hidden", "metadata-value");
    return value;
}

void candidate_text_uses_title_for_display()
{
    const auto projected = spagyrist::project_candidate_text(3, rich_candidate());

    SPAGYRIST_CHECK(projected.index == 3);
    SPAGYRIST_CHECK(projected.display == "Linux - Kernel");
}

void candidate_text_falls_back_to_description_for_display()
{
    auto value = rich_candidate();
    value.subtitle.reset();

    const auto projected = spagyrist::project_candidate_text(0, value);

    SPAGYRIST_CHECK(projected.display == "Linux - Unix-like operating system");
}

void candidate_text_search_includes_standard_fields()
{
    const auto projected = spagyrist::project_candidate_text(0, rich_candidate());

    SPAGYRIST_CHECK(projected.search.find("Linux") != std::string::npos);
    SPAGYRIST_CHECK(projected.search.find("Kernel") != std::string::npos);
    SPAGYRIST_CHECK(projected.search.find("Unix-like operating system") != std::string::npos);
    SPAGYRIST_CHECK(projected.search.find("https://example.test/linux") != std::string::npos);
}

void candidate_text_search_excludes_metadata_by_default()
{
    const auto projected = spagyrist::project_candidate_text(0, rich_candidate());

    SPAGYRIST_CHECK(projected.search.find("metadata-value") == std::string::npos);
}

void candidate_text_sanitizes_control_characters()
{
    spagyrist::candidate value;
    value.id = "control";
    value.title = "Line\nBreak\x1b[31m";
    value.subtitle = "Tabbed\tText";
    value.description = "Carriage\rReturn\aBell";
    value.url = "https://example.test/delete\x7f";

    const auto projected = spagyrist::project_candidate_text(0, value);

    SPAGYRIST_CHECK(projected.display == "Line Break [31m - Tabbed Text");
    SPAGYRIST_CHECK(projected.search.find('\n') == std::string::npos);
    SPAGYRIST_CHECK(projected.search.find('\t') == std::string::npos);
    SPAGYRIST_CHECK(projected.search.find('\r') == std::string::npos);
    SPAGYRIST_CHECK(projected.search.find('\a') == std::string::npos);
    SPAGYRIST_CHECK(projected.search.find('\x1b') == std::string::npos);
    SPAGYRIST_CHECK(projected.search.find('\x7f') == std::string::npos);
}

void candidate_text_projection_keeps_original_candidate()
{
    const auto original = rich_candidate();
    auto value = original;

    static_cast<void>(spagyrist::project_candidate_text(0, value));

    SPAGYRIST_CHECK(value.title == original.title);
    SPAGYRIST_CHECK(value.subtitle == original.subtitle);
    SPAGYRIST_CHECK(value.description == original.description);
    SPAGYRIST_CHECK(value.url == original.url);
    SPAGYRIST_CHECK(value.preview == original.preview);
    SPAGYRIST_CHECK(value.metadata == original.metadata);
}

void candidate_texts_keep_original_indices()
{
    std::vector<spagyrist::candidate> values;
    values.push_back(rich_candidate());
    values.push_back(rich_candidate());
    values.back().title = "Second";

    const auto projected = spagyrist::project_candidate_texts(values);

    SPAGYRIST_CHECK(projected.size() == 2);
    SPAGYRIST_CHECK(projected[0].index == 0);
    SPAGYRIST_CHECK(projected[1].index == 1);
    SPAGYRIST_CHECK(projected[1].display == "Second - Kernel");
}

} // namespace

void run_candidate_text_tests()
{
    candidate_text_uses_title_for_display();
    candidate_text_falls_back_to_description_for_display();
    candidate_text_search_includes_standard_fields();
    candidate_text_search_excludes_metadata_by_default();
    candidate_text_sanitizes_control_characters();
    candidate_text_projection_keeps_original_candidate();
    candidate_texts_keep_original_indices();
}
