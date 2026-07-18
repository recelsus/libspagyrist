#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace spagyrist {

using metadata_map = std::unordered_map<std::string, std::string>;

struct candidate {
    std::string id;
    std::string title;
    std::optional<std::string> subtitle;
    std::optional<std::string> description;
    std::optional<std::string> url;
    std::optional<std::string> preview;
    metadata_map metadata;
};

struct candidate_list {
    std::vector<candidate> candidates;
};

struct selection {
    std::size_t index{};
    candidate item;
};

[[nodiscard]] bool is_valid(const candidate& value) noexcept;

} // namespace spagyrist
