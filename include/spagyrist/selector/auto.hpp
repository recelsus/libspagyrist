#pragma once

#include "spagyrist/selector/builtin.hpp"
#include "spagyrist/selector/number.hpp"

namespace spagyrist {

struct auto_selector_options {
    builtin_selector_options builtin;
    number_selector_options number;
};

class auto_selector final : public selector {
public:
    explicit auto_selector(auto_selector_options options = {});

    [[nodiscard]] bool is_available() const override;

    [[nodiscard]] std::optional<std::size_t>
    select(std::span<const candidate> candidates) override;

    [[nodiscard]] selector_result
    select_result(std::span<const candidate> candidates) override;

private:
    auto_selector_options options_;
};

} // namespace spagyrist
