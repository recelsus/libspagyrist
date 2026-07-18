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

} // namespace

void run_ranking_tests()
{
    ranking_places_higher_score_first();
    ranking_keeps_stable_order_for_same_score();
    ranking_filters_unmatched_candidates();
    ranking_preserves_match_positions();
    ranking_can_keep_input_order();
    ranking_handles_empty_candidate_list();
}
