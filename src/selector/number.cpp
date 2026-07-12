#include "spagyrist/selector/number.hpp"

#include <charconv>
#include <iostream>
#include <string>

namespace spagyrist {
namespace {

std::string summary(const candidate& value)
{
    auto output = value.title;
    if (value.subtitle && !value.subtitle->empty()) {
        output += " - " + *value.subtitle;
    } else if (value.description && !value.description->empty()) {
        output += " - " + *value.description;
    }
    return output;
}

std::optional<std::size_t> parse_choice(const std::string& value, std::size_t count)
{
    std::size_t choice{};
    const auto result = std::from_chars(value.data(), value.data() + value.size(), choice);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) {
        return std::nullopt;
    }
    if (choice == 0 || choice > count) {
        return std::nullopt;
    }
    return choice - 1;
}

} // namespace

number_selector::number_selector(number_selector_options options)
    : options_(options)
{
    if (options_.input == nullptr) {
        options_.input = &std::cin;
    }
    if (options_.output == nullptr) {
        options_.output = &std::cout;
    }
}

std::optional<std::size_t>
number_selector::select(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return std::nullopt;
    }

    auto& input = *options_.input;
    auto& output = *options_.output;

    output << "Select a result:\n";
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        output << "  " << (i + 1) << ". " << summary(candidates[i]) << '\n';
    }
    output << "Enter number, or empty to cancel: ";
    output.flush();

    std::string line;
    if (!std::getline(input, line) || line.empty()) {
        return std::nullopt;
    }

    return parse_choice(line, candidates.size());
}

} // namespace spagyrist

