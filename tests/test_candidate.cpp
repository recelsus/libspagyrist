#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

namespace {

void valid_candidate_passes_validation()
{
    spagyrist::candidate value;
    value.id = "Linux";
    value.title = "Linux";
    value.description = "Family of Unix-like operating systems";
    value.url = "https://en.wikipedia.org/wiki/Linux";

    SPAGYRIST_CHECK(spagyrist::is_valid(value));
    SPAGYRIST_CHECK(spagyrist::validate(value).ok());
}

void invalid_candidate_reports_required_fields()
{
    const auto result = spagyrist::validate(spagyrist::candidate{});

    SPAGYRIST_CHECK(!result.ok());
    SPAGYRIST_CHECK(result.errors.size() == 2);
    SPAGYRIST_CHECK(result.errors[0].path == "candidate.id");
    SPAGYRIST_CHECK(result.errors[1].path == "candidate.title");
}

void candidate_list_reports_indexed_paths()
{
    spagyrist::candidate_list values;
    values.candidates.push_back(spagyrist::candidate{});

    const auto result = spagyrist::validate(values);

    SPAGYRIST_CHECK(!result.ok());
    SPAGYRIST_CHECK(result.errors.size() == 2);
    SPAGYRIST_CHECK(result.errors[0].path == "candidates[0].id");
    SPAGYRIST_CHECK(result.errors[1].path == "candidates[0].title");
}

} // namespace

void run_candidate_tests()
{
    valid_candidate_passes_validation();
    invalid_candidate_reports_required_fields();
    candidate_list_reports_indexed_paths();
}
