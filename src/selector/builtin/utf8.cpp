#include "utf8.hpp"

namespace spagyrist::detail {
namespace {

bool is_continuation(unsigned char value) noexcept
{
    return (value & 0xc0U) == 0x80U;
}

} // namespace

std::size_t utf8_code_point_length(unsigned char lead) noexcept
{
    if (lead < 0x80U) {
        return 1;
    }
    if (lead >= 0xc2U && lead <= 0xdfU) {
        return 2;
    }
    if (lead >= 0xe0U && lead <= 0xefU) {
        return 3;
    }
    if (lead >= 0xf0U && lead <= 0xf4U) {
        return 4;
    }
    return 0;
}

bool is_complete_utf8_code_point(std::string_view value) noexcept
{
    if (value.empty()) {
        return false;
    }

    const auto length = utf8_code_point_length(static_cast<unsigned char>(value.front()));
    if (length == 0 || value.size() != length) {
        return false;
    }

    for (std::size_t i = 1; i < value.size(); ++i) {
        if (!is_continuation(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }
    return true;
}

std::size_t utf8_safe_prefix_size(std::string_view value, std::size_t max_bytes) noexcept
{
    std::size_t offset = 0;
    while (offset < value.size()) {
        const auto length = utf8_code_point_length(static_cast<unsigned char>(value[offset]));
        if (length == 0 || offset + length > value.size() || offset + length > max_bytes) {
            break;
        }

        bool complete = true;
        for (std::size_t i = 1; i < length; ++i) {
            if (!is_continuation(static_cast<unsigned char>(value[offset + i]))) {
                complete = false;
                break;
            }
        }
        if (!complete) {
            break;
        }
        offset += length;
    }
    return offset;
}

void pop_back_utf8_code_point(std::string& value)
{
    if (value.empty()) {
        return;
    }

    auto pos = value.size() - 1;
    while (pos > 0 && is_continuation(static_cast<unsigned char>(value[pos]))) {
        --pos;
    }

    if (utf8_safe_prefix_size(std::string_view{value}.substr(pos), value.size() - pos) == value.size() - pos) {
        value.resize(pos);
        return;
    }

    value.pop_back();
}

} // namespace spagyrist::detail
