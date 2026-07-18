#include "terminal.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace spagyrist::detail {
namespace {

std::string errno_message(std::string_view prefix)
{
    std::string output{prefix};
    output += ": ";
    output += std::strerror(errno);
    return output;
}

bool write_terminal_all_impl(int fd, std::string_view value, std::string* error) noexcept
{
    while (!value.empty()) {
        const auto written = write(fd, value.data(), value.size());
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (error != nullptr) {
                *error = errno_message("terminal write failed");
            }
            return false;
        }
        if (written == 0) {
            if (error != nullptr) {
                *error = "terminal write failed: wrote 0 bytes";
            }
            return false;
        }
        value.remove_prefix(static_cast<std::size_t>(written));
    }
    return true;
}

} // namespace

terminal_screen_guard::terminal_screen_guard(int fd)
    : fd_(fd)
{
    if (fd_ < 0) {
        error_ = "invalid file descriptor";
        return;
    }
    if (!write_terminal_all_impl(fd_, "\033[?1049h", &error_)) {
        return;
    }
    screen_enabled_ = true;

    if (!write_terminal_all_impl(fd_, "\033[?25l", &error_)) {
        return;
    }
    cursor_hidden_ = true;
}

terminal_screen_guard::~terminal_screen_guard()
{
    if (cursor_hidden_) {
        static_cast<void>(try_write_terminal_all(fd_, "\033[?25h"));
    }
    if (screen_enabled_) {
        static_cast<void>(try_write_terminal_all(fd_, "\033[?1049l"));
    }
}

bool terminal_screen_guard::enabled() const noexcept
{
    return screen_enabled_ && cursor_hidden_;
}

const std::string& terminal_screen_guard::error() const noexcept
{
    return error_;
}

void write_terminal_all(int fd, std::string_view value)
{
    std::string error;
    if (!write_terminal_all_impl(fd, value, &error)) {
        throw std::runtime_error(error);
    }
}

bool try_write_terminal_all(int fd, std::string_view value) noexcept
{
    return write_terminal_all_impl(fd, value, nullptr);
}

} // namespace spagyrist::detail
