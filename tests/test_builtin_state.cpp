#include "selector/builtin_state.hpp"

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

std::vector<spagyrist::candidate_text> projected_candidates()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("Linux"));
    candidates.push_back(candidate_with_title("Linux kernel"));
    candidates.push_back(candidate_with_title("Darwin"));
    candidates.push_back(candidate_with_title("FreeBSD"));
    return spagyrist::project_candidate_texts(candidates);
}

void builtin_state_initially_shows_all_candidates()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    SPAGYRIST_CHECK(state.query().empty());
    SPAGYRIST_CHECK(state.ranked().size() == projected.size());
    SPAGYRIST_CHECK(state.cursor() == 0);
    SPAGYRIST_CHECK(state.selected_candidate_index() == 0);
}

void builtin_state_filters_when_typing()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    state.handle({.key = spagyrist::detail::terminal_key::character, .value = 'k'});

    SPAGYRIST_CHECK(state.query() == "k");
    SPAGYRIST_CHECK(state.ranked().size() == 1);
    SPAGYRIST_CHECK(state.selected_candidate_index() == 1);
}

void builtin_state_backspace_updates_query_and_results()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    state.handle({.key = spagyrist::detail::terminal_key::character, .value = 'k'});
    state.handle({.key = spagyrist::detail::terminal_key::backspace});

    SPAGYRIST_CHECK(state.query().empty());
    SPAGYRIST_CHECK(state.ranked().size() == projected.size());
}

void builtin_state_moves_selection()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    state.handle({.key = spagyrist::detail::terminal_key::arrow_down});

    SPAGYRIST_CHECK(state.cursor() == 1);
    SPAGYRIST_CHECK(state.selected_candidate_index() == 1);

    state.handle({.key = spagyrist::detail::terminal_key::arrow_up});

    SPAGYRIST_CHECK(state.cursor() == 0);
    SPAGYRIST_CHECK(state.selected_candidate_index() == 0);
}

void builtin_state_enter_selects_when_candidate_exists()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    const auto action = state.handle({.key = spagyrist::detail::terminal_key::enter});

    SPAGYRIST_CHECK(action == spagyrist::detail::builtin_selector_action::selected);
    SPAGYRIST_CHECK(state.selected_candidate_index() == 0);
}

void builtin_state_enter_keeps_editing_when_no_result_exists()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    state.handle({.key = spagyrist::detail::terminal_key::character, .value = 'z'});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = 'z'});
    const auto action = state.handle({.key = spagyrist::detail::terminal_key::enter});

    SPAGYRIST_CHECK(state.ranked().empty());
    SPAGYRIST_CHECK(action == spagyrist::detail::builtin_selector_action::editing);
    SPAGYRIST_CHECK(!state.selected_candidate_index().has_value());
}

void builtin_state_cancel_keys_cancel()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};

    SPAGYRIST_CHECK(state.handle({.key = spagyrist::detail::terminal_key::escape}) == spagyrist::detail::builtin_selector_action::cancelled);
    SPAGYRIST_CHECK(state.handle({.key = spagyrist::detail::terminal_key::ctrl_c}) == spagyrist::detail::builtin_selector_action::cancelled);
}

void builtin_state_scrolls_to_keep_cursor_visible()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state_options options;
    options.visible_count = 2;
    spagyrist::detail::builtin_selector_state state{projected, options};

    state.handle({.key = spagyrist::detail::terminal_key::arrow_down});
    state.handle({.key = spagyrist::detail::terminal_key::arrow_down});

    SPAGYRIST_CHECK(state.cursor() == 2);
    SPAGYRIST_CHECK(state.scroll_offset() == 1);
}

} // namespace

void run_builtin_state_tests()
{
    builtin_state_initially_shows_all_candidates();
    builtin_state_filters_when_typing();
    builtin_state_backspace_updates_query_and_results();
    builtin_state_moves_selection();
    builtin_state_enter_selects_when_candidate_exists();
    builtin_state_enter_keeps_editing_when_no_result_exists();
    builtin_state_cancel_keys_cancel();
    builtin_state_scrolls_to_keep_cursor_visible();
}
