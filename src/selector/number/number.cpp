#include "spagyrist/selector/number.hpp"

#include "spagyrist/candidate_text.hpp"

#include <charconv>
#include <exception>
#include <iostream>
#include <string>

namespace spagyrist {
namespace {

std::optional<std::size_t> parse_number(const std::string& value)
{
    std::size_t choice{};
    const auto result = std::from_chars(value.data(), value.data() + value.size(), choice);
    if (result.ec != std::errc{} || result.ptr != value.data() + value.size()) {
        return std::nullopt;
    }
    return choice;
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
    const auto result = select_result(candidates);
    if (result.status == selector_status::selected) {
        return result.index;
    }
    return std::nullopt;
}

selector_result
number_selector::select_result(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return selector_result::no_selection();
    }

    try {
        auto& input = *options_.input;
        auto& output = *options_.output;

        output << "Select a result:\n";
        for (std::size_t i = 0; i < candidates.size(); ++i) {
            output << "  " << (i + 1) << ". " << project_candidate_text(i, candidates[i]).display << '\n';
        }
        while (true) {
            output << "Enter number, or EOF to cancel: ";
            output.flush();
            if (!output) {
                return selector_result::error("number selector output failed");
            }

            std::string line;
            if (!std::getline(input, line)) {
                return selector_result::cancelled();
            }

            const auto choice = parse_number(line);
            if (!choice) {
                output << "Enter a number between 1 and " << candidates.size() << ".\n";
                continue;
            }
            if (*choice == 0 || *choice > candidates.size()) {
                output << "Selection out of range. Maximum is " << candidates.size() << ".\n";
                continue;
            }
            return selector_result::selected(*choice - 1);
        }
    } catch (const std::exception& error) {
        return selector_result::error(error.what());
    } catch (...) {
        return selector_result::error("number selector failed");
    }
}

} // namespace spagyrist
