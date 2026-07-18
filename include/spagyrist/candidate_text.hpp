#pragma once

#include "spagyrist/candidate.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace spagyrist {

struct candidate_text_options {
    bool include_title{true};
    bool include_subtitle{true};
    bool include_description{true};
    bool include_url{true};
    std::string separator{" "};
};

struct candidate_text {
    std::size_t index{};
    std::string display;
    std::string search;
};

[[nodiscard]] std::string sanitize_candidate_text(std::string_view value);

[[nodiscard]] candidate_text
project_candidate_text(
    std::size_t index,
    const candidate& value,
    const candidate_text_options& options = {});

[[nodiscard]] std::vector<candidate_text>
project_candidate_texts(
    std::span<const candidate> candidates,
    const candidate_text_options& options = {});

} // namespace spagyrist
