#include "spagyrist/selector/builtin.hpp"

#include "spagyrist/candidate_text.hpp"
#include "state.hpp"
#include "view.hpp"
#include "terminal.hpp"

#include <algorithm>
#include <unistd.h>

namespace spagyrist {

builtin_selector::builtin_selector(builtin_selector_options options)
    : options_(options)
{
}

bool builtin_selector::is_available() const
{
    return detail::is_terminal(STDIN_FILENO) && detail::is_terminal(STDOUT_FILENO);
}

std::optional<std::size_t>
builtin_selector::select(std::span<const candidate> candidates)
{
    const auto result = select_result(candidates);
    if (result.status == selector_status::selected) {
        return result.index;
    }
    return std::nullopt;
}

selector_result
builtin_selector::select_result(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return selector_result::no_selection();
    }
    if (!is_available()) {
        return selector_result::unavailable("builtin selector requires a TTY");
    }

    try {
        const auto projected = project_candidate_texts(candidates);
        auto visible_count = options_.visible_count == 0 ? std::size_t{1} : options_.visible_count;
        if (const auto terminal_size = detail::query_terminal_size(STDOUT_FILENO)) {
            if (terminal_size->rows > 2) {
                visible_count = std::min(visible_count, terminal_size->rows - 1);
            }
        }

        detail::builtin_selector_state_options state_options;
        state_options.visible_count = visible_count;
        detail::builtin_selector_state state{projected, state_options};

        detail::raw_terminal_mode mode{STDIN_FILENO};
        if (!mode.enabled()) {
            return selector_result::error(mode.error().empty() ? "failed to enable raw terminal mode" : mode.error());
        }

        const detail::terminal_screen_guard screen{STDOUT_FILENO};
        if (!screen.enabled()) {
            return selector_result::error(screen.error().empty() ? "failed to initialize terminal screen" : screen.error());
        }
        while (true) {
            detail::builtin_selector_view_options view_options;
            view_options.use_color = options_.use_color;
            if (const auto terminal_size = detail::query_terminal_size(STDOUT_FILENO)) {
                view_options.width = terminal_size->columns;
            }
            detail::write_terminal_all(STDOUT_FILENO, detail::render_builtin_selector_screen(state, projected, view_options));

            const auto action = state.handle(detail::read_terminal_input(STDIN_FILENO));
            if (action == detail::builtin_selector_action::cancelled) {
                return selector_result::cancelled();
            }
            if (action == detail::builtin_selector_action::selected) {
                const auto selected = state.selected_candidate_index();
                if (!selected) {
                    return selector_result::invalid_selection(candidates.size());
                }
                if (*selected >= candidates.size()) {
                    return selector_result::invalid_selection(*selected);
                }
                return selector_result::selected(*selected);
            }
        }
    } catch (const std::exception& error) {
        return selector_result::error(error.what());
    } catch (...) {
        return selector_result::error("builtin selector failed");
    }
}

} // namespace spagyrist
