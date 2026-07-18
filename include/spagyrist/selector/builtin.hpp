#pragma once

#include "spagyrist/selector.hpp"

#include <cstddef>

namespace spagyrist {

struct builtin_selector_options {
    std::size_t visible_count{10};
    bool use_color{true};
};

class builtin_selector final : public selector {
public:
    explicit builtin_selector(builtin_selector_options options = {});

    [[nodiscard]] bool is_available() const override;

    [[nodiscard]] std::optional<std::size_t>
    select(std::span<const candidate> candidates) override;

private:
    builtin_selector_options options_;
};

} // namespace spagyrist
