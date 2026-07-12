#pragma once

#include <string>
#include <vector>

namespace spagyrist {

enum class error_code {
    invalid_candidate,
    invalid_document,
};

struct validation_error {
    error_code code{error_code::invalid_document};
    std::string path;
    std::string message;
};

struct validation_result {
    std::vector<validation_error> errors;

    [[nodiscard]] bool ok() const noexcept
    {
        return errors.empty();
    }
};

} // namespace spagyrist

