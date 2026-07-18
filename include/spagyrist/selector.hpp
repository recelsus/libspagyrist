#pragma once

#include "spagyrist/candidate.hpp"

#include <optional>
#include <span>
#include <string>

namespace spagyrist {

enum class selector_status {
    selected,
    no_selection,
    cancelled,
    unavailable,
    error,
    invalid_selection,
};

struct selector_result {
    selector_status status{selector_status::no_selection};
    std::optional<std::size_t> index;
    std::string message;

    [[nodiscard]] static selector_result selected(std::size_t index);
    [[nodiscard]] static selector_result no_selection();
    [[nodiscard]] static selector_result cancelled();
    [[nodiscard]] static selector_result unavailable(std::string message = {});
    [[nodiscard]] static selector_result error(std::string message);
    [[nodiscard]] static selector_result invalid_selection(std::size_t index);
};

struct candidate_selection_result {
    selector_status status{selector_status::no_selection};
    std::optional<selection> selected;
    std::string message;
};

class selector {
public:
    virtual ~selector() = default;

    [[nodiscard]] virtual bool is_available() const;

    [[nodiscard]] virtual std::optional<std::size_t>
    select(std::span<const candidate> candidates) = 0;

    [[nodiscard]] virtual selector_result
    select_result(std::span<const candidate> candidates);
};

[[nodiscard]] candidate_selection_result
select_candidate_result(selector& selector, std::span<const candidate> candidates);

[[nodiscard]] std::optional<selection>
select_candidate(selector& selector, std::span<const candidate> candidates);

[[nodiscard]] candidate_selection_result
select_candidate_with_fallback_result(
    selector& primary,
    selector& fallback,
    std::span<const candidate> candidates);

[[nodiscard]] std::optional<selection>
select_candidate_with_fallback(
    selector& primary,
    selector& fallback,
    std::span<const candidate> candidates);

} // namespace spagyrist
