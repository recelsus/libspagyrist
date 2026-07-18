#include "format.hpp"

#include "spagyrist/candidate_text.hpp"

#include <charconv>

namespace spagyrist::detail::fzf {
namespace {

std::string escaped_preview(std::string_view value)
{
    std::string output;
    output.reserve(value.size());
    for (const auto ch : value) {
        const auto byte = static_cast<unsigned char>(ch);
        switch (ch) {
        case '\\':
            output += "\\\\";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\n";
            break;
        case '\t':
            output += "\\t";
            break;
        default:
            if (byte < 0x20 || byte == 0x7f) {
                output += ' ';
            } else {
                output += ch;
            }
            break;
        }
    }
    return output;
}

std::string line_for_candidate(const candidate& value, const candidate_text& text)
{
    auto line = text.display;
    line += '\t';
    line += std::to_string(text.index);
    line += '\t';
    if (value.preview && !value.preview->empty()) {
        line += escaped_preview(*value.preview);
    }
    line += '\n';
    return line;
}

} // namespace

std::vector<std::string> default_arguments()
{
    return {
        "--delimiter",
        "\t",
        "--with-nth",
        "1",
        "--nth",
        "1",
        "--accept-nth",
        "2",
        "--no-sort",
    };
}

bool has_preview(std::span<const candidate> candidates)
{
    for (const auto& candidate : candidates) {
        if (candidate.preview && !candidate.preview->empty()) {
            return true;
        }
    }
    return false;
}

bool has_preview_argument(const std::vector<std::string>& arguments)
{
    for (const auto& argument : arguments) {
        if (argument == "--preview" || argument.rfind("--preview=", 0) == 0) {
            return true;
        }
    }
    return false;
}

std::string input_for_candidates(std::span<const candidate> candidates)
{
    std::string input;
    const auto projected = project_candidate_texts(candidates);
    for (std::size_t i = 0; i < projected.size(); ++i) {
        input += line_for_candidate(candidates[i], projected[i]);
    }
    return input;
}

std::optional<std::size_t> parse_selected_index(std::string_view output)
{
    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
        output.remove_suffix(1);
    }

    std::size_t index{};
    auto parse_index = [&index](std::string_view index_text) {
        const auto result = std::from_chars(index_text.data(), index_text.data() + index_text.size(), index);
        return result.ec == std::errc{} && result.ptr == index_text.data() + index_text.size();
    };

    if (parse_index(output)) {
        return index;
    }

    const auto first_tab = output.find('\t');
    if (first_tab == std::string_view::npos) {
        return std::nullopt;
    }
    if (parse_index(output.substr(0, first_tab))) {
        return index;
    }
    const auto second_tab = output.find('\t', first_tab + 1);
    const auto index_text = output.substr(
        first_tab + 1,
        second_tab == std::string_view::npos ? output.size() - first_tab - 1 : second_tab - first_tab - 1);
    if (parse_index(index_text)) {
        return index;
    }
    return std::nullopt;
}

} // namespace spagyrist::detail::fzf
