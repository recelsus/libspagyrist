#include "builtin_view.hpp"

#include <algorithm>
#include <string_view>
#include <utility>

namespace spagyrist::detail {
namespace {

struct truncated_display {
    std::string text;
    std::size_t matched_prefix_size{};
};

truncated_display truncate_display(std::string_view value, std::size_t width)
{
    if (width == 0) {
        return {};
    }
    if (value.size() <= width) {
        return truncated_display{
            .text = std::string{value},
            .matched_prefix_size = value.size(),
        };
    }
    if (width <= 1) {
        return truncated_display{
            .text = "~",
            .matched_prefix_size = 0,
        };
    }
    std::string output{value.substr(0, width - 1)};
    output += '~';
    return truncated_display{
        .text = std::move(output),
        .matched_prefix_size = width - 1,
    };
}

std::vector<std::size_t> visible_positions(
    const std::vector<std::size_t>& positions,
    std::size_t matched_prefix_size)
{
    std::vector<std::size_t> output;
    for (const auto position : positions) {
        if (position >= matched_prefix_size) {
            break;
        }
        output.push_back(position);
    }
    return output;
}

std::string highlighted_display(
    std::string_view display,
    const std::vector<std::size_t>& positions,
    bool use_color)
{
    std::string output;
    std::size_t position_index = 0;
    for (std::size_t i = 0; i < display.size(); ++i) {
        while (position_index < positions.size() && positions[position_index] < i) {
            ++position_index;
        }
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
    if (use_color) {
        output += "\033[0m";
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

        const auto content_width = options.width > 2 ? options.width - 2 : std::size_t{};
        const auto display = truncate_display(candidate->display, content_width);
        const auto positions = visible_positions(
            ranked_candidate.display_positions,
            display.matched_prefix_size);

        std::string line = row == state.cursor() ? "> " : "  ";
        line += highlighted_display(display.text, positions, options.use_color);
        append_line(output, line);
    }

    if (ranked.empty()) {
        append_line(output, "  no matches");
    }

    return output;
}

} // namespace spagyrist::detail
