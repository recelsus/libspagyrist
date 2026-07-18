#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <string>
#include <vector>

namespace {

void fuzzy_match_matches_exact_text()
{
    const auto result = spagyrist::fuzzy_match("Linux", "Linux");

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(result.positions == std::vector<std::size_t>({0, 1, 2, 3, 4}));
    SPAGYRIST_CHECK(result.score > 1000.0);
}

void fuzzy_match_matches_prefix()
{
    const auto prefix = spagyrist::fuzzy_match("Lin", "Linux");
    const auto later = spagyrist::fuzzy_match("Lin", "GNU/Linux");

    SPAGYRIST_CHECK(prefix.matched);
    SPAGYRIST_CHECK(later.matched);
    SPAGYRIST_CHECK(prefix.score > later.score);
}

void fuzzy_match_matches_subsequence()
{
    const auto result = spagyrist::fuzzy_match("lx", "Linux");

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(result.positions == std::vector<std::size_t>({0, 4}));
}

void fuzzy_match_rejects_missing_characters()
{
    const auto result = spagyrist::fuzzy_match("lz", "Linux");

    SPAGYRIST_CHECK(!result.matched);
    SPAGYRIST_CHECK(result.positions.empty());
}

void fuzzy_match_keeps_scattered_long_matches_with_lower_score()
{
    const auto scattered = spagyrist::fuzzy_match(
        "Apple",
        "A research operating system from Bell Labs https://example.test/plan9");
    const auto close = spagyrist::fuzzy_match("Apple", "Apple operating system core");

    SPAGYRIST_CHECK(scattered.matched);
    SPAGYRIST_CHECK(close.matched);
    SPAGYRIST_CHECK(close.score > scattered.score);
}

void fuzzy_match_allows_long_candidate_abbreviation()
{
    const auto result = spagyrist::fuzzy_match(
        "fb",
        "src/selector/builtin/fuzzy_builtin_selector_behavior.cpp");

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(result.positions.size() == 2);
}

void fuzzy_match_scores_consecutive_match_higher()
{
    const auto consecutive = spagyrist::fuzzy_match("abc", "abc");
    const auto separated = spagyrist::fuzzy_match("abc", "a_b_c");

    SPAGYRIST_CHECK(consecutive.matched);
    SPAGYRIST_CHECK(separated.matched);
    SPAGYRIST_CHECK(consecutive.score > separated.score);
}

void fuzzy_match_scores_word_boundary_higher()
{
    const auto boundary = spagyrist::fuzzy_match("k", "linux-kernel");
    const auto middle = spagyrist::fuzzy_match("k", "linker");

    SPAGYRIST_CHECK(boundary.matched);
    SPAGYRIST_CHECK(middle.matched);
    SPAGYRIST_CHECK(boundary.score > middle.score);
}

void fuzzy_match_scores_case_boundary_higher()
{
    const auto boundary = spagyrist::fuzzy_match("k", "linuxKernel");
    const auto middle = spagyrist::fuzzy_match("k", "linker");

    SPAGYRIST_CHECK(boundary.matched);
    SPAGYRIST_CHECK(middle.matched);
    SPAGYRIST_CHECK(boundary.score > middle.score);
}

void fuzzy_match_scores_shorter_candidate_higher()
{
    const auto shorter = spagyrist::fuzzy_match("lin", "linux");
    const auto longer = spagyrist::fuzzy_match("lin", "linux operating system family");

    SPAGYRIST_CHECK(shorter.matched);
    SPAGYRIST_CHECK(longer.matched);
    SPAGYRIST_CHECK(shorter.score > longer.score);
}

void fuzzy_match_empty_query_matches()
{
    const auto result = spagyrist::fuzzy_match("", "Linux");

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(result.score == 0.0);
    SPAGYRIST_CHECK(result.positions.empty());
}

void fuzzy_match_empty_candidate_does_not_match_nonempty_query()
{
    const auto result = spagyrist::fuzzy_match("Linux", "");

    SPAGYRIST_CHECK(!result.matched);
}

void fuzzy_match_is_smart_case_by_default()
{
    const auto lowercase_query = spagyrist::fuzzy_match("linux", "Linux");
    const auto uppercase_query = spagyrist::fuzzy_match("linux", "Linux", {.case_mode = spagyrist::match_case::sensitive});

    SPAGYRIST_CHECK(lowercase_query.matched);
    SPAGYRIST_CHECK(!uppercase_query.matched);
}

void fuzzy_match_sensitive_smart_case_when_query_has_uppercase()
{
    const auto result = spagyrist::fuzzy_match("LIN", "Linux");

    SPAGYRIST_CHECK(!result.matched);
}

void fuzzy_match_handles_utf8_as_bytes_without_crashing()
{
    const std::string candidate = "日本語 Linux";
    const auto result = spagyrist::fuzzy_match("Lin", candidate);

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(!result.positions.empty());
}

void fuzzy_match_handles_invalid_utf8_bytes_without_crashing()
{
    const std::string candidate = std::string{"\xff\xfe", 2} + "Linux";
    const auto result = spagyrist::fuzzy_match("Lin", candidate);

    SPAGYRIST_CHECK(result.matched);
}

void fuzzy_match_handles_long_strings()
{
    const std::string candidate(1000, 'a');
    const auto result = spagyrist::fuzzy_match("aaaa", candidate);

    SPAGYRIST_CHECK(result.matched);
    SPAGYRIST_CHECK(result.positions.size() == 4);
}

} // namespace

void run_matcher_tests()
{
    fuzzy_match_matches_exact_text();
    fuzzy_match_matches_prefix();
    fuzzy_match_matches_subsequence();
    fuzzy_match_rejects_missing_characters();
    fuzzy_match_keeps_scattered_long_matches_with_lower_score();
    fuzzy_match_allows_long_candidate_abbreviation();
    fuzzy_match_scores_consecutive_match_higher();
    fuzzy_match_scores_word_boundary_higher();
    fuzzy_match_scores_case_boundary_higher();
    fuzzy_match_scores_shorter_candidate_higher();
    fuzzy_match_empty_query_matches();
    fuzzy_match_empty_candidate_does_not_match_nonempty_query();
    fuzzy_match_is_smart_case_by_default();
    fuzzy_match_sensitive_smart_case_when_query_has_uppercase();
    fuzzy_match_handles_utf8_as_bytes_without_crashing();
    fuzzy_match_handles_invalid_utf8_bytes_without_crashing();
    fuzzy_match_handles_long_strings();
}
