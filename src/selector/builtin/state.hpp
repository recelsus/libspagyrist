#pragma once

#include "spagyrist/candidate_text.hpp"
#include "spagyrist/ranking.hpp"
#include "terminal.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace spagyrist::detail {

struct builtin_selector_state_options {
    std::size_t visible_count{10};
    ranking_options ranking;
};

enum class builtin_selector_action {
    editing,
    selected,
    cancelled,
};

class builtin_selector_state {
public:
    explicit builtin_selector_state(
        std::span<const candidate_text> candidates,
        builtin_selector_state_options options = {});

    [[nodiscard]] const std::string& query() const noexcept;
    [[nodiscard]] std::span<const ranked_candidate> ranked() const noexcept;
    [[nodiscard]] std::size_t cursor() const noexcept;
    [[nodiscard]] std::size_t scroll_offset() const noexcept;
    [[nodiscard]] std::size_t visible_count() const noexcept;
    [[nodiscard]] std::optional<std::size_t> selected_candidate_index() const noexcept;

    builtin_selector_action handle(terminal_input input);

private:
    void refresh();
    void move_up() noexcept;
    void move_down() noexcept;
    void keep_cursor_visible() noexcept;

    std::vector<candidate_text> candidates_;
    builtin_selector_state_options options_;
    std::string query_;
    std::vector<ranked_candidate> ranked_;
    std::size_t cursor_{};
    std::size_t scroll_offset_{};
};

} // namespace spagyrist::detail
