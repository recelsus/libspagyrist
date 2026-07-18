#include "spagyrist/selector/fzf.hpp"

#include "executable.hpp"
#include "format.hpp"
#include "process.hpp"

#include <stdexcept>
#include <utility>

namespace spagyrist {

fzf_selector::fzf_selector(fzf_selector_options options)
    : options_(std::move(options))
{
}

bool fzf_selector::is_available() const
{
    return detail::fzf::executable_exists(options_.executable);
}

std::optional<std::size_t>
fzf_selector::select(std::span<const candidate> candidates)
{
    const auto result = select_result(candidates);
    if (result.status == selector_status::selected) {
        return result.index;
    }
    return std::nullopt;
}

selector_result
fzf_selector::select_result(std::span<const candidate> candidates)
{
    if (candidates.empty()) {
        return selector_result::no_selection();
    }
    if (!is_available()) {
        return selector_result::unavailable("fzf executable is not available");
    }

    try {
        auto arguments = detail::fzf::default_arguments();
        arguments.insert(arguments.end(), options_.arguments.begin(), options_.arguments.end());
        if (detail::fzf::has_preview(candidates) && !detail::fzf::has_preview_argument(arguments)) {
            arguments.push_back("--preview");
            arguments.push_back("printf '%b' {3}");
        }

        const auto result = detail::fzf::run_process(
            options_.executable,
            arguments,
            detail::fzf::input_for_candidates(candidates));

        if (result.exit_code == 130 || result.exit_code == 1) {
            return selector_result::cancelled();
        }
        if (result.exit_code != 0) {
            auto message = "fzf exited with status " + std::to_string(result.exit_code);
            if (!result.error.empty()) {
                auto error = result.error;
                while (!error.empty() && (error.back() == '\n' || error.back() == '\r')) {
                    error.pop_back();
                }
                message += ": " + error;
            }
            return selector_result::error(message);
        }

        const auto selected = detail::fzf::parse_selected_index(result.output);
        if (!selected) {
            return selector_result::error("fzf returned invalid output");
        }
        if (*selected >= candidates.size()) {
            return selector_result::invalid_selection(*selected);
        }
        return selector_result::selected(*selected);
    } catch (const std::exception& error) {
        return selector_result::error(error.what());
    } catch (...) {
        return selector_result::error("fzf selector failed");
    }
}

} // namespace spagyrist
