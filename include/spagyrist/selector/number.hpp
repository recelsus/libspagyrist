#pragma once

#include "spagyrist/selector.hpp"

#include <iosfwd>

namespace spagyrist {

struct number_selector_options {
    std::istream* input{};
    std::ostream* output{};
};

class number_selector final : public selector {
public:
    explicit number_selector(number_selector_options options = {});

    [[nodiscard]] std::optional<std::size_t>
    select(std::span<const candidate> candidates) override;

private:
    number_selector_options options_;
};

} // namespace spagyrist

