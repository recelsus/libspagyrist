#include "builtin_view.hpp"

#include <algorithm>
#include <string_view>

namespace spagyrist::detail {
namespace {

std::string truncate_line(std::string value, std::size_t width)
{
    if (width == 0 || value.size() <= width) {
        return value;
    }
    if (width <= 1) {
        return value.substr(0, width);
    }
    value.resize(width - 1);
    value += '~';
    return value;
}

std::string highlighted_display(
    std::string_view display,
    const std::vector<std::size_t>& positions,
    bool use_color)
{
    std::string output;
    std::size_t position_index = 0;
    for (std::size_t i = 0; i < display.size(); ++i) {
        const auto matched = position_index < positions.size() && positions[position_index] == i;
        if (matched && use_color) {
            output += "\033[1m";
        } else if (matched) {
            output += '[';
        }

        output += display[i];

        if (matched && use_color) {
            output += "\033[0m";
            ++position_index;
        } else if (matched) {
            output += ']';
            ++position_index;
        }
    }
    return output;
}

const candidate_text* find_candidate(const std::vector<candidate_text>& candidates, std::size_t index)
{
    for (const auto& candidate : candidates) {
        if (candidate.index == index) {
            return &candidate;
        }
    }
    return nullptr;
}

void append_line(std::string& output, std::string_view line)
{
    output += line;
    output += "\r\n";
}

} // namespace

std::string render_builtin_selector_screen(
    const builtin_selector_state& state,
    const std::vector<candidate_text>& candidates,
    const builtin_selector_view_options& options)
{
    std::string output;
    if (options.clear_screen) {
        output += "\033[?25l\033[H\033[J";
    }

    append_line(output, "> " + state.query() + " (" + std::to_string(state.ranked().size()) + ")");

    const auto ranked = state.ranked();
    const auto begin = std::min(state.scroll_offset(), ranked.size());
    const auto end = std::min(begin + state.visible_count(), ranked.size());
    for (std::size_t row = begin; row < end; ++row) {
        const auto& ranked_candidate = ranked[row];
        const auto* candidate = find_candidate(candidates, ranked_candidate.index);
        if (candidate == nullptr) {
            continue;
        }

        std::string line = row == state.cursor() ? "> " : "  ";
        line += truncate_line(
            highlighted_display(candidate->display, ranked_candidate.positions, options.use_color),
            options.width > 2 ? options.width - 2 : options.width);
        append_line(output, line);
    }

    if (ranked.empty()) {
        append_line(output, "  no matches");
    }

    return output;
}

} // namespace spagyrist::detail
