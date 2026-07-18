#include "spagyrist/matcher.hpp"

#include <algorithm>
#include <utility>

namespace spagyrist {
namespace {

bool is_ascii_upper(char value)
{
    return value >= 'A' && value <= 'Z';
}

bool is_ascii_lower(char value)
{
    return value >= 'a' && value <= 'z';
}

char ascii_lower(char value)
{
    if (is_ascii_upper(value)) {
        return static_cast<char>(value - 'A' + 'a');
    }
    return value;
}

bool has_ascii_upper(std::string_view value)
{
    return std::any_of(value.begin(), value.end(), is_ascii_upper);
}

bool should_ignore_case(std::string_view query, match_case case_mode)
{
    switch (case_mode) {
    case match_case::sensitive:
        return false;
    case match_case::insensitive:
        return true;
    case match_case::smart:
        return !has_ascii_upper(query);
    }
    return true;
}

bool chars_equal(char query, char candidate, bool ignore_case)
{
    if (!ignore_case) {
        return query == candidate;
    }
    return ascii_lower(query) == ascii_lower(candidate);
}

bool is_separator(char value)
{
    return value == ' '
        || value == '\t'
        || value == '\n'
        || value == '\r'
        || value == '-'
        || value == '_'
        || value == '.'
        || value == '/'
        || value == ':';
}

bool is_word_boundary(std::string_view value, std::size_t pos)
{
    return pos == 0 || is_separator(value[pos - 1]);
}

bool is_case_boundary(std::string_view value, std::size_t pos)
{
    return pos > 0 && is_ascii_lower(value[pos - 1]) && is_ascii_upper(value[pos]);
}

double score_match(std::string_view query, std::string_view candidate, const std::vector<std::size_t>& positions, bool ignore_case)
{
    if (positions.empty()) {
        return 0.0;
    }

    double score = static_cast<double>(query.size()) * 10.0;

    if (query.size() == candidate.size()) {
        bool exact = true;
        for (std::size_t i = 0; i < query.size(); ++i) {
            if (!chars_equal(query[i], candidate[i], ignore_case)) {
                exact = false;
                break;
            }
        }
        if (exact) {
            score += 1000.0;
        }
    }

    if (positions.front() == 0) {
        score += 200.0;
    } else {
        score -= static_cast<double>(positions.front()) * 2.0;
    }

    std::size_t total_gap = 0;
    for (std::size_t i = 0; i < positions.size(); ++i) {
        const auto pos = positions[i];
        if (is_word_boundary(candidate, pos)) {
            score += 8.0;
        }
        if (is_case_boundary(candidate, pos)) {
            score += 6.0;
        }
        if (i > 0) {
            const auto gap = positions[i] - positions[i - 1] - 1;
            total_gap += gap;
            if (gap == 0) {
                score += 15.0;
            }
        }
    }

    const auto span = positions.back() - positions.front() + 1;
    score -= static_cast<double>(total_gap) * 1.5;
    score -= static_cast<double>(span > query.size() ? span - query.size() : 0) * 0.75;
    score -= static_cast<double>(candidate.size()) * 0.01;
    return score;
}

bool is_scattered_long_match(std::string_view query, const std::vector<std::size_t>& positions)
{
    if (query.size() < 4 || positions.size() < 2) {
        return false;
    }

    const auto span = positions.back() - positions.front() + 1;
    return span > query.size() * 4;
}

} // namespace

match_result
fuzzy_match(
    std::string_view query,
    std::string_view candidate,
    const match_options& options)
{
    if (query.empty()) {
        return match_result{
            .matched = true,
            .score = 0.0,
            .positions = {},
        };
    }
    if (candidate.empty()) {
        return match_result{};
    }

    const auto ignore_case = should_ignore_case(query, options.case_mode);
    std::vector<std::size_t> positions;
    positions.reserve(query.size());

    std::size_t candidate_pos = 0;
    for (const auto query_ch : query) {
        bool found = false;
        while (candidate_pos < candidate.size()) {
            if (chars_equal(query_ch, candidate[candidate_pos], ignore_case)) {
                positions.push_back(candidate_pos);
                ++candidate_pos;
                found = true;
                break;
            }
            ++candidate_pos;
        }
        if (!found) {
            return match_result{};
        }
    }

    if (options.reject_scattered_long_matches && is_scattered_long_match(query, positions)) {
        return match_result{};
    }

    return match_result{
        .matched = true,
        .score = score_match(query, candidate, positions, ignore_case),
        .positions = std::move(positions),
    };
}

} // namespace spagyrist
