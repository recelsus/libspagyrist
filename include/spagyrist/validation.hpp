#pragma once

#include "spagyrist/candidate.hpp"
#include "spagyrist/document.hpp"
#include "spagyrist/error.hpp"

namespace spagyrist {

[[nodiscard]] validation_result validate(const candidate& value);
[[nodiscard]] validation_result validate(const candidate_list& value);
[[nodiscard]] validation_result validate(const document& value);

} // namespace spagyrist

