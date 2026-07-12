#pragma once

#include "spagyrist/candidate.hpp"

#include <optional>
#include <span>

namespace spagyrist {

class selector {
public:
    virtual ~selector() = default;

    [[nodiscard]] virtual std::optional<std::size_t>
    select(std::span<const candidate> candidates) = 0;
};

[[nodiscard]] std::optional<selection>
select_candidate(selector& selector, std::span<const candidate> candidates);

} // namespace spagyrist

