#include "builtin_state.hpp"

#include <algorithm>
#include <utility>

namespace spagyrist::detail {

builtin_selector_state::builtin_selector_state(
    std::span<const candidate_text> candidates,
    builtin_selector_state_options options)
    : candidates_(candidates.begin(), candidates.end())
    , options_(std::move(options))
{
    if (options_.visible_count == 0) {
        options_.visible_count = 1;
    }
    refresh();
}

const std::string& builtin_selector_state::query() const noexcept
{
    return query_;
}

std::span<const ranked_candidate> builtin_selector_state::ranked() const noexcept
{
    return ranked_;
}

std::size_t builtin_selector_state::cursor() const noexcept
{
    return cursor_;
}

std::size_t builtin_selector_state::scroll_offset() const noexcept
{
    return scroll_offset_;
}

std::size_t builtin_selector_state::visible_count() const noexcept
{
    return options_.visible_count;
}

std::optional<std::size_t> builtin_selector_state::selected_candidate_index() const noexcept
{
    if (ranked_.empty() || cursor_ >= ranked_.size()) {
        return std::nullopt;
    }
    return ranked_[cursor_].index;
}

builtin_selector_action builtin_selector_state::handle(terminal_input input)
{
    switch (input.key) {
    case terminal_key::character:
        query_ += input.value;
        refresh();
        return builtin_selector_action::editing;
    case terminal_key::backspace:
        if (!query_.empty()) {
            query_.pop_back();
            refresh();
        }
        return builtin_selector_action::editing;
    case terminal_key::arrow_up:
    case terminal_key::ctrl_p:
        move_up();
        return builtin_selector_action::editing;
    case terminal_key::arrow_down:
    case terminal_key::ctrl_n:
        move_down();
        return builtin_selector_action::editing;
    case terminal_key::enter:
        return selected_candidate_index()
            ? builtin_selector_action::selected
            : builtin_selector_action::editing;
    case terminal_key::escape:
    case terminal_key::ctrl_c:
    case terminal_key::end_of_input:
        return builtin_selector_action::cancelled;
    case terminal_key::unknown:
        return builtin_selector_action::editing;
    }
    return builtin_selector_action::editing;
}

void builtin_selector_state::refresh()
{
    ranked_ = rank_candidates(query_, candidates_, options_.ranking);
    if (ranked_.empty()) {
        cursor_ = 0;
        scroll_offset_ = 0;
        return;
    }
    cursor_ = std::min(cursor_, ranked_.size() - 1);
    keep_cursor_visible();
}

void builtin_selector_state::move_up() noexcept
{
    if (ranked_.empty()) {
        return;
    }
    if (cursor_ > 0) {
        --cursor_;
    }
    keep_cursor_visible();
}

void builtin_selector_state::move_down() noexcept
{
    if (ranked_.empty()) {
        return;
    }
    if (cursor_ + 1 < ranked_.size()) {
        ++cursor_;
    }
    keep_cursor_visible();
}

void builtin_selector_state::keep_cursor_visible() noexcept
{
    if (cursor_ < scroll_offset_) {
        scroll_offset_ = cursor_;
        return;
    }
    if (cursor_ >= scroll_offset_ + options_.visible_count) {
        scroll_offset_ = cursor_ - options_.visible_count + 1;
    }
}

} // namespace spagyrist::detail
