#pragma once

#include "spagyrist/selector.hpp"

#include <string>
#include <vector>

namespace spagyrist {

struct fzf_selector_options {
    std::string executable{"fzf"};
    std::vector<std::string> arguments;
};

class fzf_selector final : public selector {
public:
    explicit fzf_selector(fzf_selector_options options = {});

    [[nodiscard]] bool is_available() const;

    [[nodiscard]] std::optional<std::size_t>
    select(std::span<const candidate> candidates) override;

private:
    fzf_selector_options options_;
};

} // namespace spagyrist
