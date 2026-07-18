#include "spagyrist/spagyrist.hpp"

#include "test_support.hpp"

#include <string>
#include <utility>
#include <vector>

namespace {

spagyrist::candidate candidate_with_title(std::string title)
{
    spagyrist::candidate value;
    value.id = title;
    value.title = std::move(title);
    return value;
}

spagyrist::candidate candidate_with_fields(std::string title, std::string subtitle, std::string description, std::string url)
{
    auto value = candidate_with_title(std::move(title));
    value.subtitle = std::move(subtitle);
    value.description = std::move(description);
    value.url = std::move(url);
    return value;
}

void ranking_places_higher_score_first()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("GNU/Linux"));
    candidates.push_back(candidate_with_title("Linux"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("Lin", projected);

    SPAGYRIST_CHECK(ranked.size() == 2);
    SPAGYRIST_CHECK(ranked[0].index == 1);
    SPAGYRIST_CHECK(ranked[1].index == 0);
}

void ranking_keeps_stable_order_for_same_score()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("abc"));
    candidates.push_back(candidate_with_title("abc"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("abc", projected);

    SPAGYRIST_CHECK(ranked.size() == 2);
    SPAGYRIST_CHECK(ranked[0].index == 0);
    SPAGYRIST_CHECK(ranked[1].index == 1);
}

void ranking_filters_unmatched_candidates()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("Linux"));
    candidates.push_back(candidate_with_title("Darwin"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("Lin", projected);

    SPAGYRIST_CHECK(ranked.size() == 1);
    SPAGYRIST_CHECK(ranked[0].index == 0);
}

void ranking_preserves_match_positions()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("Linux"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("lx", projected);

    SPAGYRIST_CHECK(ranked.size() == 1);
    SPAGYRIST_CHECK(ranked[0].positions == std::vector<std::size_t>({0, 4}));
    SPAGYRIST_CHECK(ranked[0].search_positions == std::vector<std::size_t>({0, 4}));
    SPAGYRIST_CHECK(ranked[0].display_positions == std::vector<std::size_t>({0, 4}));
}

void ranking_separates_search_and_display_match_positions()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_fields(
        "Linux",
        "Kernel",
        "Apple appears only in the searchable description.",
        "https://example.test/linux"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("Apple", projected);

    SPAGYRIST_CHECK(ranked.size() == 1);
    SPAGYRIST_CHECK(!ranked[0].search_positions.empty());
    SPAGYRIST_CHECK(ranked[0].display_positions.empty());
}

void ranking_can_keep_input_order()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("GNU/Linux"));
    candidates.push_back(candidate_with_title("Linux"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    spagyrist::ranking_options options;
    options.order = spagyrist::ranking_order::input;
    const auto ranked = spagyrist::rank_candidates("Lin", projected, options);

    SPAGYRIST_CHECK(ranked.size() == 2);
    SPAGYRIST_CHECK(ranked[0].index == 0);
    SPAGYRIST_CHECK(ranked[1].index == 1);
}

void ranking_handles_empty_candidate_list()
{
    std::vector<spagyrist::candidate_text> projected;

    const auto ranked = spagyrist::rank_candidates("Lin", projected);

    SPAGYRIST_CHECK(ranked.empty());
}

void ranking_keeps_scattered_matches_but_ranks_close_match_first()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_fields(
        "Linux",
        "Operating system kernel",
        "A Unix-like operating system family used across servers and devices.",
        "https://example.test/linux"));
    candidates.push_back(candidate_with_fields(
        "Darwin",
        "Apple operating system core",
        "The open source Unix-like core used by Apple's operating systems.",
        "https://example.test/darwin"));
    candidates.push_back(candidate_with_fields(
        "Plan 9",
        "Distributed operating system",
        "A research operating system from Bell Labs.",
        "https://example.test/plan9"));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    const auto ranked = spagyrist::rank_candidates("Apple", projected);

    SPAGYRIST_CHECK(ranked.size() >= 1);
    SPAGYRIST_CHECK(ranked[0].index == 1);
}

} // namespace

void run_ranking_tests()
{
    ranking_places_higher_score_first();
    ranking_keeps_stable_order_for_same_score();
    ranking_filters_unmatched_candidates();
    ranking_preserves_match_positions();
    ranking_separates_search_and_display_match_positions();
    ranking_can_keep_input_order();
    ranking_handles_empty_candidate_list();
    ranking_keeps_scattered_matches_but_ranks_close_match_first();
}
