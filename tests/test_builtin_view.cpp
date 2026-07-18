#include "selector/builtin/view.hpp"

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
    return spagyrist::project_candidate_texts(candidates);
}

void builtin_view_renders_query_count_and_selection()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find(">  (3)") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("> Linux") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("  Linux kernel") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("\r\n> Linux\r\n") != std::string::npos);
}

void builtin_view_highlights_matches_without_color()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "L"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "i"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "n"});
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("[L][i][n]ux") != std::string::npos);
}

void builtin_view_does_not_highlight_search_only_matches()
{
    std::vector<spagyrist::candidate> candidates;
    auto value = candidate_with_title("Linux");
    value.subtitle = "Kernel";
    value.description = "Apple appears only in the hidden search text.";
    candidates.push_back(std::move(value));

    const auto projected = spagyrist::project_candidate_texts(candidates);
    spagyrist::detail::builtin_selector_state state{projected};
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "A"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "p"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "p"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "l"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "e"});
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.use_color = false;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("> Linux - Kernel") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("[") == std::string::npos);
}

void builtin_view_renders_no_matches()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "z"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "z"});
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("  no matches") != std::string::npos);
}

void builtin_view_truncates_long_lines()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.width = 8;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("> Linux") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("  Linux~") != std::string::npos);
}

void builtin_view_ignores_highlights_outside_truncated_display()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "k"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "e"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "r"});
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.width = 8;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("> Linux~") != std::string::npos);
    SPAGYRIST_CHECK(screen.find('[') == std::string::npos);
}

void builtin_view_resets_ansi_color_at_line_end()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "L"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "i"});
    state.handle({.key = spagyrist::detail::terminal_key::character, .value = "n"});
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.use_color = true;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("\033[1mL\033[0m") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("ux\033[0m\r\n") != std::string::npos);
}

void builtin_view_handles_extremely_narrow_width()
{
    const auto projected = projected_candidates();
    spagyrist::detail::builtin_selector_state state{projected};
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.width = 1;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("> \r\n") != std::string::npos);
}

void builtin_view_does_not_split_utf8_when_truncating()
{
    std::vector<spagyrist::candidate> candidates;
    candidates.push_back(candidate_with_title("\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86"));
    const auto projected = spagyrist::project_candidate_texts(candidates);
    spagyrist::detail::builtin_selector_state state{projected};
    spagyrist::detail::builtin_selector_view_options options;
    options.clear_screen = false;
    options.width = 6;

    const auto screen = spagyrist::detail::render_builtin_selector_screen(state, projected, options);

    SPAGYRIST_CHECK(screen.find("> \xe3\x81\x82~") != std::string::npos);
    SPAGYRIST_CHECK(screen.find("\xe3\x81~") == std::string::npos);
}

} // namespace

void run_builtin_view_tests()
{
    builtin_view_renders_query_count_and_selection();
    builtin_view_highlights_matches_without_color();
    builtin_view_does_not_highlight_search_only_matches();
    builtin_view_renders_no_matches();
    builtin_view_truncates_long_lines();
    builtin_view_ignores_highlights_outside_truncated_display();
    builtin_view_resets_ansi_color_at_line_end();
    builtin_view_handles_extremely_narrow_width();
    builtin_view_does_not_split_utf8_when_truncating();
}
