#include "terminal.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <utility>

namespace spagyrist::detail {
namespace {

std::string errno_message(std::string_view prefix)
{
    std::string output{prefix};
    output += ": ";
    output += std::strerror(errno);
    return output;
}

} // namespace

raw_terminal_mode::raw_terminal_mode(int fd)
    : fd_(fd)
{
    if (fd_ < 0) {
        error_ = "invalid file descriptor";
        return;
    }

    if (tcgetattr(fd_, &original_) != 0) {
        error_ = errno_message("tcgetattr failed");
        return;
    }

    auto raw = original_;
    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_oflag &= static_cast<tcflag_t>(~OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(fd_, TCSAFLUSH, &raw) != 0) {
        error_ = errno_message("tcsetattr failed");
        return;
    }

    enabled_ = true;
}

raw_terminal_mode::raw_terminal_mode(raw_terminal_mode&& other) noexcept
    : fd_(other.fd_)
    , enabled_(other.enabled_)
    , error_(std::move(other.error_))
    , original_(other.original_)
{
    other.fd_ = -1;
    other.enabled_ = false;
}

raw_terminal_mode&
raw_terminal_mode::operator=(raw_terminal_mode&& other) noexcept
{
    if (this != &other) {
        restore();
        fd_ = other.fd_;
        enabled_ = other.enabled_;
        error_ = std::move(other.error_);
        original_ = other.original_;
        other.fd_ = -1;
        other.enabled_ = false;
    }
    return *this;
}

raw_terminal_mode::~raw_terminal_mode()
{
    restore();
}

bool raw_terminal_mode::enabled() const noexcept
{
    return enabled_;
}

const std::string& raw_terminal_mode::error() const noexcept
{
    return error_;
}

void raw_terminal_mode::restore() noexcept
{
    if (!enabled_) {
        return;
    }

    static_cast<void>(tcsetattr(fd_, TCSAFLUSH, &original_));
    enabled_ = false;
}

bool is_terminal(int fd) noexcept
{
    return fd >= 0 && isatty(fd) == 1;
}

std::optional<terminal_size> query_terminal_size(int fd) noexcept
{
    if (fd < 0) {
        return std::nullopt;
    }

    winsize size{};
    if (ioctl(fd, TIOCGWINSZ, &size) != 0 || size.ws_row == 0 || size.ws_col == 0) {
        return std::nullopt;
    }

    return terminal_size{
        .rows = size.ws_row,
        .columns = size.ws_col,
    };
}

terminal_input parse_terminal_input(std::string_view input) noexcept
{
    if (input.empty()) {
        return terminal_input{.key = terminal_key::end_of_input};
    }

    const auto first = static_cast<unsigned char>(input.front());
    if (first == 0x03) {
        return terminal_input{.key = terminal_key::ctrl_c};
    }
    if (first == '\r' || first == '\n') {
        return terminal_input{.key = terminal_key::enter};
    }
    if (first == 0x7f || first == 0x08) {
        return terminal_input{.key = terminal_key::backspace};
    }
    if (first == 0x1b) {
        if (input == "\x1b[A") {
            return terminal_input{.key = terminal_key::arrow_up};
        }
        if (input == "\x1b[B") {
            return terminal_input{.key = terminal_key::arrow_down};
        }
        if (input.size() == 1) {
            return terminal_input{.key = terminal_key::escape};
        }
        return terminal_input{.key = terminal_key::unknown};
    }
    if (first >= 0x20) {
        return terminal_input{.key = terminal_key::character, .value = input.front()};
    }
    return terminal_input{.key = terminal_key::unknown};
}

terminal_input read_terminal_input(int fd)
{
    char ch{};
    while (true) {
        const auto count = read(fd, &ch, 1);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(errno_message("read failed"));
        }
        if (count == 0) {
            return terminal_input{.key = terminal_key::end_of_input};
        }
        break;
    }

    if (ch != '\x1b') {
        return parse_terminal_input(std::string_view{&ch, 1});
    }

    char sequence[3]{ch, 0, 0};
    for (std::size_t i = 1; i < 3; ++i) {
        const auto count = read(fd, &sequence[i], 1);
        if (count <= 0) {
            return parse_terminal_input(std::string_view{sequence, i});
        }
    }
    return parse_terminal_input(std::string_view{sequence, 3});
}

} // namespace spagyrist::detail
