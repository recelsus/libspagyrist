#include "spagyrist/candidate_text.hpp"

#include <utility>

namespace spagyrist {
namespace {

void append_search_part(std::string& output, std::string_view value, std::string_view separator)
{
    if (value.empty()) {
        return;
    }
    if (!output.empty()) {
        output += separator;
    }
    output += sanitize_candidate_text(value);
}

} // namespace

std::string sanitize_candidate_text(std::string_view value)
{
    std::string output;
    output.reserve(value.size());
    for (const auto ch : value) {
        const auto byte = static_cast<unsigned char>(ch);
        if (byte < 0x20 || byte == 0x7f) {
            output += ' ';
        } else {
            output += ch;
        }
    }
    return output;
}

candidate_text
project_candidate_text(
    std::size_t index,
    const candidate& value,
    const candidate_text_options& options)
{
    auto display = sanitize_candidate_text(value.title);
    if (value.subtitle && !value.subtitle->empty()) {
        display += " - ";
        display += sanitize_candidate_text(*value.subtitle);
    } else if (value.description && !value.description->empty()) {
        display += " - ";
        display += sanitize_candidate_text(*value.description);
    }

    std::string search;
    if (options.include_title) {
        append_search_part(search, value.title, options.separator);
    }
    if (options.include_subtitle && value.subtitle) {
        append_search_part(search, *value.subtitle, options.separator);
    }
    if (options.include_description && value.description) {
        append_search_part(search, *value.description, options.separator);
    }
    if (options.include_url && value.url) {
        append_search_part(search, *value.url, options.separator);
    }

    return candidate_text{
        .index = index,
        .display = std::move(display),
        .search = std::move(search),
    };
}

std::vector<candidate_text>
project_candidate_texts(
    std::span<const candidate> candidates,
    const candidate_text_options& options)
{
    std::vector<candidate_text> output;
    output.reserve(candidates.size());
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        output.push_back(project_candidate_text(i, candidates[i], options));
    }
    return output;
}

} // namespace spagyrist
